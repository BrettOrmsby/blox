#ifndef chunk_h
#define chunk_h

#include "common.h"
#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_INPUT,
  OP_POP,
  OP_DEFINE_GLOBAL,
  OP_GET_LOCAL,
  OP_GET_GLOBAL,
  OP_GET_UPVALUE,
  OP_SET_UPVALUE,
  OP_SET_LOCAL,
  OP_SET_GLOBAL,
  OP_SET_ARRAY,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_RETURNS,
  OP_RETURN,
  OP_PRINT,
  OP_JUMP,
  OP_JUMP_BACK,
  OP_IF,
  OP_ELSE,
  OP_ENDIF,
  OP_DUP,
  OP_SET_CELL,
  OP_LOOP,
  OP_ENDLOOP,
  OP_CALL,
  OP_INDEX,
  OP_CLOSURE,
  OP_CLOSE_UPVALUE,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_MODULUS,
  OP_NOT,
} OpCode;

typedef struct {
  int count;
  int capacity;
  uint8_t *code;
  int *lines;
  ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
int addConstant(Chunk *chunk, Value value);
void freeChunk(Chunk *chunk);
#endif
