FLAGS = -Wall -g

all: bf-lang

bf-lang: main.o chunk.o memory.o debug.o value.o vm.o compiler.o scanner.o
	gcc $(FLAGS) -o bf-lang main.o chunk.o memory.o debug.o value.o vm.o compiler.o scanner.o

main.o: main.c chunk.h vm.h common.h
	gcc $(FLAGS) -c main.c

chunk.o: chunk.c chunk.h memory.h value.h common.h
	gcc $(FLAGS) -c chunk.c

memory.o: memory.c memory.h common.h
	gcc $(FLAGS) -c memory.c

debug.o: debug.c debug.h value.h common.h
	gcc $(FLAGS) -c debug.c

value.o: value.c value.h common.h
	gcc $(FLAGS) -c value.c

vm.o: vm.c vm.h chunk.h debug.h common.h value.h compiler.h
	gcc $(FLAGS) -c vm.c

compiler.o: compiler.c compiler.h common.h scanner.h
	gcc $(FLAGS) -c compiler.c

scanner.o: scanner.c scanner.h common.h
	gcc $(FLAGS) -c scanner.c

clean:
	rm *.o bf-lang
