#include "chip8.h"

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
	BYTE	hex[4];
	BYTE	 x;
	BYTE	 nn;
    t_u16	nnn;

	hex[0] = (op & 0xF000) >> 12;
	hex[1] = (op & 0x0F00) >> 8;
	hex[2] = (op & 0x00F0) >> 4;
	hex[3] = (op & 0x000F);

	nn = op & 0xFF;	
	nnn = op & 0xFFF;

	if (match(hex, 0, 0, 0, 0))
		return;
	else if (match(hex, 0, 0, 0xE, 0))
	{
		memset(data->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
	}	
	else if (match(hex, 0, 0, 0xE, 0xE))
	{
		data->pc = pop(data->stack, &data->stack_ptr);
	}
	else if (hex[0] == 1)
	{
		data->pc = nnn;
	}
	else if (hex[0] == 2)
	{
		push(data->stack, &data->stack_ptr, nnn);
		data->pc = nnn;
	}
	else if (hex[0] == 6)
	{
		x = hex[1];
		data->v[x] = nn;
	}
	else if (hex[0] == 7)
	{
		x = hex[1]; 
		data->v[x] += nn; 
	}
	else if (hex[0] == 0xA)
	{
		data->i = nnn;
	}
	else if (hex[0] == 0xD)
	{
		bool flipped = false;
		BYTE x_coord = data->v[hex[1]];
		BYTE y_coord = data->v[hex[2]];
		for (int i = 0; i < hex[3]; ++i)
		{
			BYTE pixels = data->ram[data->i + i];
			for (int x_line = 0; x_line < 8; ++x_line)
			{
				//get current pixel bit (mask = 10000000)
				if ((pixels & (0x80 >> (BYTE)x_line)) != 0)
				{
					BYTE x = (x_coord + (BYTE)x_line) % (BYTE)SCREEN_WIDTH;
					BYTE y = (y_coord + (BYTE)i) % (BYTE)SCREEN_HEIGHT;
					int	xy = x + (BYTE)SCREEN_WIDTH * y;
					flipped |= data->screen[xy]; 
					data->screen[xy] ^= 1;
				}
			}
		}
		if (flipped)
			data->v[0xF] = 1;
		else
			data->v[0xF] = 0;	
	}
	else
	{
		printf("%d %d %d %d\n", hex[0], hex[1], hex[2], hex[3]);
		write(2, "Opcode does not exist\n", 22);
	}
}

