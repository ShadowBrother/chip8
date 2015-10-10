#include "RCA1802.h"


void RCA1802::emulateCycle(){

	//get I and N from memory, both used to determine Instruction
	I = (chip8->memory[chip8->pc] & 0xF0) >> 4;//I is high 4-bits
	N = chip8->memory[chip8->pc] & 0x0F;//N is low 4-bits
	chip8->pc++;//increment pc

	switch (I){//switch on Instruction register
	case 0://IDL or LDN
	{
		if (N == 0){//IDL idle
			IDL = true;//set idle to true and end cycle
			return;
		}
		else//LDN Load Via N
		{
			//copy the memory byte pointed to by the specified address register into the accumulator
			D = chip8->memory[R[N]];
		}
	}
	break;
	case 1://INC increment RN
	{
		R[N]++;
	}
	break;
	case 2://DEC decrement RN
	{
		R[N]--;
	}
	break;
	case 3://Branches
	{
		bool toBranch = false;
		if ((N <= 0x3) || ((N >= 0x8) && (N <= 0xB)))//Branch on Q,Z, DF 
		{
			if (N == 0x0 || N == 0x8)//unconditional branch
			{
				toBranch = true;
			}
			if (N == 0x1 || N == 9){//branching based on Q
				toBranch = Q;
			}
			else if (N == 0x2 || N == 0xA)//branching based on whether D is Zero
			{
				if (D == 0x0){ toBranch = true; }
				else{ toBranch = false; }
			}
			else{//branching on DF
				if (DF == 0x1){ toBranch = true; }
				else{ toBranch = false; }
			}
		}
		else//Branch on EF1-4
		{
			toBranch = EF[N % 4];//use modulus 4 to get EF0-4 since EF branches are all offset by 4 or 12
		}
		if (N >= 0x8)//Branch NOTs
		{
			toBranch != toBranch;//NOT result
		}

		if (toBranch)
		{//branch aka skip
			chip8->pc += 2;
		}
	}
	break;
	case 4://LDA Load-Advance RN
	{
		//copy the memory byte pointed to by the specified address register into the accumulator
		D = chip8->memory[R[N]];
		R[N]++;//advance RN
	}
	break;
	case 5://STR Store Via RN
	{
		//using the specified address register, store contents of accumulator into memory
		chip8->memory[R[N]] = D;
	}
	break;
	case 6://IRX, Output, Input
	{
		if (N == 0)//IRX
		{
			//Increment the register pointed to by X
			R[X]++;
		}
		if ((N >= 0x1) && (N <= 0x7))//OUT 
		{
			//output on port p the memory byte pointed to by the address register pointed to by X,
			//then increment the address register
			p[N] = chip8->memory[R[X]++]; 

		}
		if ((P >= 0x9) && (N <= 0xF))//INP
		{
			//input from port p a byte to be stored into the memory location pointed to by the address register
			//pointed to by X and also place the byte into D
			D = chip8->memory[R[X]] = p[N];
		}

	}
	break;
	case 7://Control & Memory Ref., Arithmetic w/carry, control, Arith. immdediate w/carry
	{
		if (N <= 1)// RET return or DIS
		{
			//read the byte from the memory location pointed to by the register pointed to by X and increment that register
			byte temp = chip8->memory[R[X]++];
			//copy right 4 bits read into P, and left 4 bits into X
			P = temp & 0xF;
			X = (temp & 0xF0) >> 4;
			IE = true;//enable interupt
		}
		if (N == 1)//DIS return and disable interrupts
		{
			//Return already handled above, disable interrupt
			IE = false;
		}
		if (N == 2) //LDXA Load D via RX and Advance
		{
			//Load accumulator from memory byte pointed to by the address register pointed to by X and increment register
			D = chip8->memory[R[X]++];
		}
		if (N == 3)//STXD Store Via RX and Decrement RX
		{//Store the accumulator into the memory location pointed to by the address register pointed to by X and decrement 
			chip8->memory[R[X]--] = D;
		}
		if (N == 4)//ADC add with carry
		{

		}
	}
	break;

	};
};