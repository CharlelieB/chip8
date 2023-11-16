#include "chip8.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdlib.h>

const t_u16 START_ADDR = 0x200;


//TODO : Font must me laod to RAM
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
	memset(data->keys, 0, 16);
	data->renderer = 0;
	data->window = 0;
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

t_u16	fetch(t_data *data)
{
	t_u16	opcode;
	t_u16	left;
	t_u16	right;

	left = data->ram[data->pc];
	right = data->ram[data->pc + 1];
	opcode = (left << 8) | (right);
	data->pc += 2;
	return opcode;
}
void	tick(t_data *data)
{
	t_u16 op = fetch(data);	
	execute(op, data);
}

bool	parse_file(char *path, t_data *data)
{
	int	fd;
	size_t	len;
	ssize_t n;
	BYTE	*ptr;

	/* 
		NOT a real parsing, user can provide a file bigger than ram
		Must be improved
	*/
	len = 4096 - START_ADDR;
	fd = open(path, O_RDONLY);
	if (fd < 0)
	{
		perror("Unable to open file");
		return false;
	}
	ptr = data->ram;
	ptr += START_ADDR;
	n = read(fd, ptr, len);
	if (n == -1)
	{
		perror("read file");
		close(fd);
		return (false);
	}
     	close(fd);
	return true;
}

void	draw(t_data *data)
{
	int		screen_size;
	int		x;
	int		y;
	SDL_Rect	rect;

	screen_size = SCREEN_WIDTH * SCREEN_HEIGHT;
	if (SDL_SetRenderDrawColor(data->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE) != 0)
		destroy_SDL(data, "Unable to set color for render");
	if (SDL_RenderClear(data->renderer) != 0)
		destroy_SDL(data, "Unable to clear render");
	if (SDL_SetRenderDrawColor(data->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE) != 0)
		destroy_SDL(data, "Unable to set color for render");
	for (int i = 0; i < screen_size; ++i)
	{
		if (data->screen[i])
		{	
			x = (i % SCREEN_WIDTH);
		    y = (i / SCREEN_WIDTH);
			rect.x = x * SCALE; 
			rect.y = y * SCALE;
			rect.h = SCALE; 
			rect.w = SCALE;
			if (SDL_RenderFillRect(data->renderer, &rect) != 0)
				destroy_SDL(data, "Unable to draw rect");
		}
	}
	SDL_RenderPresent(data->renderer);
}

void	key_event(int value, bool keys[], bool is_pressed)
{
	/*
	Key mapping
	1	2	3	C	|	1	2	3	4
	4	5	6	D	|	Q	W	E	R
	7	8	9	E	|	A	S	D	F
	A	0	B	F	|	Z	X	C	V
	*/
	switch (value)
	{
		case SDLK_1:keys[0x1] = is_pressed;break;
		case SDLK_2:keys[0x2] = is_pressed;break;
		case SDLK_3:keys[0x3] = is_pressed;break;
		case SDLK_4:keys[0xC] = is_pressed;break;
		case SDLK_q:keys[0x4] = is_pressed;break;
		case SDLK_w:keys[0x5] = is_pressed;break;
		case SDLK_e:keys[0x6] = is_pressed;break;
		case SDLK_r:keys[0xD] = is_pressed;break;
		case SDLK_a:keys[0x7] = is_pressed;break;
		case SDLK_s:keys[0x8] = is_pressed;break;
		case SDLK_d:keys[0x9] = is_pressed;break;
		case SDLK_f:keys[0xE] = is_pressed;break;
		case SDLK_z:keys[0xA] = is_pressed;break;
		case SDLK_x:keys[0x0] = is_pressed;break;
		case SDLK_c:keys[0xB] = is_pressed;break;
		case SDLK_v:keys[0xF] = is_pressed;break;
	}
}

int main(int argc, char **argv)
{
	t_data	data;

	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		return 1;
	}
	init(&data);
	setup_SDL(&data);
	if (!parse_file(argv[1], &data))
		return 1;
	while(true)
	{
		if (SDL_PollEvent(&data.event))
		{
			if (data.event.type == SDL_QUIT)
				break;
			if (data.event.type == SDL_KEYDOWN)
			{
				if (data.event.key.keysym.sym == SDLK_ESCAPE)
					break;
				key_event(data.event.key.keysym.sym, data.keys, true);
			}
			if (data.event.type == SDL_KEYUP)
				key_event(data.event.key.keysym.sym, data.keys, false);
		}
		// for (int i = 0; i < 10; ++i)
			tick(&data);
		draw(&data);
	}
	SDL_Delay(1000);
	destroy_SDL(&data, NULL);
	return 0;
}
