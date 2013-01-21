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

enum {
	IN,
	OUT
};

struct pt {
	int x, y;
};

struct icon {
	struct icon *next, *link;
	SDL_Rect rect;
	SDL_Surface *text;
	struct pt center;
	int x1, y1, x2, y2, type;
};

SDL_Surface *screen;
SDL_Color font_color;

TTF_Font *font;

struct icon *first_input, *first_output;

int mousebutton[10];
struct icon *dragging, *linked;
struct pt mouse, start;

void *xcalloc (unsigned int a, unsigned int b);
struct icon *on_input (struct pt *p);
struct icon *on_output (struct pt *p);
void link (struct icon *ip1, struct icon *ip2);
void process_input (void);
void mk_icon (int x, int y, char *name, int type);
void draw (void);

void *
xcalloc (unsigned int a, unsigned int b)
{
	void *p;

	if ((p = calloc (a, b)) == NULL) {
		fprintf (stderr, "memory error\n");
		exit (1);
	}

	return (p);
}

struct icon *
on_input (struct pt *p)
{
	struct icon *ip;

	for (ip = first_input; ip; ip = ip->next) {
		if (p->x >= ip->x1 && p->x <= ip->x2
		    && p->y >= ip->y1 && p->y <= ip->y2)
			return (ip);
	}

	return (NULL);
}

struct icon *
on_output (struct pt *p)
{
	struct icon *ip;

	for (ip = first_output; ip; ip = ip->next) {
		if (p->x >= ip->x1 && p->x <= ip->x2
		    && p->y >= ip->y1 && p->y <= ip->y2)
			return (ip);
	}

	return (NULL);
}

void
link (struct icon *ip1, struct icon *ip2)
{
	ip1->link = ip2;
	ip2->link = ip1;
}

void
process_input (void)
{
	struct icon *ip;
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
				dragging = 0;

				for (ip = first_input; ip; ip = ip->next) {
					ip->link = 0;
				}

				for (ip = first_output; ip; ip = ip->next) {
					ip->link = 0;
				}
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

			dragging = on_input (&mouse);
			if (dragging)
				start = mouse;

			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.button < 0
			    || event.button.button >= 10)
				break;

			mousebutton[event.button.button] = 0;

			if (dragging) {
				linked = on_output (&mouse);

				if (linked)
					link (dragging, linked);
			}

			dragging = 0;
			break;
		}
	}
}

void
mk_icon (int x, int y, char *name, int type)
{
	struct icon *ip;

	ip = xcalloc (1, sizeof *ip);

	ip->rect.x = x;
	ip->rect.y = y;
	ip->text = TTF_RenderText_Blended (font, name, font_color);
	ip->x1 = ip->rect.x - 5;
	ip->y1 = ip->rect.y - 5;
	ip->x2 = ip->rect.x + ip->text->w + 5;
	ip->y2 = ip->rect.y + ip->text->h + 5;
	ip->center.x = (ip->x1 + ip->x2) / 2;
	ip->center.y = (ip->y1 + ip->y2) / 2;
	ip->type = type;

	switch (type) {
	case IN:
		if (!first_input) {
			first_input = ip;
		} else {
			ip->next = first_input;
			first_input = ip;
		}
		break;
	case OUT:
		if (!first_output) {
			first_output = ip;
		} else {
			ip->next = first_output;
			first_output = ip;
		}
		break;
	}
}

void
draw (void)
{
	struct icon *ip;

	for (ip = first_input; ip; ip = ip->next) {
		SDL_BlitSurface (ip->text, NULL, screen, &ip->rect);
		roundedRectangleRGBA (screen, ip->x1, ip->y1, ip->x2, ip->y2,
				      4, 0x00, 0x00, 0x00, 0xff);

		if (ip->link)
			aalineRGBA (screen, ip->x2, ip->center.y,
				    ip->link->x1, ip->link->center.y,
				    0x88, 0x88, 0x88, 0xff);
	}

	for (ip = first_output; ip; ip = ip->next) {
		SDL_BlitSurface (ip->text, NULL, screen, &ip->rect);
		roundedRectangleRGBA (screen, ip->x1, ip->y1, ip->x2, ip->y2,
				      4, 0x00, 0x00, 0x00, 0xff);
	}

	if (dragging)
		aalineRGBA (screen, start.x, start.y, mouse.x, mouse.y,
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

	mk_icon (50, 50, "Input", IN);
	mk_icon (250, 50, "Output", OUT);
	mk_icon (250, 250, "Output 2", OUT);

	while (1) {
		process_input ();
		SDL_FillRect (screen, NULL, BACKGROUND);
		draw ();
		SDL_Flip (screen);
		SDL_Delay (15);
	}

	return (0);
}
