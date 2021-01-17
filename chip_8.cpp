#include "chip_8.h"
#include <fstream>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <ctime>

const unsigned int START_ADDRESS = 0x200;

// Fontset for CHIP-8
unsigned char fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8(){
	// Memory, registers, display, timers, stack, and other values reset during constructor creation
	pc = 0x200;

	// Place fontset into memory starting at address 0x50
	for (std::size_t i{0}; i < 80; i++) {
		memory[0x50 + i] = fontset[i];
	}

	// Initializing and seeding random number generator
	srand((unsigned) time(0));

	// Set up jump tables
	op_table[0x0] = &Chip8::table0;
	op_table[0x1] = &Chip8::op_1NNN;
	op_table[0x2] = &Chip8::op_2NNN;
	op_table[0x3] = &Chip8::op_3XNN;
	op_table[0x4] = &Chip8::op_4XNN;
	op_table[0x5] = &Chip8::op_5XY0;
	op_table[0x6] = &Chip8::op_6XNN;
	op_table[0x7] = &Chip8::op_7XNN;
	op_table[0x8] = &Chip8::table8;
	op_table[0x9] = &Chip8::op_9XY0;
	op_table[0xA] = &Chip8::op_ANNN;
	op_table[0xB] = &Chip8::op_BNNN;
	op_table[0xC] = &Chip8::op_CXNN;
	op_table[0xD] = &Chip8::op_DXYN;
	op_table[0xE] = &Chip8::tableE;
	op_table[0xF] = &Chip8::tableF;

	op_table0[0x0] = &Chip8::op_00E0;
	op_table0[0xE] = &Chip8::op_00EE;

	op_table8[0x0] = &Chip8::op_8XY0;
	op_table8[0x1] = &Chip8::op_8XY1;
	op_table8[0x2] = &Chip8::op_8XY2;
	op_table8[0x3] = &Chip8::op_8XY3;
	op_table8[0x4] = &Chip8::op_8XY4;
	op_table8[0x5] = &Chip8::op_8XY5;
	op_table8[0x6] = &Chip8::op_8XY6;
	op_table8[0x7] = &Chip8::op_8XY7;
	op_table8[0xE] = &Chip8::op_8XYE;

	op_tableE[0x1] = &Chip8::op_EXA1;
	op_tableE[0xE] = &Chip8::op_EX9E;

	op_tableF[0x07] = &Chip8::op_FX07;
	op_tableF[0x0A] = &Chip8::op_FX0A;
	op_tableF[0x15] = &Chip8::op_FX15;
	op_tableF[0x18] = &Chip8::op_FX18;
	op_tableF[0x1E] = &Chip8::op_FX1E;
	op_tableF[0x29] = &Chip8::op_FX29;
	op_tableF[0x33] = &Chip8::op_FX33;
	op_tableF[0x55] = &Chip8::op_FX55;
	op_tableF[0x65] = &Chip8::op_FX65;

}

void Chip8::loadROM(char const* filename) {
	// Open file as binary and place stream pointer at the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open()) {
		// Get size of file and allocate buffer for data
		std::streampos size = file.tellg();
		char *buffer = new char[size];

		// Set pointer back to beginning of file
		file.seekg(0, file.beg);

		// Read file and fill buffer
		file.read(buffer, size);
		file.close();

		// Load contents of ROM and place into memory, starting at 0x200
		for (unsigned int i{0}; i < size; i++) {
			memory[0x200 + i] = buffer[i];
		}

		// Deallocate buffer
		delete[] buffer;
	}

}

void Chip8::emulateCycle() {
	// Fetch opcode
	opcode = (memory[pc] << 8) | memory[pc + 1];

	// Increment pointer counter to next instruction
	pc += 2;

	// Decode and execute opcode
	op_table[(opcode & 0xF000u) >> 12u]();

	// Update timers
	if (delay_timer > 0) {
		delay_timer--;
	}
	if (sound_timer > 0) {
		sound_timer--;
	}
}

// ----Helper functions---- //
uint8_t Chip8::getX() {
	return (opcode & 0x0F00u) >> 8u;
}

uint8_t Chip8::getY() {
	return (opcode & 0x00F0u) >> 4u;
}

uint8_t Chip8::getN() {
	return opcode & 0x000Fu;
}

uint8_t Chip8::getNN() {
	return opcode & 0x00FFu;
}

uint16_t Chip8::getNNN() {
	return opcode & 0x0FFFu;
}

// ----Opcodes---- //

// 00E0 - CLS : Clears the display
void Chip8::op_00E0() {
	memset(display, 0, sizeof(display));
}

// 00EE - RET : Returns from a subroutine
void Chip8::op_00EE() {
	sp--;
	pc = stack[sp];
}

// 1NNN - Jp addr : Jump to location NNN
void Chip8::op_1NNN() {
	pc = getNNN();
}

// 2NNN - Call addr : Call subroutine at nnn
void Chip8::op_2NNN() {
	stack[sp] = pc;
	sp++;
	pc = getNNN();
}

// 3XNN - SE VX, byte : Skip next instruction if VX = NN
void Chip8::op_3XNN() {
	if (V[getX()] == getNN()) {
		pc += 2;
	}
}

// 4XNN - SNE VX, byte : Skip next instruction if VX != NN
void Chip8::op_4XNN() {
	if (V[getX()] != getNN()) {
		pc += 2;
	}
}

// 5XY0 - SE VX, VY : Skip next instruction if VX = VY
void Chip8::op_5XY0() {
	if (V[getX()] == V[getY()]) {
		pc += 2;
	}
}

// 6XNN - LD Vx, byte : Set VX = NN
void Chip8::op_6XNN() {
	V[getX()] = getNN();
}

// 7XNN - ADD VX, byte : Set VX = VX + NN
void Chip8::op_7XNN() {
	V[getX()] += getNN();
}

// 8XY0 - LD VX, VY : Set VX = VY
void Chip8::op_8XY0() {
	V[getX()] = V[getY()];
}

// 8XY1 - OR VX, VY : Set VX = VX OR VY
void Chip8::op_8XY1() {
	V[getX()] = V[getX()] | V[getY()];
}

// 8XY2 - AND VX, VY : Set VX = VX AND VY
void Chip8::op_8XY2() {
	V[getX()] = V[getX()] & V[getY()];
}

// 8XY3 - XOR VX, VY : Set VX = VX XOR VY
void Chip8::op_8XY3() {
	V[getX()] = V[getX()] ^ V[getY()];
}

// 8XY4 - ADD VX, VY : Set VX = VX + VY, set VF = carry
void Chip8::op_8XY4() {
	uint16_t sum = V[getX()] + V[getY()];

	if (sum > 255) {
		V[0xFu] = 1;
	} else {
		V[0xFu] = 0;
	}

	V[getX()] = sum & 0xFFu;
}

// 8XY5 - SUB VX, VY : Set VX = VX - VY, set VF = NOT borrow
void Chip8::op_8XY5() {
	if (V[getX()] > V[getY()]) {
		V[0xFu] = 1;
	} else {
		V[0xFu] = 0;
	}

	V[getX()] -= V[getY()];
}

// 8XY6 - SHR VX {, VY} : Set VX = VX SHR 1
void Chip8::op_8XY6() {
	V[0xFu] = V[getX()] & 0x1u;
	V[getX()] >>= 1;
}

// 8XY7 - SUBN VX, VY : Set VX = VY - VX, set VF = NOT borrow
void Chip8::op_8XY7() {
	if (V[getY()] > V[getX()]) {
		V[0xFu] = 1;
	} else {
		V[0xFu] = 0;
	}

	V[getX()] = V[getY()] - V[getX()];
}

// 8XYE - SHL VX {, VY} : Set VX = VX SHL 1
void Chip8::op_8XYE() {
	V[0xFu] = (V[getX()] & 0b10000000u) >> 7u;
	V[getX()] <<= 1;
}

// 9XY0 - SNE VX, VY : Skip next instruction if VX != VY
void Chip8::op_9XY0() {
	if (V[getX()] != V[getY()]) {
		pc += 2;
	}
}

// ANNN - LD I, addr : Set I = NNN
void Chip8::op_ANNN() {
	I = getNNN();
}

// BNNN - JP V0, addr : Jump to location NNN + V0
void Chip8::op_BNNN() {
	pc = getNNN() + V[0];
}

// CXNN - RND VX, byte : Set VX = random byte AND NN
void Chip8::op_CXNN() {
	V[getX()] = (rand() % 256) & getNN();
}

// DXYN - DRW VX, VY, nibble : Display n-byte sprite starting at
// memory location I at (VX, VY), set VF = collision
void Chip8::op_DXYN() {
	uint8_t xPos{V[getX()]};
	uint8_t yPos{V[getY()]};
	uint8_t num_bytes{getN()};
	uint8_t byte{};

	V[0xFu] = 0;
	for (std::size_t row{0}; row < num_bytes; ++row) {
		byte = memory[I + row];
		for (std::size_t col{0}; col < 8; ++col) {
			if (byte & (0b10000000u >> col) != 0) {
				if (display[col + xPos + (yPos + row)*64] == 0xFFFFFFFFu) {
					V[0xFu] = 1;
				}

				display[col + xPos + (yPos + row)*64] ^= 0xFFFFFFFFu;
			}
		}

	}

}

// EX9E - SKP VX : Skip next instruction if key with the value of VX is pressed
void Chip8::op_EX9E() {
	if (keys[V[getX()]]) {
		pc += 2;
	}
}

// EXA1 - SKNP VX : Skip next instruction if key with the value VX is not pressed
void Chip8::op_EXA1() {
	if (!keys[V[getX()]]) {
		pc += 2;
	}
}

// FX07 - LD VX, DT : Set VX = delay timer value
void Chip8::op_FX07() {
	V[getX()] = delay_timer;
}

// FX0A - LD VX, K : Wait for a key press, store the value of the key in VX
void Chip8::op_FX0A() {
	bool wait{true};
	for (std::size_t i{0}; i < 16; ++i) {
		if (keys[i]) {
			V[getX()] = i;
			wait = false;
			break;
		}
	}
	if (wait) {
		pc -= 2;
	}
}

// FX15 - LD DT, VX : Set delay timer = VX
void Chip8::op_FX15() {
	delay_timer = V[getX()];
}

// FX18 - LD ST, VX : Set sound timer = VX
void Chip8::op_FX18() {
	sound_timer = V[getX()];
}

// FX1E - ADD I, VX : Set I = I + VX
void Chip8::op_FX1E() {
	I += V[getX()];
}

// FX29 - LD F, VX : Set I = location of sprite for digit VX
void Chip8::op_FX29() {
	I = 0x50 + 5*V[getX()];
}

// FX33 - LD B, VX : Store BCD representation of VX in memory locations I, I+1, I+2
void Chip8::op_FX33() {
	uint8_t num{V[getX()]};

	memory[I + 2] = num % 10;
	num /= 10;

	memory[I + 1] = num % 10;
	num /= 10;

	memory[I] = num % 10;
}

// FX55 - VD [I], VX : Store registers V0 through VX in memory starting at location I
void Chip8::op_FX55() {
	for (std::size_t i{0}; i <= getX(); ++i) {
		memory[I + i] = V[i];
	}
}

// FX65 - LD VX, [I] : Read registers V0 through VX from memory starting at location I
void Chip8::op_FX65() {
	for (std::size_t i{0}; i <= getX(); ++i) {
		V[i] = memory[I + i];
	}
}

// ----Table Functions---- //

void Chip8::null() {

}

void Chip8::table0() {
	op_table0[getN()]();
}

void Chip8::table8() {
	op_table8[getN()]();
}

void Chip8::tableE() {
	op_tableE[getN()]();
}

void Chip8::tableF() {
	op_tableF[getNN()]();
}


