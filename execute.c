#include "chip8.h"
#include <bool.h>

/*
	Instructions set

	0000 - Nop (do nothing)
	00E0 - Clear
	00EE - Return from subroutine
	1NNN - Jump to NNN
	2NNN - Call subroutine at NNN
	6XNN - VX == NN (set V register to NN value)
	7XNN - VX += NN
	ANNN - Register I set to NNN
	DXYN - Display N-pixels tall (always 8 pixwls wide) sprite at mem location I at VX, VY, set VF = collision

*/
bool	match(BYTE hex[], BYTE code1, BYTE code2, BYTE code3,  BYTE code4)
{
	return (hex[0] == code1 && hex[1] == code2 && hex[2] == code3 && hex[3] == code4);
}

void	execute(t_u16 op, t_data *data)
{
	BYTE hex[4];

	hex[0] = (op & 0xF000) >> 12;
	hex[1] = (op & 0x0F00) >> 8;
	hex[2] = (op & 0x00F0) >> 4;
	hex[3] = (op & 0x000F);
	
	if (match(hex, 0, 0, 0, 0))
		return;
	else if (match(hex, 0, 0, 0xE, 0))
		memset(data->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
	else if (match(hex, 0, 0, 0xE, 0xE))
		data->pc = pop(data->stack, &data->stack_ptr);
	else if (hex[0] == 1)
		data->pc = op & 0xFFF;
	else if (hex[0] == 2)
		data->pc = push(data->stack, &data->stack_ptr, op & 0xFFF);
	else if (hex[0] == 6)
		data->v[hex[1]] = op & 0xFF; 
	else if (hex[0] == 7)
		data->v[hex[1]] += op & 0xFF; 
	else if (hex[0] == 0xA)
		data->i = op & 0xFFF;
	else if (hex[0] == 0xD)
	{
		bool flipped = false;
		for (int i = 0; i < hex[3]; ++i)
		{
			t_u16 addr = data->i + i;
			BYTE pixels = ram[addr];
			for (int j = 0; j < 8; ++j)
			{
				if (pixels & (0x80 >> j)) != 0
				{
					BYTE x = (hex[1] + j) % (BYTE)SCREEN_WIDTH;
					BYTE y = (hex[2] + j) % (BYTE)SCREEN_HEIGHT;
					BYTE xy = x + (BYTE)SCREEN_WIDTH * y;
					flipped |= data->screen[xy]; 
					data->screen[xy] ^= 1;
				}
			}
		}
		if (flipped)
			v[0xF] = 1;
		else
			v[0xF] = 0;	
	}
	else
		write(2, "Opcode does not exist\n", 22);
}

