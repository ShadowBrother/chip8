#include "chip8.h"
#include <iomanip>
#include <iostream>

using std::endl;

const byte Chip8::chip8_fontset[80] =
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

const std::string Chip8::keymap()
	{ //                                           0123456789ABCDEF
		static std::string* ret = new std::string("X123QWEASDZC4RFV"); 
		return *ret;
	}


Chip8::Chip8()
{
	initialize();

}

void Chip8::initialize()
{
	
	logg = new std::ostringstream();
	Log() << "Initializing ostring stream\n";
	std::cout << Log().str();
	pc = 0x200; //Program counter starts at 0x200
	opcode = 0;
	I = 0;
	sp = 0;

	int i = 0;
	//clear display
	for (i = 0; i < screen_size; i++)
	{
		gfx[i] = 0;
	}
	drawFlag = true;

	//clear registers
	for (i = 0; i < num_registers; i++)
	{
		V[i] = 0;
	}

	//load fontset and clear memory
	for (i = 0; i < fontset_size; i++){
		memory[i] =  chip8_fontset[i];
	}

	//clear memory
	for (; i < mem_size; i++)
	{
		memory[i] = 0;
	}

	//reset timers
	delay_timer = 0;
	sound_timer = 0;

	srand(time(NULL));//seed rand
	
}

//Chip8 Destuctor
Chip8::~Chip8()
{
	if (Log()){
		if (Log().good()){
			Log() << std::endl; //add endline to logg
			//open output file stream
			std::ofstream logFile;
			logFile.open("D:\\My Documents\\Visual Studio 2013\\Projects\\chip8\\chip8Log", std::ios::out);

			logFile << Log().str();//output logg string to file
			std::cerr << logFile.fail();

			logFile.close();
		}
	}
}

//returns the current program counter
dByte Chip8::getPc()
{
	return pc;
}

//returns opcode stored at memory location of current pc
dByte Chip8::getOpcode(dByte pc)
{
	return memory[pc] << 8 | memory[pc + 1];//opcode is 2 bytes, so must grab both halfs from memory
}

void Chip8::loadGame(char* game)
{
	dByte pc = 0x200;//where in memory to write incoming data
	std::ifstream infile;
	infile.open(game, std::ios::binary | std::ios::in);
	byte aByte = infile.get();
	if (infile.bad())
	{
		throw("Error reading file: %s\n", game);
	}
	while (infile.good())
	{
		memory[pc++] = aByte;//store game byte into memory, increment pc
		aByte = infile.get();//get next byte of data
	}
	infile.close();
}

std::ostringstream &Chip8::Log()
{
	return *logg;
}

void Chip8::emulateCycle()
{

	std::ios normalState(nullptr);//for saving/restoring stream formatting
	std::ios hexState(nullptr);//for setting to Hex formatting
	std::ios reg(nullptr);//for setting formatting for register numbers(0-F)
	std::ios regVal(nullptr);//for setting formatting for register values (00-FF)

	normalState.copyfmt(Log());//set normalState to normal state
	Log() << std::uppercase << std::hex << std::setw(4) << std::setfill('0');//formatting for hex output
	hexState.copyfmt(Log());//set hexState to hex formatting
	Log() << std::setw(2);
	regVal.copyfmt(Log());//set regVal formatting
	Log() << std::setw(1);
	reg.copyfmt(Log());//set reg formatting
	Log().copyfmt(normalState);
	
	//fetch opcode
	opcode = getOpcode(pc);//memory[pc] << 8 | memory[pc + 1];//opcode is 2 bytes, so must grab both halfs from memory
	
	Log().copyfmt(normalState);
	Log() << "pc: 0x";
	Log().copyfmt(hexState);//set to hex formatting
	Log() << opcode << std::endl;
	Log().copyfmt(normalState);//set to normal formatting

	printf("0x%X | 0x%X\n", memory[pc] << 8, memory[pc + 1]);
	printf("emCy opcode: 0x%X\n", opcode);
	//decode opcode

	switch (opcode & 0xF000)//switch on first 4 bits of opcode
	{
		//cases based on first 4 bits of opcode, some cases will need deeper switches
	case 0x0000://0x0NNN, 0x00E0, or 0x00EE
		switch (opcode & 0x0F00)
		{
		case 0x0000://0x00E0 or 0x00EE
			switch (opcode & 0x000F)
			{
			case 0x0000://0x00E0 Clear Screen
				printf("Clear Screen\n");
				Log() << "Clear Screen\n";

				for (int i = 0; i < screen_size; i++)
				{
					gfx[i] = 0;
				}
				drawFlag = true;//output changed, so set draw flag
				pc+= 2;
				break;
			case 0x000E: //0x00EE Returns from subroutine
				//return from subroutine
				printf("Return from subroutine\n");
				Log() << "Return from subroutine: ";
				try
				{
					pc = pop();//pop return address off stack

					Log().copyfmt(hexState);//set to hex formatting
					Log() << pc << endl;//log return address
					Log().copyfmt(normalState);//set to normal formatting
				}
				catch (char* e)
				{
					printf("%s\n", e);
					Log() << endl << e << endl;
					throw e;
				}
				pc += 2;//move to next instruction
				break;
			default:
				printf("Unknown opcode : 0x % X\n", opcode);
				break;
			}
			break;
		
		default: //0x0NNN calls RCA 1802 program at address NNN
			//call RCA 1802 program
			printf("call RCA 1802 program at adress 0x%X\n", opcode & addressMask);
			
			Log() << "call RCA 1802 program at adress 0x";
			Log().copyfmt(hexState);
			Log() << (opcode & addressMask) << endl;
			Log().copyfmt(normalState);

			break;
		}
		break; 
	case 0x1000://1NNN jumps to address NNN
		pc = opcode & addressMask;
		Log() << "jump to address ";
		Log().copyfmt(hexState);
		Log() << pc << endl;
		Log().copyfmt(normalState);

		break;
	case 0x2000://2NNN calls subroutine at NNN
		printf("push pc to stack 0x%X\n", pc);
		
				
		Log() << "push pc to stack 0x";
		Log().copyfmt(hexState);
		Log() << pc << endl;
		Log().copyfmt(normalState);
		
		
		try
		{
			push(pc);//push current program counter to stack
		}
		catch (char* e)
		{
			printf("%s\n", e);
			Log() << e << endl;
			throw e;
		}
		pc = opcode & addressMask;//set program counter to subroutine address
		printf("jump to subroutine at 0x%X\n", pc);
		
		Log() << "Jump to subroutine at 0x";
		Log().copyfmt(hexState);
		Log() << pc << endl;
		Log().copyfmt(normalState);

		break;
	case 0x3000://0x3XNN skips next instruction if VX equals NN
		Log() << "skips next instruction if VX equals NN" << endl;
		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & XRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() << "]: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VX) << endl;
		Log().copyfmt(normalState);
		Log() << "NN: 0x";
		Log().copyfmt(regVal);
		Log() << NN << endl;
		Log().copyfmt(normalState);

		if (VX == NN)
		{
			pc += 4;//skip next instruction
			Log() << "VX == NN, skipping next instruction." << endl;
		}
		else
		{
			pc += 2;//move to next instruction
			Log() << "VX != NN, move to next instruction." << endl;
		}
		break;
	case 0x4000://0x4XNN skips the next instruction if VX does NOT equal NN

		Log() << "skips the next instruction if VX does NOT equal NN" << endl;
		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & XRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() << "]: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VX) << endl;
		Log().copyfmt(normalState);
		Log() << "NN: 0x";
		Log().copyfmt(hexState);
		Log() << NN << endl;
		Log().copyfmt(normalState);

		if (VX != NN )
		{
			pc += 4;//skip next instruction
			Log() << "VX != NN, skipping next instruction." << endl;
		}
		else
		{
			pc += 2;//move to next instruction
			Log() << "VX == NN, move to next instruction." << endl;
		}
		break;
	case 0x5000://0x5XY0 skips the next instruction if VX equals VY
		Log() << "skips the next instruction if VX equals VY" << endl;
		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & XRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() <<"]: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VX) << endl;
		Log().copyfmt(normalState);
		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & YRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() << "]: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VY) << endl;
		Log().copyfmt(normalState);

		if (VX == VY)
		{
			pc += 4;//skip next instruction
			Log() << "VX == VY, skip next instruction." << endl;
		}
		else
		{
			pc += 2;//move to next instruction
			Log() << "VX != VY, move to next instruction." << endl;
		}
		break;
	case 0x6000://0x6XNN sets VX to NN
		Log() << "sets VX to NN" << endl;
		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & XRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() << "]: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VX) << endl;
		Log().copyfmt(normalState);
		Log() << "NN: 0x";
		Log().copyfmt(regVal);
		Log() << NN << endl;
		Log().copyfmt(normalState);

		VX = NN;

		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & XRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() << "] after: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VX) << endl;
		Log().copyfmt(normalState);

		pc += 2;
		break;
	case 0x7000://0x7XNN adds NN to VX
		Log() << "adds NN to VX" << endl;
		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & XRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() << "]: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VX) << endl;
		Log().copyfmt(normalState);
		Log() << "NN: 0x";
		Log().copyfmt(regVal);
		Log() << NN << endl;
		Log().copyfmt(normalState);

		VX += NN;
		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & XRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() << "] after: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VX) << endl;
		Log().copyfmt(normalState);

		pc += 2;
		break;
	case 0x8000://0x8XY0-0x8XY7 or 0x8XYE
		switch (opcode & 0x000F)
		{
		case 0x0000://0x8XY0 sets VX to the value of VY

			Log() << "sets VX to the value of VY" << endl; 
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & YRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VY) << endl;

			VX = VY;
			
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "] after: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log().copyfmt(normalState);

			pc += 2;
			break;
		case 0x0001://0x8XY1 sets VX to VX | VY

			Log() << "sets VX to the value of VX | VY" << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & YRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VY) << endl;

			VX = VX | VY;

			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "] after: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log().copyfmt(normalState);

			pc += 2;
			break;
		case 0x0002://0x8XY2 sets VX to VX & VY

			Log() << "sets VX to the value of VX & VY" << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & YRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VY) << endl;

			VX = VX & VY;

			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "] after: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log().copyfmt(normalState);

			pc += 2;
			break;
		case 0x0003://0x8XY3 sets VX to VX XOR VY
			Log() << "sets VX to the value of VX XOR VY" << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & YRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VY) << endl;

			VX = VX ^ VY;

			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "] after: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log().copyfmt(normalState);

			pc += 2;
			break;
		case 0x0004://0x8XY4 adds VY to VX. VF is set to 1 when there's a carry, else 0

			Log() << "adds VY to VX" << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & YRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VY) << endl;

			if (VY > (0xFF - VX))//overflow
			{
				Log() << "overflow, VCARRY set to 1" << endl;
				VCARRY = 1;
			}
			else
			{
				VCARRY = 0;
			}
			VX += VY;

			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "] after: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log().copyfmt(normalState);

			pc += 2;//increment pc
			break;
		case 0x0005://0x8XY5 VY is subtracted from VX. VF is set to 0 when there's a borrow, else 1

			Log() << "subtracts VY from VX" << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & YRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VY) << endl;

			if (VY > VX){//if VY is larger than VX, requires borrow

				Log() << "Borrow occurred, VCARRY set to 0" << endl;
				VCARRY = 0;
			}
			else
			{
				VCARRY = 1;
			}
			VX -= VY;

			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "] after: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log().copyfmt(normalState);


			pc += 2;//increment pc
			break;
		case 0x0006://0x8XY6 shifts VX right by one. VF is set to value of least significant bit of VX prior to shift
			Log() << "shifts VX right by one, VCARRY is set to least sig bit of VX prior to shift" << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;

			VCARRY = VX & 0x1;//set carry to least sig bit
			Log() << "VCARRY: 0x" << VCARRY <<endl;
			VX = VX >> 1;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;

			pc += 2;
			break;
		case 0x0007://0x8XY7 sets VX to VY minus VX. VF is set to 0 when borrow, else 1

			Log() << "sets vX to VY minus VX. VF set to 0 when borrow." << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & YRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VY) << endl;

			if (VX > VY)//borrow
			{
				Log() << "borrow required, VCARRY set to 0" << endl;
				VCARRY = 0;
			}
			else
			{
				Log() << "no borrow required, VCARRY set to 1" << endl;
				VCARRY = 1;
			}

			VX = VY - VX;

			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;

			pc += 2;
			break;
		case 0x000E://0x8XYE shifts VX left by one. VF is set to most significant bit of VX prior to shift

			Log() << "shifts VX left by one, VCARRY is set to most sig bit of VX prior to shift" << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;

			VCARRY = (VX & 0x80) >> 7;//most sig bit

			Log() << "VCARRY: 0x" << VCARRY << endl; 

			VX = VX << 1;//shift left

			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;

			pc += 2;//increment pc
			break;
		default:
			printf("Unrecognized opcode 0x%X\n", opcode);
			break;
		}
		break;
	case 0x9000://0x9XY0 skips next instruction if VX doesn't equal VY

		Log() << "skips next instruction if VX != VY" << endl;
		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & XRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() << "]: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VX) << endl;
		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & YRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() << "]: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VY) << endl;

		if (VX != VY)
		{
			Log() << "VX != VY, skipping next instruction." << endl;
			pc += 4;//skip next instuction
		}
		else
		{
			pc += 2;//move to next instruction
		}
		break;
	case 0xA000://0xANNN sets I to address NNN
		Log() << "sets I to address NNN" << endl;
		Log().copyfmt(hexState);
		Log() << "I: 0x" << I << endl;
		Log() << "NNN: 0x" << dByte(opcode & addressMask) << endl;

		I = opcode & addressMask;

		Log() << "I: 0x" << I << endl;

		pc += 2;
		break;
	case 0xB000://0xBNNN jumps to address NNN plus V0

		Log() << "jumps to address NNN + V0" << endl;
		Log().copyfmt(hexState);
		Log() << "NNN: 0x" << dByte(opcode & addressMask) << endl;
		Log() << "V0: 0x" << V[0] << endl;

		pc = (opcode & addressMask) + V[0];

		Log() << "pc: 0x" << pc << endl;

		break;
	case 0xC000://0xCXNN sets VX to a random number, masked by NN

		Log() << "sets VX to a random number, masked by NN" << endl;
		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & XRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() << "]: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VX) << endl;
		Log() << "NN: 0x" << dByte(opcode & 0xFF) << endl;

		VX = (rand() % 0xFF) & (opcode & 0xFF);//get random number between 0 and 0xFF, mask it with NN

		Log() << "V[";
		Log().copyfmt(reg);
		Log() << ((opcode & XRegMask) >> 8);
		Log().copyfmt(normalState);
		Log() << "]: 0x";
		Log().copyfmt(regVal);
		Log() << dByte(VX) << endl;

		pc += 2;
		break;
	case 0xD000://0xDXYN sprites stored in memory at location in I, max 8bits wide. Wraps around screen.
		//if when drawn, clears a pixel, register VF is set to 1, otherwise 0. All drawing is XOR drawing.
		//draws at coordinate (VX, VY) with height N, width 8
		
		{
			Log() << "Draw Sprite" << endl;
			

			int x = dByte(VX);
			int y = dByte(VY);
			int height = opcode & 0xF;//N
			VCARRY = 0;
			int gIndex = x + y * screen_width;//starting graphics point
			byte sRow;//sprite row

			Log() << "x = " << x << endl << "y = " << y << endl << "height = " << height << endl << "gIndex = " << gIndex << endl;

			for (int i = 0; i < height; i++)//height number of rows
			{
				Log() << "i = " << i << endl;
				sRow = memory[I + i];
				Log().copyfmt(hexState);
				Log() << "sRow = " << dByte(sRow) << endl;
				for (int j = 0; j < 8; j++)//number of columns
				{					
					Log().copyfmt(normalState);
					Log() << "j = " << j << endl; 
					Log().copyfmt(hexState);

					// shifting a 1 over 7-j places to extract the bit of sRow we need, then shifting it back to least sig bit, & with 1 since >> doesn't guarantee 0's
					byte newPixel = (((sRow & (1 << (7 - j))) >> (7 - j)) & 1);

					Log() << "newPixel: " << dByte(newPixel) << endl;

					newPixel = (gfx[gIndex + j + i * screen_width]) ^ newPixel;//newPixel is XOR of gfx pixel and sprite pixel

					Log() << "gfx pixel: " << dByte(gfx[gIndex + j + i * screen_width]) << endl;
					Log() << "newPixel after XOR: " << dByte(newPixel) << endl;

					if ((newPixel != gfx[gIndex + j + i * screen_width]) && (newPixel == 0))//pixel has been flipped AND UNSET
					{
						VCARRY = 1;
						Log() << "pixel has been unset, VCARRY set to 1" << endl;
					}
					if ((newPixel > 1) || (newPixel < 0)){
						Log() << "newPixel out of bounds!: " << dByte(newPixel) << endl;
						throw "ERROR: newPixel is out of bounds! " ;
					}
					gfx[gIndex + j + i * screen_width] = newPixel;//set new pixel

				}
			}
			drawFlag = true;
			pc += 2;
		}
		break;
	case 0xE000://0xEx9E or 0xEXA1
		switch (opcode & 0x000F)
		{
		case 0x000E://0xEX9E skips next instruction if key stored in VX is pressed
			if (key[VX] != 0)//key is pressed
			{
				pc += 4;//skip next instruction
			}
			else
			{
				pc += 2;//move to next instruction
			}
			break;
		case 0x0001://0xEXA1 skips the next instruction if the key stored in VX is NOT pressed
			if (key[VX] == 0)//key is not pressed
			{
				pc += 4;//skip next instruction
			}
			else
			{
				pc += 2;//move to next instruction
			}
			break;

		default:
			printf("Unrecognized opcode 0x%X\n", opcode);
			break;
		}
		break;
	case 0xF000://one of several possible opcodes starting with 0xF
		switch (opcode & 0x00FF)
		{
		case 0x0007://0xFX07 sets VX to the value of the delay timer

			Log() << "sets VX to delay timer" << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log() << "delay_timer: " << delay_timer << endl;

			VX = delay_timer;

			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;

			pc += 2;
			break;
		case 0x000A://0xFX0A A key press is awaited, and then stored in VX
		{
			bool keyPressed = false;
			for (int i = 0; i < num_keys; i++)
			{
				if (key[i] == 1){
					keyPressed = true;
					VX = byte(i);
				}
			}
			if (!keyPressed)//key not pressed, don't increment pc, loop on same opcode again
			{
				return;
			}
			//int key = 0;
			//char c;
			//printf("get keypress\n");
			//do
			//{
			//	c = getchar();//get key press
			//	key = getKeymap(c);//uses keymap to get chip8 hex key from keyboard key
			//} while (key == -1);//if key pressed was not a mapped key, wait for valid keypress

			//printf("key %c mapped to key %d\n", c, key);
			//VX = (byte)key;//store key press in VX
			//printf("VX: 0x%X\n", VX);
		}
		pc += 2;
			break;
		case 0x0015://0xFX15 sets delay timer to VX
			delay_timer = VX;
			pc += 2;
			break;
		case 0x0018://0xFX18 sets the sound timer to VX
			sound_timer = VX;
			pc += 2;
			break;
		case 0x001E://0xFX1E adds VX to I

			Log() << "adds VX to I, sets VCARRY to 1 on range overflow" << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log() << "I: 0x" << I << endl;

			if ((I + dByte(VX)) > 0xFFF)//range overflow
			{
				VCARRY = 1;
				Log() << "I + VX overflows I, VCARRY set to 1" << endl;
			}
			else
			{
				VCARRY = 0;
			}
			I += VX;

			Log() << "I: 0x" << I << endl;

			pc += 2;
			break;
		case 0x0029://0xFX29 sets I to location of sprite for char in VX. char 0-F represented by 4x5 font
			Log() << "Set I to location of sprite for char in VX." << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: 0x";
			Log().copyfmt(regVal);
			Log() << dByte(VX) << endl;
			Log() << "I before: " << I << endl;

			I = dByte(VX) * 0x5;//offset of 0x5
			
			Log() << "I: " << I << endl;

			pc += 2;
			break;
		case 0x0033://0xFX33 stores Binary-coded decimal representation of VX, with most sig of 3 digits
			//at the address in I, the middle digit at I + 1, the least sig digit at I + 2.
			Log() << "stores Binary-coded decimal representation of VX with most sig digit at mem address starting at I " << endl;
			Log() << "V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]: ";
			
			Log() << int(VX) << endl;


			memory[I] = (byte)((int)VX / 100);
			memory[I + 1] = (byte)(((int)VX / 10) % 10);
			memory[I + 2] = (byte)((int)VX % 10);

			Log() << "memory[";
			Log().copyfmt(hexState);
			Log() << I << "]: ";
			Log().copyfmt(normalState);
			Log() << int(memory[I]) << endl;
			Log() << "memory[";
			Log().copyfmt(hexState);
			Log() << I + 1 << "]: ";
			Log().copyfmt(normalState);
			Log() << int(memory[I + 1]) << endl;
			Log() << "memory[";
			Log().copyfmt(hexState);
			Log() << I + 2 << "]: ";
			Log().copyfmt(normalState);
			Log() << int(memory[I + 2]) << endl;

			pc += 2;
			break;
		case 0x0055://0xFX55 stores V0 to VX in memory starting at address I

			Log() << "stores registers V0 to VX in memory starting at address I" << endl;
			Log() << "Storing registers from V[0] to V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]" << endl;
			
			for (int i = 0; i <= int((opcode & XRegMask) >> 8); i++)
			{

				Log() << "V[";
				Log().copyfmt(reg);
				Log() << i;
				Log().copyfmt(normalState);
				Log() << "]: 0x";
				Log().copyfmt(regVal);
				Log() << dByte(VX) << endl;

				memory[I + i] = V[i];

				Log() << "memory[";
				Log().copyfmt(reg);
				Log() << I + i;
				Log().copyfmt(normalState);
				Log() << "]: 0x";
				Log().copyfmt(regVal);
				Log() << dByte(memory[I + i]) << endl;
			}
			pc += 2;
			break;
		case 0x0065://0xFX65 fills V0 to VX with values from memory starting at address I

			Log() << "fills registers V0 to VX from memory starting at address I" << endl;
			Log() << "filling registers from V[0] to V[";
			Log().copyfmt(reg);
			Log() << ((opcode & XRegMask) >> 8);
			Log().copyfmt(normalState);
			Log() << "]";

			for (int i = 0; i <= int((opcode & XRegMask)>> 8); i++)
			{
				Log() << "V[";
				Log().copyfmt(reg);
				Log() << i;
				Log().copyfmt(normalState);
				Log() << "]: 0x";
				Log().copyfmt(regVal);
				Log() << dByte(VX) << endl;

				V[i] = memory[I + i];

				Log() << "V[";
				Log().copyfmt(reg);
				Log() << i;
				Log().copyfmt(normalState);
				Log() << "]: 0x";
				Log().copyfmt(regVal);
				Log() << dByte(VX) << endl;
			}
			pc += 2;
			break;
		default:
			printf("Unrecognized opcode 0x%X", opcode);
			break;
		}
		break;

	default:
		printf("Unknown opcode: 0x%X\n", opcode);
		break;
	}

	//update timers
	if (delay_timer > 0)
	{
		--delay_timer;
	}
	if (sound_timer > 0)
	{
		printf("\a");//beep
		--sound_timer;
	}

}

void Chip8::accessMemory(dByte index, dByte opcode){
	printf("accMem opcode: 0x%X\n", opcode);
	memory[index] = (byte)((opcode & 0xFF00) >> 8) & 0x00FF;
	printf("0x%X\n", (byte)((opcode & 0xFF00) >> 8) & 0x00FF);
	memory[index + 1] = (byte)(opcode & 0x00FF);
	printf("0x%X & 0x%X = 0x%X\n", opcode, 0x00FF, (opcode & 0x00FF));
	
	printf("0x%X\n", memory[index] << 8 | memory[index + 1]);
}

void Chip8::debugPrint()
{
	printf("Screen: \n");
	int i, j;
	for (i = 0; i < 32; i++)
	{
		putchar(10);
		for (j = 0; j < 64; j++)
		{
			if (gfx[j + i * 64] == 1)
			{
				putchar(42);//*
			}
			else
			{
				putchar(32);
			}
		}
	}
	printf("\n");
}