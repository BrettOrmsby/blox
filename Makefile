FLAGS = -Wall -g

all: bf-lang

bf-lang: main.o chunk.o memory.o debug.o value.o vm.o compiler.o scanner.o object.o table.o
	gcc $(FLAGS) -o bf-lang main.o chunk.o memory.o debug.o value.o vm.o compiler.o scanner.o object.o table.o

main.o: main.c chunk.h vm.h common.h
	gcc $(FLAGS) -c main.c

chunk.o: chunk.c chunk.h memory.h value.h common.h vm.h
	gcc $(FLAGS) -c chunk.c

memory.o: memory.c memory.h common.h object.h vm.h debug.h compiler.h
	gcc $(FLAGS) -c memory.c

debug.o: debug.c debug.h value.h common.h object.h
	gcc $(FLAGS) -c debug.c

value.o: value.c value.h common.h object.h
	gcc $(FLAGS) -c value.c

vm.o: vm.c vm.h chunk.h debug.h common.h value.h compiler.h object.h memory.h table.h object.h
	gcc $(FLAGS) -c vm.c

compiler.o: compiler.c compiler.h common.h scanner.h vm.h debug.h object.h memory.h
	gcc $(FLAGS) -c compiler.c

scanner.o: scanner.c scanner.h common.h
	gcc $(FLAGS) -c scanner.c
object.o: object.c object.h common.h memory.h value.h vm.h table.h chunk.h
	gcc $(FLAGS) -c object.c
table.o: table.c table.h common.h value.h memory.h object.h
	gcc $(FLAGS) -c table.c
clean:
	rm *.o bf-lang
