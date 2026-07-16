FLAGS = -Wall -g
SRC = src
BUILD = build

all: bf-lang

$(BUILD):
	mkdir -p $(BUILD)

bf-lang: $(BUILD)/main.o $(BUILD)/chunk.o $(BUILD)/memory.o $(BUILD)/debug.o $(BUILD)/value.o $(BUILD)/vm.o $(BUILD)/compiler.o $(BUILD)/scanner.o $(BUILD)/object.o $(BUILD)/table.o
	gcc $(FLAGS) -o bf-lang $(BUILD)/main.o $(BUILD)/chunk.o $(BUILD)/memory.o $(BUILD)/debug.o $(BUILD)/value.o $(BUILD)/vm.o $(BUILD)/compiler.o $(BUILD)/scanner.o $(BUILD)/object.o $(BUILD)/table.o

$(BUILD)/main.o: $(SRC)/main.c $(SRC)/chunk.h $(SRC)/vm.h $(SRC)/common.h | $(BUILD)
	gcc $(FLAGS) -c $(SRC)/main.c -o $(BUILD)/main.o

$(BUILD)/chunk.o: $(SRC)/chunk.c $(SRC)/chunk.h $(SRC)/memory.h $(SRC)/value.h $(SRC)/common.h $(SRC)/vm.h | $(BUILD)
	gcc $(FLAGS) -c $(SRC)/chunk.c -o $(BUILD)/chunk.o

$(BUILD)/memory.o: $(SRC)/memory.c $(SRC)/memory.h $(SRC)/common.h $(SRC)/object.h $(SRC)/vm.h $(SRC)/debug.h $(SRC)/compiler.h | $(BUILD)
	gcc $(FLAGS) -c $(SRC)/memory.c -o $(BUILD)/memory.o

$(BUILD)/debug.o: $(SRC)/debug.c $(SRC)/debug.h $(SRC)/value.h $(SRC)/common.h $(SRC)/object.h | $(BUILD)
	gcc $(FLAGS) -c $(SRC)/debug.c -o $(BUILD)/debug.o

$(BUILD)/value.o: $(SRC)/value.c $(SRC)/value.h $(SRC)/common.h $(SRC)/object.h | $(BUILD)
	gcc $(FLAGS) -c $(SRC)/value.c -o $(BUILD)/value.o

$(BUILD)/vm.o: $(SRC)/vm.c $(SRC)/vm.h $(SRC)/chunk.h $(SRC)/debug.h $(SRC)/common.h $(SRC)/value.h $(SRC)/compiler.h $(SRC)/object.h $(SRC)/memory.h $(SRC)/table.h $(SRC)/object.h | $(BUILD)
	gcc $(FLAGS) -c $(SRC)/vm.c -o $(BUILD)/vm.o

$(BUILD)/compiler.o: $(SRC)/compiler.c $(SRC)/compiler.h $(SRC)/common.h $(SRC)/scanner.h $(SRC)/vm.h $(SRC)/debug.h $(SRC)/object.h $(SRC)/memory.h | $(BUILD)
	gcc $(FLAGS) -c $(SRC)/compiler.c -o $(BUILD)/compiler.o

$(BUILD)/scanner.o: $(SRC)/scanner.c $(SRC)/scanner.h $(SRC)/common.h | $(BUILD)
	gcc $(FLAGS) -c $(SRC)/scanner.c -o $(BUILD)/scanner.o
$(BUILD)/object.o: $(SRC)/object.c $(SRC)/object.h $(SRC)/common.h $(SRC)/memory.h $(SRC)/value.h $(SRC)/vm.h $(SRC)/table.h $(SRC)/chunk.h | $(BUILD)
	gcc $(FLAGS) -c $(SRC)/object.c -o $(BUILD)/object.o
$(BUILD)/table.o: $(SRC)/table.c $(SRC)/table.h $(SRC)/common.h $(SRC)/value.h $(SRC)/memory.h $(SRC)/object.h | $(BUILD)
	gcc $(FLAGS) -c $(SRC)/table.c -o $(BUILD)/table.o
clean:
	rm -rf $(BUILD) bf-lang
