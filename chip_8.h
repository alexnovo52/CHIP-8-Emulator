#include <cstdint>

class Chip8 {
private:
	// Default initialization resets system and clears memory
	uint8_t V[16]{};
	uint8_t memory[4096]{};
	uint16_t I{};
	uint16_t pc{};
	uint16_t stack[16]{};
	uint8_t sp{};
	uint8_t delay_timer{};
	uint8_t sound_timer{};
	uint16_t opcode{};

	// Helper functions
	uint8_t getX();
	uint8_t getY();
	uint8_t getN();
	uint8_t getNN();
	uint16_t getNNN();

	// Opcodes
	void op_00E0();
	void op_00EE();
	void op_1NNN();
	void op_2NNN();
	void op_3XNN();
	void op_4XNN();
	void op_5XY0();
	void op_6XNN();
	void op_7XNN();
	void op_8XY0();
	void op_8XY1();
	void op_8XY2();
	void op_8XY3();
	void op_8XY4();
	void op_8XY5();
	void op_8XY6();
	void op_8XY7();
	void op_8XYE();
	void op_9XY0();
	void op_ANNN();
	void op_BNNN();
	void op_CXNN();
	void op_DXYN();
	void op_EX9E();
	void op_EXA1();
	void op_FX07();
	void op_FX0A();
	void op_FX15();
	void op_FX18();
	void op_FX1E();
	void op_FX29();
	void op_FX33();
	void op_FX55();
	void op_FX65();

	// Tables & Table Functions
	typedef void (Chip8::*opcodeFunc)();
	opcodeFunc op_table[0xF + 1]{&Chip8::null};
	opcodeFunc op_table0[0xE + 1]{&Chip8::null};
	opcodeFunc op_table8[0xE + 1]{&Chip8::null};
	opcodeFunc op_tableE[0xE + 1]{&Chip8::null};
	opcodeFunc op_tableF[0x65 + 1]{&Chip8::null};

	void null();
	void table0();
	void table8();
	void tableE();
	void tableF();


public:
	// Default initialization resets keys and graphics
	uint8_t keys[16]{};
	uint32_t display[64*32]{};

	Chip8();

	void loadROM(char const* filename);
	void emulateCycle();
};
