#include <stdint.h>
#include "chip8.h"
#include <string.h>

const t_u16 START_ADDR 0x200;

const BYTE FONTSET[80] = {
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
	0xF0, 0x80, 0xF0, 0x80, 0x80 // F
};

typedef struct s_data
{
	t_u16	stack[16]
	BYTE	stack_ptr;
	t_u16	pc;
	BYTE	v[16]; //registers
	t_u16	i;
	BYTE	ram[4096];
	BYTE	screen[SCREEN_WIDTH * SCREEN_HEIGHT];
	BYTE	delay_timer;
	BYTE	sound_timer;
}		t_data;

void	push(t_u16 stack[], BYTE *stack_ptr, t_u16 value)
{
	if (*stack_ptr == 16)
	{
		write(2, "Stack overflow\n", 15);
		exit(1);
	}
	stack[*stack_ptr] = value;
	++(*stack_ptr);
}

t_u16	pop(t_u16 stack[], BYTE *stack_ptr)
{
	if (*stack_ptr == 0)
	{
		write(2, "Stack underflow\n", 16);
		exit(1);
	}
	--(*stack_ptr);
	return (stack[*stack_ptr]);
}

void	init(t_data *data)
{
	data->stack_ptr = 0;
	memset(data->stack, 0, 16);
	data->pc = START_ADDR;
	memset(data->v, 0, 16);
	data->i = 0;
	memset(data->ram, 0, 4096);
	memset(data->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
	data->delay_timer = 0;
	data->sound_timer = 0;
	data->stack_ptr = 0;
}

void	timer(t_data *data)
{
	if (data->delay_timer > 0)
		--(data->delay_timer);
	if (data->sound_timer > 0)
	{
		if (data->sound_timer == 1)
			write(1, "BIP\n", 4);
		--(data->sound_timer);
	}
}

t_u16	fetch(t_u16 opcode, t_data *data)
{
	BYTE left = data->ram[data->pc];
	BYTE right = data->ram[data->pc + 1];
	opcode = (left << 8) | (right);
	data->pc += 2;
	return opcode;
}
void	tick(t_u16 opcode, t_data *data)
{
	t_u16 op = fetch(opcode, data);	
	execute(op);
}

int main(void)
{
	t_data data;

	init(&data);

	return 0;
}
