#include <stdio.h>
#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#define WIDTH 640
#define HEIGHT 480
#define BACKGROUND 0xffffff
#define FOREGROUND 0x000000
#define FONT "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf"

SDL_Surface *screen, *imgTxt;
SDL_Rect txtRect;
SDL_Color fColor;

TTF_Font *font;

void process_input (void);
void draw (void);

void
process_input (void)
{
	SDL_Event event;
	int key;

	while (SDL_PollEvent (&event)) {
		key = event.key.keysym.sym;
		switch (event.type) {
		case SDL_QUIT:
			exit (0);
		case SDL_KEYUP:
			if (key == SDLK_ESCAPE || key == 'q') {
				exit (0);
			}
			break;
		}
	}
}

void
draw (void)
{
	SDL_BlitSurface (imgTxt, NULL, screen, &txtRect);
}

int
main (int argc, char **argv)
{
	if (SDL_Init (SDL_INIT_VIDEO) != 0) {
		printf ("unable to initialize SDL: %s\n", SDL_GetError ());
		return (1);
	}

	screen = SDL_SetVideoMode (WIDTH, HEIGHT, 32,
				   SDL_HWSURFACE | SDL_DOUBLEBUF);

	if (screen == NULL) {
		printf ("unable to set video mode: %s\n", SDL_GetError ());
		return (1);
	}

	TTF_Init ();
	font = TTF_OpenFont (FONT, 24);

	txtRect.x = 10;
	txtRect.y = 150;
	fColor.r = 0x00;
	fColor.g = 0x00;
	fColor.b = 0x00;
	imgTxt = TTF_RenderText_Blended (font, "hello, world", fColor);

	while (1) {
		process_input ();
		SDL_FillRect (screen, NULL, BACKGROUND);
		draw ();
		SDL_Flip (screen);
		SDL_Delay (15);
	}

	return (0);
}
