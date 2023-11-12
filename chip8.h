#ifndef CHIP8_H
#define CHIP8_H
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32 
#define SCALE 15
#define WINDOW_WIDTH (SCREEN_WIDTH * SCALE)
#define WINDOW_HEIGHT (SCREEN_HEIGHT * SCALE)

typedef uint8_t BYTE;
typedef uint16_t t_u16;

typedef struct s_data
{
	t_u16		stack[16];
	BYTE		stack_ptr;
	t_u16		pc;
	BYTE		v[16]; //registers
	t_u16		i;
	BYTE		ram[4096];
	BYTE		screen[SCREEN_WIDTH * SCREEN_HEIGHT];
	BYTE		delay_timer;
	BYTE		sound_timer;
	ssize_t		mem_size;
	SDL_Window	*window;
	SDL_Renderer	*renderer;
	SDL_Event 	event;
}		t_data;

void    push(t_u16 stack[], BYTE *stack_ptr, t_u16 value);
t_u16   pop(t_u16 stack[], BYTE *stack_ptr);
void    execute(t_u16 op, t_data *data);
void    setup_SDL(t_data *data);
void    destroy_SDL(t_data *data, char *message);

#endif
