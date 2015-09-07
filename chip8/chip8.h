//chip8.h header file for chip8 cpu implementation
//Jesse Hoyt - jesselhoyt@gmail.com

#ifndef CHIP8_H
#define CHIP8_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>

#define mem_size 4096 //4k memory
#define screen_width 64
#define screen_height 32
#define screen_size screen_width * screen_height //screen 64 X 32 pixels
#define stack_size 16 //16 level stack
#define fontset_size 80 
#define num_registers 16 //16 registers V0-VF
#define num_keys 16 //16 input keys

//define often used masks for opcodes
#define addressMask 0x0FFF
#define XRegMask 0x0F00
#define YRegMask 0x00F0

#define VX V[(opcode & XRegMask) >> 8]
#define VY V[(opcode & YRegMask) >> 4]
#define VCARRY V[0xF]
#define NN (opcode & 0x00FF)



typedef unsigned char byte;
typedef unsigned short dByte;//Double Byte(2 bytes = 16 bits, size of opcodes)

class Chip8
{
private:
	std::ostringstream *logg;

	dByte opcode; //current opcode
	byte memory[mem_size]; //4k memory
	byte V[num_registers]; //16 registers, 16th register is the 'carry flag'
	dByte I; //address register
	dByte pc; //program counter
	
	byte delay_timer; //Timer for timing game events
	byte sound_timer; //beeping sound played when sound timer value is nonzero
	dByte stack[stack_size]; //stack for storing return adresses when calling subroutines
	dByte sp; //stack pointer
	
	
	
	inline void push(dByte address)
	{ 
		if (sp > stack_size){
			throw("Stack full!");
		}
		else
		{
			stack[sp] = address;
			++sp;
		}
	};

	inline dByte pop(){
		if (sp > 0)//check that stack is not empty
		{ --sp; return stack[sp]; }
		else{ throw("Tried to pop from empty stack!"); }
	};
	
	static const byte chip8_fontset[80];
	

	
public:
	Chip8();
	virtual ~Chip8();

	
	std::ostringstream &Log();
	byte gfx[screen_size]; //pixel array, 1 pixel is set, 0 pixel is not set
	void initialize(std::ostringstream *oss); //initalize registers and memory
	void emulateCycle(); //fetch, decode, execute opcode, update timers
	void loadGame(char* game); //load game into memory
	void accessMemory(dByte index, dByte opcode);
	void debugPrint();
	dByte getPc();
	dByte getOpcode(dByte pc);
	bool drawFlag; //flag set to true if screen should be updated
	byte key[num_keys]; //array to store keyboard key states
	static const std::string keymap();//used with getKeymap to return chip8 hex keys 0x0-0xF
	static int getKeymap(char c){//takes input key char, returns chip8 key 0-0xF or -1 if not a mapped key
		printf("km c: %c\n", c);
		size_t key = keymap().find(toupper(c));
		printf("km key: %d\n", key);
		if (key != std::string::npos){ return key; }
		else { return -1; }
	};
};

#endif