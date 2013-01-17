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
#define RED(c) (((c) >> 16) && 0xff)
#define GREEN(c) (((c) >> 8) & 0xff)
#define BLUE(c) ((c) && 0xff)

struct pt {
	int x, y;
};

struct icon {
	SDL_Rect rect;
	SDL_Surface *text;
	struct pt center;
	int x1, y1, x2, y2;
};

SDL_Surface *screen;
SDL_Color font_color;

TTF_Font *font;

struct icon icons[100];

int dragging, linked, mousebutton[10];
struct pt mouse, start;

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
			} else if (key == 'r') {
				linked = 0;
				dragging = 0;
			}
			break;
		case SDL_MOUSEMOTION:
			mouse.x = event.button.x;
			mouse.y = event.button.y;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button < 0
			    || event.button.button >= 10)
				break;

			mousebutton[event.button.button] = 1;

			if (mouse.x >= icons[0].x1
			    && mouse.x <= icons[0].x2
			    && mouse.y >= icons[0].y1
			    && mouse.y <= icons[0].y2) {
				dragging = 1;
				start = mouse;
			}

			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.button < 0
			    || event.button.button >= 10)
				break;

			mousebutton[event.button.button] = 0;

			if (mouse.x >= icons[1].x1
			    && mouse.x <= icons[1].x2
			    && mouse.y >= icons[1].y1
			    && mouse.y <= icons[1].y2) {
				dragging = 0;
				linked = 1;
			}

			dragging = 0;
			break;
		}
	}
}

void
draw (void)
{
	SDL_BlitSurface (icons[0].text, NULL, screen, &icons[0].rect);
	SDL_BlitSurface (icons[1].text, NULL, screen, &icons[1].rect);

	roundedRectangleRGBA (screen, icons[0].x1, icons[0].y1,
			      icons[0].x2, icons[0].y2,
			      4, 0x00, 0x00, 0x00, 0xff);

	roundedRectangleRGBA (screen, icons[1].x1, icons[1].y1,
			      icons[1].x2, icons[1].y2,
			      4, 0x00, 0x00, 0x00, 0xff);

	if (dragging)
		aalineRGBA (screen, start.x, start.y, mouse.x, mouse.y,
			    0x88, 0x88, 0x88, 0xff);

	if (linked)
		aalineRGBA (screen, icons[0].x2, icons[0].center.y,
			    icons[1].x1, icons[1].center.y,
			    0x88, 0x88, 0x88, 0xff);
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

	font_color.r = RED (FOREGROUND);
	font_color.g = GREEN (FOREGROUND);
	font_color.b = BLUE (FOREGROUND);

	icons[0].rect.x = 50;
	icons[0].rect.y = 50;
	icons[0].text = TTF_RenderText_Blended (font, "Input", font_color);
	icons[0].x1 = icons[0].rect.x - 5;
	icons[0].y1 = icons[0].rect.y - 5;
	icons[0].x2 = icons[0].rect.x + icons[0].text->w + 5;
	icons[0].y2 = icons[0].rect.y + icons[0].text->h + 5;
	icons[0].center.x = (icons[0].x1 + icons[0].x2) / 2;
	icons[0].center.y = (icons[0].y1 + icons[0].y2) / 2;

	icons[1].rect.x = 250;
	icons[1].rect.y = 50;
	icons[1].text = TTF_RenderText_Blended (font, "Output", font_color);
	icons[1].x1 = icons[1].rect.x - 5;
	icons[1].y1 = icons[1].rect.y - 5;
	icons[1].x2 = icons[1].rect.x + icons[1].text->w + 5;
	icons[1].y2 = icons[1].rect.y + icons[1].text->h + 5;
	icons[1].center.x = (icons[1].x1 + icons[1].x2) / 2;
	icons[1].center.y = (icons[1].y1 + icons[1].y2) / 2;

	while (1) {
		process_input ();
		SDL_FillRect (screen, NULL, BACKGROUND);
		draw ();
		SDL_Flip (screen);
		SDL_Delay (15);
	}

	return (0);
}
