#ifndef _RCA1802
#define _RCA1802


//RCA1802 microprocessor emulator
//references: http://www.atarimagazines.com/computeii/issue3/page52.php, http://cosmacelf.com/publications/books/short-course-in-programming.html#chapter2

typedef unsigned char byte;
typedef unsigned short dByte;


class RCA1802
{
private:

	byte* memory;//system memory
	byte D;//Accumulator, used for operating on data
	dByte R[16];//16 16-bit address registers
	byte P;//a "4 bit" register that points to the register which contains the PC(program counter)
	byte X;//"4-bit" register that points to an address register
	byte T;//Register for temporary storage of P and X
	byte I;//"4-bit" register that holds instruction
	byte N;//"4-bit" register used in instruction handling, such as determining which register is being operated on
	byte DF;//"1-bit" carry bit
	bool IE;//"1-bit" Interrupt Enable flag. True: Interrupts enabled, False: Interrupts disabled
	bool EF[4];//4 External Flags
	bool Q;//led true:on or false:off
	byte p[16];//input/output ports

	bool IDL; //true for idle, false for running
	
public:
	RCA1802();//will allocate 4k of memory
	RCA1802(int numBytes);//will allocate numBytes of memory
	RCA1802(byte* memory);//will use existing byte array as memory
	void initialize();//initialize registers
	void emulateCycle();//emulate one clock cycle

};




#endif