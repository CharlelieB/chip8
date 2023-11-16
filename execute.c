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

void	execute(t_u16 op, t_data *data)
{
	t_u8	hex1;
	t_u8	nn;
	t_u8	n;
	t_u8	x;
	t_u8	y;
    t_u16	nnn;

	hex1 = (op & 0xF000) >> 12;
	x = (op & 0x0F00) >> 8;
	y = (op & 0x00F0) >> 4;

	n = op & 0xF;
	nn = op & 0xFF;	
	nnn = op & 0xFFF;

	switch(hex1)
	{
		case 0:
		switch(op & 0x00FF)
		{
			case 0x00E0:
			memset(data->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
			break;

			case 0x00EE:
			data->pc = pop(data->stack, &data->stack_ptr);
			break;
		} break;

		case 1:
		data->pc = nnn;
		break;

		case 2:
		push(data->stack, &data->stack_ptr, data->pc);
		data->pc = nnn;
		break;

		case 3:
		if (data->v[x] == nn)
			data->pc += 2;
		break;

		case 4:
		if (data->v[x] != nn)
			data->pc += 2;
		break;

		case 5:
		if (data->v[x] == data->v[y])
			data->pc += 2;
		break;

		case 6:
		data->v[x] = nn;
		break;

		case 7:
		data->v[x] += nn;
		break;

		case 8:
		switch (n)
		{
			case 0:
			data->v[x] = data->v[y];
			break;

			case 1:
			data->v[x] = data->v[x] | data->v[y];
			break;

			case 2:
			data->v[x] = data->v[x] & data->v[y];
			break;

			case 3:
			data->v[x] = data->v[x] ^ data->v[y];
			break;

			case 4:;
			t_u8 old = data->v[x];
			data->v[x] += data->v[y];
			//checking overflow
			data->v[0xF] = data->v[x] < old;
			break;

			case 5:
			data->v[0xF] = data->v[x] > data->v[y];
			data->v[x] -= data->v[y];
			break;

			case 6:
			//check if least-significant bit is 1 (underflow)
			data->v[0xF] = data->v[x] & 1;
			//divide vx by 2
			data->v[x] >>= 1;
			break;
	
			case 7:
			data->v[0xF] = data->v[x] < data->v[y];
			data->v[x] = data->v[y] - data->v[x];
			break;

			case 0xE:
			//check if most-significant bit is 1 (overflow)
			data->v[0xF] = data->v[x] & 0x80;
			//multiple vx by 2
			data->v[x] <<= 1;
			break;
		}break;
	
		case 9:
		if (data->v[x] != data->v[y])
			data->pc += 2;
		break;

		case 0xA:
		data->i = nnn;
		break;

		case 0xB:
		data->pc = nnn + data->v[0];
		break;

		case 0xC:
		data->v[x] = (rand() % 256) & nn;
		break;

		case 0xD:;
		bool flipped = false;
		t_u8 x_coord = data->v[x];
		t_u8 y_coord = data->v[y];
		for (int i = 0; i < n; ++i)
		{
			t_u8 pixels = data->ram[data->i + i];
			for (int x_line = 0; x_line < 8; ++x_line)
			{
				//get current pixel bit (mask = 10000000)
				if ((pixels & (0x80 >> (t_u8)x_line)) != 0)
				{
					t_u8 x_screen = (x_coord + (t_u8)x_line) % (t_u8)SCREEN_WIDTH;
					t_u8 y_screen = (y_coord + (t_u8)i) % (t_u8)SCREEN_HEIGHT;
					int	xy = x_screen + (t_u8)SCREEN_WIDTH * y_screen;
					flipped |= data->screen[xy]; 
					data->screen[xy] ^= 1;
				}
			}
		}
		data->v[0xF] = flipped;	
		break;

		case 0xE:
		switch (nn)
		{
			case 0x9E:
			if (data->keys[data->v[x]])
				data->pc += 2;
			break;

			case 0xA1:
			if (!data->keys[data->v[x]])
				data->pc += 2;
			break;
		}break;

		case 0xF:
		switch (nn)
		{
			case 0x07:
			data->v[x] = data->delay_timer;
			break;
		
			case 0x0A:;
			bool	pressed = false;
			for (int i = 0; i < 16; ++i)
			{
				if (data->keys[i])
				{
					data->v[x] = (t_u8)i;
					pressed = true;
					break;
				}
			}
			if (!pressed)
				data->pc -= 2;
			break;

			case 0x15:
			data->delay_timer = data->v[x];
			break;

			case 0x18:
			data->sound_timer = data->v[x];
			break;

			case 0x1E:
			data->i += data->v[x];
			break;
		
			case 0x29:
			data->i = (data->v[x] * 5) + FONT_START_ADDR;
			break;

			case 0x33:;
			t_u8 tmp = data->v[x];
			t_u16 bcd = 0;
			//double dabble
			for (int i = 0; i < 8; ++i)
			{
				if ((bcd & 0xF) > 0x4)
					bcd += 0x3;
				if ((bcd & 0xFF) > 0x40)
					bcd += 0x30;
				if ((bcd & 0xF00) > 0x400)
					bcd += 0x300;
				bcd <<= 1;
				if (tmp & 0x80)
					bcd += 1;
				tmp <<= 1;
			}
			break;

			case 0x55:
			for (int i = 0; i <= x; ++i)
			{
				data->ram[i + data->i] = data->v[i];
			}
			break;
			
			case 0x65:
			for (int i = 0; i <= x; ++i)
			{
				data->v[i] = data->ram[i + data->i];
			}
			break;

		}break;

		default:
		write(2, "Opcode does not exist\n", 22);
		break;
	}
}

