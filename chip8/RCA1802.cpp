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
			toBranch = !toBranch;//NOT result
		}

		if (toBranch)
		{//branch 
			chip8->pc = (--(chip8->pc) & 0xFF00) | chip8->memory[chip8->pc];//short branch only changes least sig byte of pc
		}
		else
		{//skip to next instruction
			chip8->pc++;
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
			//add memory byte pointed to by the address register pointed to by X, the value of DF register and accumulator
			byte temp = chip8->memory[R[X]] + DF;//RX + DF
			
			if (temp > (0xFF - D))//check if addition will overflow byte
			{
				DF = 0x1;
			}
			else
			{
				DF = 0x0;
			}
			
			D += temp;//perform addition
			
		}
		if (N == 5)//SDB Subtract D from memory with Borrow
		{
			
			byte temp = chip8->memory[R[X]] + DF - 1; // adjust memory value with Borrow.DF == 0 means borrow(subtract 1), DF == 1 means no borrow
			if (temp < D)//will borrow
			{
				DF = 0;
			}
			else
			{
				DF = 1;
			}
			D = temp - D;
			
		}
		if (N == 6)//SHRC Shift D Right with Carry
		{
			//shift D right 1 position, putting least sig bit in DF, first storing DF to move into most sig bit
			byte lsb = D & 0x1;//store least sig bit of D
			D = D >> 1;//shift D right 1
			D = D | (DF << 7) ;//shift DF left 7 to align with most sig bit of D, | with D
			DF = lsb;//move D's original least sig bit to DF
		}
		if (N == 7)//SMB Subtract Memory from D with Borrow 
		{
			
			D += DF - 1;//adjust D with Borrow. DF == 0 means borrow(subtract 1), DF == 1 means no borrow 
			if (chip8->memory[R[X]] > D)//will borrow
			{
				DF = 0;
			}
			else
			{
				DF = 1;
			}
			D -= chip8->memory[R[X]] ;
			
		}
		if (N == 8)//SAV SAVe T
		{
			//copy contents of the T register into the memory location pointed to by the address register pointed to by X
			chip8->memory[R[X]] = T;
		}
		if (N == 9)//MARK Save X and P in T
		{
			T = (X << 4) | P;// Store X as the highest 4 bits of T, P as lowest 4 bits
		}
		if (N == 0xA)//REQ REset Q
		{
			//turn the Q flip-flop in the 1802 off, and set the Q output pin low
			Q = false;
		}
		if (N == 0xB)//SEQ SEt Q
		{
			//Set the Q flip-flop in the 1802 on, and set the Q output pin high
			Q = true;
		}
		if (N == 0xC)//ADCI b ADd with Carry Immediate
		{
			//Add the memory byte pointed to by the second byte of the instruction plus value of DF to D
			byte temp = chip8->memory[chip8->pc++] + DF;//DF + second byte of instruction, increment pc for next instruction
			
			if (temp > (0xFF - D))//check if addition will overflow byte
			{
				DF = 0x1;
			}
			else
			{
				DF = 0x0;
			}

			D += temp;//perform addition
			
		}
		if (N == 0xD)//SDBI b Subtract D from Immediate byte with Borrow
		{
			byte temp = chip8->memory[chip8->pc++] + DF - 1;//adjust memory value with Borrow. DF == 0 means borrow(subtract 1), DF == 1 means no borrow 
			if (temp < D)//will borrow
			{
				DF = 0;
			}
			else
			{
				DF = 1;
			}
			D = temp - D;//subtract next byte from D, increment pc to next instruction
			
		}
		if (N == 0xE)//SHLC SHift Left with Carry
		{
			//Shift bits of D left 1 position, storing most sig bit in DF after original DF is placed in least sig bit of D
			byte msb = D & 0x80; //store most sig bit of D
			D = (D << 1) | DF;//shift D left 1 and | with DF
			DF = msb;
		}
		if (N == 0xF)//SMBI b Subtract Memory Immediate from D, with Borrow
		{
			D += DF - 1;//adjust D with Borrow. DF == 0 means borrow(subtract 1), DF == 1 means no borrow 
			byte temp = chip8->memory[chip8->pc++]; //store next mem byte, increment pc to next instruction
			if (temp > D)//will borrow
			{
				DF = 0;
			}
			else
			{
				DF = 1;
			}
			D -= temp;//subtract next byte from D
			
		}
	}
	break;
	case 8://GLO Get LOw Byte of RN
	{
		//Copy the least significant 8 bits of the specified register into D
		D = chip8->memory[R[N]] & 0xFF;
	}
	break;
	case 9://GHI Get High byte of register RN
	{
		//Copy the most significant 8 bits of specified register into D
		D = chip8->memory[R[N]] & 0xFF00;
	}
	break;
	case 0xA://PLO Put D into LOw byte of register
	{
		//Copy D into the least significant 8 bits of the specified register
		chip8->memory[R[N]] = (chip8->memory[R[N]] & 0xFF00) | D;
		
	}
	break;
	case 0xB://PHI Put D into HIgh byte of register
	{
		//Copy D into the most significant 8 bits the specified register
		chip8->memory[R[N]] = (chip8->memory[R[N]] & 0xFF) | ((dByte)D << 8);
	}
	break;
	case 0xC://Long Branches/Skips
	{
		if (N == 4)//NOP No Operation
		{
			break;
		}

		bool toBranch = false;
		bool toSkip = true;

		//3 byte Branch Operations
		if (N == 0 || N == 8)//unconditional
		{
			toBranch = true;
		}
		
		if (N == 1 || N == 9)//Branch on Q
		{
			toBranch = Q;
		}
		if (N == 2 || N == 0xA)//Branch on Zero
		{
			if (D == 0)
			{
				toBranch = true;
			}
		}
		if (N == 3 || N == 0xB)//Branch on DF
		{
			if (DF == 1)
			{
				toBranch = true;
			}
		}
		if (N >= 8 && N <= 0xB)//inverse conditionals 
		{
			toBranch = !toBranch;
		}
		//1 byte Skip Operations

		if (N == 5 || N == 0xD)//Skip on Q
		{
			toSkip = Q;
		}
		if (N == 6 || N == 0xE)//Skip on Zero
		{
			if (D == 0)
			{
				toSkip = true;
			}
		}
		if (N == 7 || N == 0xF)//Skip on DF
		{
			if (DF == 1)
			{
				toSkip = true;
			}
		}

		if (N >= 5 && N <= 7)//inverse conditionals
		{
			toSkip = !toSkip;
		}

		if (N == 0xC)//Long Skip if Interrupts Enabled
		{
			toSkip = IE;
		}

		if (toBranch)//branch using next 2 bytes
		{
			chip8->pc = dByte((chip8->memory[chip8->pc]) << 8) | dByte(chip8->memory[chip8->pc + 1]);//Set next 2 bytes as pc
		}
		else if (toSkip)//Skip 2 bytes
		{
			chip8->pc += 2;
		}

	}
	break;
	case 0xD://SEP SEt P Register
	{
		//Set P to point to Register N
		P = N;
	}
	break;
	case 0xE://SEX SEt X Register
	{
		//Set X to point to Register N
		X = N;
	}
	break;
	case 0xF://Logic, Arithmetic, Logic Immediate, Arithmatic Immediate
	{
		if (N == 0)//LDX Load D via RX
		{
			//Load accumulator from memory byte pointed to by RX
			D = chip8->memory[R[X]];
		}
		
		if (N == 1)//OR  Logical OR
		{
			//Set D to the Logical OR of D and memory pointed to by RX
			D = D | chip8->memory[R[X]];
		}

		if (N == 2)//AND Logical AND
		{
			//Set D to the logical AND of D and memory pointed to by RX
			D = D & chip8->memory[R[X]];
		}

		if (N == 3)//XOR eXclusive OR
		{
			//Set D to the exclusive OR of D and memory pointed to by RX
			D = D ^ chip8->memory[R[X]];
		}

		if (N = 4)//ADD Add
		{
			//Set D to the addition of D and memory pointed to by RX, Set DF to 1 if carry
			if (chip8->memory[R[X]] > 0xFF - D)//carry
			{
				DF = 1;
			}
			else
			{
				DF = 0;
			}

			D += chip8->memory[R[X]];
		}

		if (N == 5)//SD Subtract D from memory
		{
			//Subtract accumulator from byte in memory pointed to by RX
			if (D > chip8->memory[R[X]])//check if result will be negative
			{
				DF = 0;
			}
			else
			{
				DF = 1;
			}
			D = chip8->memory[R[X]] - D;
		}

		if (N == 6)//SHR SHift D Right
		{
			//Shift bits of D right 1 place, first storing least sig bit of D in DF
			DF = D & 1;
			D = D >> 1;
		}

		if (N == 7)//SM Subract Memory byte from D
		{
			//Subtract byte from memory pointed to by RX from D, store carry out in DF
			if (chip8->memory[R[X]] > D){//check if result will be negative
				DF = 0;
			}
			else
			{
				DF = 1;
			}
			D -= chip8->memory[R[X]];
		}

		if (N == 8)//LDI Load D Immediate
		{
			//Copy the second byte of the instruction into D
			D = chip8->memory[chip8->pc++];//Set D and increment pc
		}

		if (N == 9)//ORI  Logical OR Immediate
		{
			//Set D to the Logical OR of D and byte following instruction
			D = D | chip8->memory[chip8->pc++];
		}

		if (N == 0xA)//ANI Logical ANd Immediate
		{
			//Set D to the logical AND of D and byte following instruction
			D = D & chip8->memory[chip8->pc++];
		}

		if (N == 0xB)//XRI eXclusive oR Immediate
		{
			//Set D to the exclusive OR of D and byte following instruction
			D = D ^ chip8->memory[chip8->pc++];
		}

		if (N == 0xC)//ADI ADd Immediate
		{
			//Set D to the sum of D and next byte of instruction, put carry in DF
			if (chip8->memory[chip8->pc] > 0xFF - D)//test if carry
			{
				DF = 1;
			}
			else
			{
				DF = 0;
			}
			D += chip8->memory[chip8->pc++];//Set D and increment pc
		}

		if (N == 0xD)//SDI Subtract D from Immediate byte
		{
			//Subtract D from second byte of instruction, storing result in D
			if (D > chip8->memory[chip8->pc])//test if result will be negative
			{
				DF = 0;
			}
			else
			{
				DF = 1;
			}
			D = chip8->memory[chip8->pc] - D;
		}

		if (N == 0xE)//SHL SHift D Left
		{
			//Shift bits of D left 1 pos, first storing most sig bit of D in DF
			DF = D & 0x80;
			D = D << 1;
		}

		if (N == 0xF)//SMI Subtract Memory from D, Immediate
		{
			//Subtract Memory byte pointed to by RX from D, store in D
			if (D < chip8->memory[R[X]])
			{
				DF = 0;
			}
			else
			{
				DF = 1;
			}
			D -= chip8->memory[R[X]];
		}

	}
	};
};