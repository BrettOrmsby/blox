#include<stdio.h>
#include<string.h>

#define MEMORY_SIZE 30000

unsigned char memory[MEMORY_SIZE];
int pointer = 0;

int findZeroLeft(int position) {
	for (int i = position; i >= 0; i--) {
		if (memory[i] == 0) {
			return i;
		}
	}
	for (int i = MEMORY_SIZE - 1; i > position; i--) {
		if (memory[i] == 0) {
			return i;
		}
	}
	return -1;
}

int findZeroRight(int position) {
	for (int i = position; i < MEMORY_SIZE; i++) {
		if (memory[i] == 0) {
			return i;
		}
	}
	for (int i = 0; i < position; i++) {
		if (memory[i] == 0) {
			return i;
		}
	}
	return -1;
}

int main() {
	memset(memory, 0, MEMORY_SIZE);

	while (memory[pointer] != 0) {
	}
	while (memory[pointer] != 0) {
	}
	while (memory[pointer] != 0) {
	}
	while (memory[pointer] != 0) {
	}
	while (memory[pointer] != 0) {
	}
	while (memory[pointer] != 0) {
	}

	return 0;
}
