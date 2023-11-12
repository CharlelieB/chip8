#include "chip8.h"

void	destroy_SDL(t_data *data, char *message)
{
	char status;

	if (message)
	{
		SDL_Log("Error : %s > %s\n", message, SDL_GetError());
		status = EXIT_FAILURE;
	}
	else
		status = EXIT_SUCCESS;
	if (data->renderer)
		SDL_DestroyRenderer(data->renderer);
	if (data->window)
		SDL_DestroyWindow(data->window);
	SDL_Quit();
	exit(status);
}
void	setup_SDL(t_data *data)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		destroy_SDL(data, "SDL init failed");
	data->window = SDL_CreateWindow(
		"CHIP8",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		0
	);
	if (data->window == NULL)
		destroy_SDL(data, "Unable to create window");
	data->renderer = SDL_CreateRenderer(
		data->window,
		-1,
		SDL_RENDERER_PRESENTVSYNC
	);
	if (data->renderer == NULL)
		destroy_SDL(data, "Unable to create render");
}

