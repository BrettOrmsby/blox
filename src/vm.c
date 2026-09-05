#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"
#include "compiler.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "memory.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = NULL;
}

static void runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for(int i = vm.frameCount -1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
        if(function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }
    resetStack();
}

void initVM() {
 resetStack();
 vm.objects = NULL;
 vm.bytesAllocated = 0;
 vm.nextGC = 1024 * 1024;

 vm.grayCount = 0;
 vm.grayCapacity = 0;
 vm.grayStack = NULL;

 vm.cellPointer = 0;
 vm.scopeCount = 0;

 initTable(&vm.globals);
 initTable(&vm.strings);
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeObjects();
}

void pushVM(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value popVM() {
    vm.stackTop--;
    Value removed = *vm.stackTop;
    return removed;
}

static Value peekVM(int distance) {
    return vm.stackTop[-1 - distance];
}

static void pop() {
    printf("<[-]");
    vm.cellPointer--;
    popVM();
}

void copyCell(int pos) {
    // Need to copy the cell into the next two open cells,
    // then use the last cell to copy it back into the original
    int distance = vm.cellPointer - pos;
    if(distance < 0) {
        runtimeError("Copy Cell Failed. Somthing happened to get a negetive distance");
    }

    // PART 1: Duplicate cell at pos into the 2 blank cells
    for(int i = 0; i < distance; i++) printf("<");
    printf("[-");
    for(int i = 0; i < distance; i++) printf(">");
    printf("+>+");
    for(int i = 0; i < distance + 1; i++) printf("<");
    printf("]");

    // PART 2: Copy the cell at the latest position back into pos
    // Note that this zeroes out the last cell too
    for(int i = 0; i < distance + 1;  i++) printf(">");
    printf("[-");
    for(int i = 0; i < distance + 1;  i++) printf("<");
    printf("+");
    for(int i = 0; i < distance + 1;  i++) printf(">");
    printf("]");
    vm.cellPointer++;
}

static void push(Value value) {
    switch(value.type) {
        case VAL_REFERENCE:
            copyCell(AS_NUMBER(value));
            vm.cellPointer--; // Since we increment it in the copyCell function
            break;
        case VAL_NIL:
            printf(">");
            break;
        case VAL_BOOL:
            if(AS_BOOL(value)) {
                printf("+>");
            } else {
                printf(">");
            }
            break;
        case VAL_NUMBER:
            {
                int number = AS_NUMBER(value);
                for(int i = 0; i < number; i++) {
                    printf("+");
                }
                printf(">");
                break;
            }
        case VAL_OBJ:
            // TODO: is this needed?
            // Objects are treated as 0 if they are not used in their special
            // locations.
            printf(">");
            vm.cellPointer++;
            pushVM(value);
            return;
    }
    pushVM(REFERENCE_VAL(vm.cellPointer));
    vm.cellPointer++;
}

static void setReference(int pos) {
    int dist = vm.cellPointer - pos - 1;
    // Zero out variable
    for(int i = 0; i <= dist; i++) printf("<");
    printf("[-]");
    for(int i = 0; i < dist; i++) printf(">");
    // Copy current cell into variable
    printf("[-");
    for(int i = 0; i < dist; i++) printf("<");
    printf("+");
    for(int i = 0; i < dist; i++) printf(">");
    printf("]");
    vm.cellPointer--;
    popVM();
}

static bool call(ObjClosure* closure, int argCount) {
    if(argCount != closure->function->arity) {
        runtimeError("Expected %d arguments but got %d.", closure->function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount) {
    if(IS_OBJ(callee)) {
        switch(OBJ_TYPE(callee)) {
            case OBJ_CLOSURE:
                return call(AS_CLOSURE(callee), argCount);
            default:
                break; // Non-callable object type
        }
    }
    runtimeError("Can only call functions.");
    return false;
}

static ObjUpvalue* captureUpvalue(Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue* createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }
    return createdUpvalue;
}

static void closeUpvalues(Value* last) {
    while (vm.openUpvalues != NULL &&
        vm.openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

static InterpretResult run() {
    CallFrame* frame = &vm.frames[vm.frameCount - 1];

    #define READ_BYTE() (*frame->ip++)

    #define READ_SHORT() \
        (frame->ip += 2, \
        (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

    #define READ_CONSTANT() \
        (frame->closure->function->chunk.constants.values[READ_BYTE()])

    #define READ_STRING() AS_STRING(READ_CONSTANT())

    for(;;) {
        #ifdef DEBUG_TRACE_EXECUTION
        printf("\n");
        for(Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("{ ");
            printValue(*slot);
            printf(" }");
        }
        printf("\n");
        disassembleInstruction(&frame->closure->function->chunk, (int)(frame->ip - frame->closure->function->chunk.code));
        printf("Cellpointer: %d\n", vm.cellPointer);
        #endif
        uint8_t instruction;
        switch(instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NIL: push(NIL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            case OP_POP: pop(); break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if(!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_SET_LOCAL: {
                // TODO: test local fucntions (if possible)
                uint8_t slot = READ_BYTE();
                push(peekVM(0)); // Duplicate the cell since an expression statement will pop it
                setReference(AS_NUMBER(frame->slots[slot]));
                // TODO: what if not a ref
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value reference;
                if(!tableGet(&vm.globals, name, &reference)) {
                    runtimeError("Undefined variables '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                if(!IS_REFERENCE(reference)) {
                    runtimeError("Invalid reassign to '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(peekVM(0)); // Duplicate the cell since an expression statement will pop it
                setReference(AS_NUMBER(reference));
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                tableSet(&vm.globals, name, peekVM(0));
                popVM(); // Globals do not remain on the value stack since they are kept in a table
                break;
            }
            case OP_GET_UPVALUE: {
                // TODO: not implemented
                uint8_t slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                // TODO: not implemented
                uint8_t slot = READ_BYTE();
                push(peekVM(0)); // Duplicate the cell since an expression statement will pop it
                // TODO: what if not a ref
                setReference(AS_NUMBER(*frame->closure->upvalues[slot]->location));
                break;
            }
            case OP_EQUAL:
                // [Start][Decrement both cells][Normalize to 1][Negate]
                // EX: 970 --> 200 --> 010 --> 000
                //       ^      ^      ^        ^
                // EX: 550 --> 000 --> 000 --> 100
                //       ^      ^      ^        ^
                printf("<[-<->]<[[-]>+<]+>[-<->]");
                vm.cellPointer--;
                popVM();
                popVM();
                pushVM(REFERENCE_VAL(vm.cellPointer-1));
                break;
            case OP_GREATER:  {
                // See below for less than to understand. The left and right are just swaped.
                printf("<[->>+<<]<[->>>>+<<<<]>+>>+>+<[->-[>]<<]<[->[-]<]<[-<>>[-]+<<]>>[-<<+>>]>>[-]<[-]<[-]<[-]");
                vm.cellPointer--;
                popVM();
                popVM();
                pushVM(REFERENCE_VAL(vm.cellPointer-1));
            }
            break;
            case OP_LESS: {
                // See https://stackoverflow.com/a/13327857
                // Setup required 0 1 0 >l r 0
                // <[->>>+<<<]<[->>>+<<<]>+>>
                // Need to start by pointing to l
                // +>+<                   This is for managing if l=0 and r=0
                //     [->-[>]<<]             This is a magic loop. if l is the one which
                //                            reaches 0 first (l<r),then pointer will be at(4).
                //                            Else it will be at (3)
                //     <[-
                //          //       BLOCK (l>=r)
                //          //You are at (2) and do whatever you want and come back to (2).
                //          //Its a must
                //         >[-]< // Zero out cell 3
                //     ]
                //     <[-<
                //          //       BLOCK(l<r)
                //          //You are at (1) and do whatever you want and come back to (1).
                //          //Its a must
                //        >>[-]+<< // Set cell 3 to 1
                //     ]
                // >>[-<<+>>] // Copy 3 into 1
                // >>[-]<[-]<[-]<[-] // Zero out 5, 4, 3, 2
                printf("<[->>>+<<<]<[->>>+<<<]>+>>+>+<[->-[>]<<]<[->[-]<]<[-<>>[-]+<<]>>[-<<+>>]>>[-]<[-]<[-]<[-]");
                vm.cellPointer--;
                popVM();
                popVM();
                pushVM(REFERENCE_VAL(vm.cellPointer-1));
            };
            break;

            case OP_ADD:
                printf("<[-<+>]");
                vm.cellPointer--;
                popVM();
                popVM();
                pushVM(REFERENCE_VAL(vm.cellPointer-1));
                break;
            case OP_SUBTRACT:
                printf("<[-<->]");
                vm.cellPointer--;
                popVM();
                popVM();
                pushVM(REFERENCE_VAL(vm.cellPointer-1));
                break;
            case OP_MULTIPLY:
                    /*
                     // Layout will be [L][R][R'][Result]
                     <<[- // Loop for L
                        >[->+>+<<] // Add R to R' and Result
                        >[-<+>] // Move R' into R
                     <<]
                     >>>[-<<<+>>>] // Copy Result into L
                     <<[-] // Zero out R
                     */
                    printf("<<[->[->+>+<<]>[-<+>]<<]>>>[-<<<+>>>]<<[-]");
                    vm.cellPointer--;
                    popVM();
                    popVM();
                    pushVM(REFERENCE_VAL(vm.cellPointer-1));
                break;
            case OP_DIVIDE:
                /*
                 * SEE https://esolangs.org/wiki/Brainfuck_algorithms#Divmod
                 # Set up for this senario
                 +<<
                 # >n d 1 0 0 0
                 [->-[>+>>]>[[-<+>]+>+>>]<<<<<]
                 # >0 d-n%d n%d+1 n/d 0 0
                 # Copy 4 into 1, and zero out 2 and 3
                 >>>[-<<<+>>>]<[-]<[-]
                 */
                printf("+<<[->-[>+>>]>[[-<+>]+>+>>]<<<<<]>>>[-<<<+>>>]<[-]<[-]");
                vm.cellPointer--;
                popVM();
                popVM();
                pushVM(REFERENCE_VAL(vm.cellPointer-1));
            break;
            case OP_NOT:
                /* [operand][result]
                    +          // Set result to 1
                    <[[-]>-<]  // If the operand is > 0, clear it, then set result to 0
                    >[-<+>]    // Move result into operand
                */
                printf("+<[[-]>-<]>[-<+>]");
                popVM();
                pushVM(REFERENCE_VAL(vm.cellPointer-1));
                break;
            case OP_PRINT:
                printf("<.[-]");
                vm.cellPointer--;
                popVM();
                break;
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_JUMP_BACK: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            // If and else uses a flag cell after the condition to
            // conditionaly enter or skip the else block
            // The content between these three commands must end at the
            // cell it started at
            case OP_IF:
                printf("+<[[-]>->");
                vm.cellPointer++;
                popVM(); // for the condition
                break;
            case OP_ELSE:
                printf("<<]>[[-]>");
                break;
            case OP_ENDIF:
                printf("<]<[-]");
                vm.cellPointer -= 2; // Condition and flag are clear
                break;
            case OP_DUP:
                push(REFERENCE_VAL(vm.cellPointer-1));
                break;
            case OP_SET_CELL:
                setReference(vm.cellPointer - READ_BYTE());
                break;
            case OP_LOOP:
                printf("+[>");
                vm.cellPointer++;
                break;
            case OP_ENDLOOP:
                printf("<]");
                vm.cellPointer--;
                break;
            case OP_CALL: {
                int argCount = READ_BYTE();
                if(!callValue(peekVM(argCount), argCount)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount -1];
                break;
            }
            case OP_CLOSURE: {
                // TODO: not implemented
                ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = newClosure(function);
                push(OBJ_VAL(closure));
                for(int i = 0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if(isLocal) {
                        closure->upvalues[i] = captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case OP_CLOSE_UPVALUE:
                // TODO: not implemented
                closeUpvalues(vm.stackTop - 1);
                pop();
                break;
            case OP_RETURN: {
                // TODO: not fully* implemented
                // TODO: need to use the poped value, handle special case/common where
                // it is a reference value than needs to be copied back.
                pop();
                closeUpvalues(frame->slots);
                vm.frameCount --;
                if(vm.frameCount == 0) {
                    pop();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                // move the cell at result location back to the current location
                // for(int i = vm.cellPointer; i < resultLocation; i++) {
                //     printf(">");
                // }
                // printf("[-");
                // for(int i = resultLocation; i > vm.cellPointer; i--) {
                //     printf("<");
                // }
                // printf("+");
                // for(int i = vm.cellPointer; i < resultLocation; i++) {
                //     printf(">");
                // }
                // printf("]");
                // for(int i = resultLocation; i > vm.cellPointer; i--) {
                //     printf("<");
                // }
                // printf(">");
                // push(RE)
                push(BOOL_VAL(true));
                frame = &vm.frames[vm.frameCount -1];
                break;
            }
        }
    }
    #undef READ_BYTE
    #undef READ_SHORT
    #undef READ_CONSTANT
    #undef READ_STRING
    #undef BINARY_OP
}
InterpretResult interpret(const char* source) {
    ObjFunction* function = compile(source);
    if(function == NULL) return INTERPRET_COMPILE_ERROR;

    pushVM(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    popVM();
    pushVM(OBJ_VAL(closure));
    call(closure, 0);

    return run();
}
