#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <time.h>
#include <signal.h>

#define WIDTH 1280
#define HEIGHT 960
#define BACKGROUND 0xffffff
#define FOREGROUND 0x000000
#define FONT "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansMono.ttf"
#define RED(c) (((c) >> 16) && 0xff)
#define GREEN(c) (((c) >> 8) & 0xff)
#define BLUE(c) ((c) && 0xff)

#define BIGRED 121
#define MISSILE_OJ 92
#define MISSILE_RED 42
#define KNIFE_UP 123
#define KNIFE_DOWN 54
#define JOY_LEFT 19
#define JOY_RIGHT 20
#define JOY_DOWN 18
#define JOY_UP 21
#define ARCADE_GREEN 82
#define ARCADE_RED 8
#define ARCADE_BLACK 96
#define ARCADE_YELLOW 83
#define KEY_0 79
#define KEY_1 80
#define KEY_2 81
#define KEY_3 77
#define KEY_4 31
#define KEY_5 53
#define KEY_6 37

enum {
	NOTHING,
	MEME
};

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
	int x1, y1, x2, y2, type, keycode, value, used, output;
};

SDL_Surface *screen;
SDL_Color font_color;

TTF_Font *font;

struct icon *input_head, *output_head;

int mousebutton[10], off, sock;
struct icon *dragging, *linked;
struct pt mouse, start;

void *xcalloc (unsigned int a, unsigned int b);
struct icon *on_input (struct pt *p);
struct icon *on_output (struct pt *p);
void link_io (struct icon *ip1, struct icon *ip2);
void process_input (void);
struct icon *overlap (struct icon *ip1);
void mk_in (int x, int y, char *name, int keycode);
void mk_out (int x, int y, char *name, int type);
void draw (void);

void
usage (void)
{
	printf ("usage: demo hostname\n");
	exit (1);
}

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

	for (ip = input_head; ip; ip = ip->next) {
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

	for (ip = output_head; ip; ip = ip->next) {
		if (p->x >= ip->x1 && p->x <= ip->x2
		    && p->y >= ip->y1 && p->y <= ip->y2)
			return (ip);
	}

	return (NULL);
}

void
link_io (struct icon *ip1, struct icon *ip2)
{
	if (ip1->link && ip1->link->link == ip1)
		ip1->link->link = 0;

	if (ip2->link && ip2->link->link == ip2)
		ip2->link->link = 0;

	ip1->link = ip2;
	ip2->link = ip1;
}

void
process (char *buf)
{
	struct icon *ip;
	int value, code;

	if (sscanf (buf, "kbd0 %d %d", &value, &code) == 2) {
		for (ip = input_head; ip; ip = ip->next) {
			if (ip->keycode == code) {
				ip->value = value;
				ip->used = 0;
			}
		}
	}
}

void
get_pushed (void)
{
	char ch, buf[1000];

	if (read (sock, &ch, 1) != 1) {
		printf ("error reading\n");
		exit (1);
	}

	buf[off++] = ch;

	if (ch == '\n') {
		buf[off-1] = 0;
		process (buf);
		off = 0;
	}
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

				for (ip = input_head; ip; ip = ip->next) {
					ip->link = 0;
				}

				for (ip = output_head; ip; ip = ip->next) {
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
					link_io (dragging, linked);
			}

			dragging = 0;
			break;
		}
	}
}

void
random_meme (void)
{
	int pid;

	if ((pid = fork ()) == 0) {
		exit (system ("/home/atw/tfdemo/randmeme.py"));
	}
}

void
selector (struct icon *ip)
{
	if (ip->link) {
		switch (ip->link->output) {
		case MEME:
			printf ("selecting %d for %d\n", MEME, ip->keycode);
			random_meme ();
			ip->used = 1;
			break;
		default:
			break;
		}
	}
}

void
process_buttons (void)
{
	struct icon *ip;

	for (ip = input_head; ip; ip = ip->next) {
		if (ip->value && ip->used == 0) {
			switch (ip->keycode) {
			default:
				selector (ip);
				break;
			}
		}
	}
}

struct icon *
overlap (struct icon *ip1)
{
	struct icon *ip2;

	for (ip2 = input_head; ip2; ip2 = ip2->next) {
		if (ip1->x2 < ip2->x1 || ip1->x1 > ip2->x2
		    || ip1->y2 < ip2->y1 || ip1->y1 > ip2->y2)
			continue;

		return (ip2);
	}

	for (ip2 = output_head; ip2; ip2 = ip2->next) {
		if (ip1->x2 < ip2->x1 || ip1->x1 > ip2->x2
		    || ip1->y2 < ip2->y1 || ip1->y1 > ip2->y2)
			continue;

		return (ip2);
	}

	return (NULL);
}

void
mk_in (int x, int y, char *name, int keycode)
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

	if (overlap (ip)) {
		printf ("failed to create icon %s: overlap not allowed\n",
			name);

		SDL_FreeSurface (ip->text);
		free (ip);
		return;
	}

	ip->center.x = (ip->x1 + ip->x2) / 2;
	ip->center.y = (ip->y1 + ip->y2) / 2;
	ip->type = IN;
	ip->keycode = keycode;

	if (!input_head) {
		input_head = ip;
	} else {
		ip->next = input_head;
		input_head = ip;
	}
}

void
mk_out (int x, int y, char *name, int type)
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
	ip->output = type;

	if (overlap (ip)) {
		printf ("failed to create icon %s: overlap not allowed\n",
			name);

		SDL_FreeSurface (ip->text);
		free (ip);
		return;
	}

	ip->center.x = (ip->x1 + ip->x2) / 2;
	ip->center.y = (ip->y1 + ip->y2) / 2;
	ip->type = OUT;

	if (!output_head) {
		output_head = ip;
	} else {
		ip->next = output_head;
		output_head = ip;
	}
}

void
draw (void)
{
	struct icon *ip;

	for (ip = input_head; ip; ip = ip->next) {
		SDL_BlitSurface (ip->text, NULL, screen, &ip->rect);
		roundedRectangleRGBA (screen, ip->x1, ip->y1, ip->x2, ip->y2,
				      4, 0x00, 0x00, 0x00, 0xff);
		if (ip->value)
			rectangleRGBA (screen, ip->x1, ip->y1, ip->x2, ip->y2,
				       0xff, 0x00, 0x00, 0xff);

		if (ip->link)
			aalineRGBA (screen, ip->x2, ip->center.y,
				    ip->link->x1, ip->link->center.y,
				    0x88, 0x88, 0x88, 0xff);
	}

	for (ip = output_head; ip; ip = ip->next) {
		SDL_BlitSurface (ip->text, NULL, screen, &ip->rect);
		roundedRectangleRGBA (screen, ip->x1, ip->y1, ip->x2, ip->y2,
				      4, 0x00, 0x00, 0x00, 0xff);

		if (ip->link && ip->link->value)
			rectangleRGBA (screen, ip->x1, ip->y1, ip->x2, ip->y2,
				       0xff, 0x00, 0x00, 0xff);
	}

	if (dragging)
		aalineRGBA (screen, start.x, start.y, mouse.x, mouse.y,
			    0x88, 0x88, 0x88, 0xff);
}

int
main (int argc, char **argv)
{
	struct sockaddr_in addr;
	struct hostent *hp;
	struct timeval tv;
	int c, win, port, maxfd;
	char *hostname;
	fd_set rset, wset;

	while ((c = getopt (argc, argv, "")) != EOF) {
		switch (c) {
		default:
			usage ();
		}
	}

	if (optind >= argc)
		usage ();

	hostname = argv[optind++];

	if (optind != argc)
		usage ();

	srand (time (NULL));

	hp = gethostbyname (hostname);
	if (hp == NULL) {
		printf ("%s not found\n", hostname);
		exit (1);
	}

	sock = socket (AF_INET, SOCK_STREAM, 0);
	win = 0;
	for (port = 9195; port <= 9200; port++) {
		memset (&addr, 0, sizeof addr);
		addr.sin_family = AF_INET;
		memcpy (&addr.sin_addr, hp->h_addr, sizeof addr.sin_addr);
		addr.sin_port = htons (port);
		if (connect (sock,
			     (struct sockaddr *) &addr, sizeof addr) >= 0) {
			win = 1;
			break;
		}
	}

	if ( ! win) {
		printf ("cannot connect to %s\n", hostname);
		exit (1);
	}

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

	mk_in (50, 15, "Big Red", BIGRED);
	mk_in (50, 65, "Orange Missile", MISSILE_OJ);
	mk_in (50, 115, "Red Missile", MISSILE_RED);
	mk_in (50, 165, "Knife Switch Up", KNIFE_UP);
	mk_in (50, 215, "Knife Switch Down", KNIFE_DOWN);
	mk_in (50, 265, "Joystick left", JOY_LEFT);
	mk_in (50, 315, "Joystick Right", JOY_RIGHT);
	mk_in (50, 365, "Joystick Down", JOY_DOWN);
	mk_in (50, 415, "Joystick Up", JOY_UP);
	mk_in (50, 465, "Arcade Green", ARCADE_GREEN);
	mk_in (50, 515, "Arcade Red", ARCADE_RED);
	mk_in (50, 565, "Arcade Black", ARCADE_BLACK);
	mk_in (50, 615, "Arcade Yellow", ARCADE_YELLOW);
	mk_in (50, 665, "Keyboard 1", KEY_0);
	mk_in (50, 715, "Keyboard 2", KEY_1);
	mk_in (50, 765, "Keyboard 3", KEY_2);
	mk_in (50, 815, "Keyboard 4", KEY_3);
	mk_in (50, 865, "Keyboard 5", KEY_4);
	mk_in (50, 915, "Keyboard 6", KEY_5);
	mk_in (50, 965, "Keyboard 7", KEY_6);

	mk_out (800, 50, "Random", MEME);

	off = 0;
	while (1) {
		maxfd = 0;

		FD_ZERO (&rset);
		FD_ZERO (&wset);

		if (sock > maxfd)
			maxfd = sock;
		FD_SET (sock, &rset);

		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		if (select (maxfd + 1, &rset, &wset, NULL, &tv) < 0) {
			printf ("select error\n");
			exit (1);
		}

		if (FD_ISSET (sock, &rset))
			get_pushed ();

		process_input ();
		process_buttons ();

		SDL_FillRect (screen, NULL, BACKGROUND);
		draw ();
		SDL_Flip (screen);
		SDL_Delay (15);
	}

	return (0);
}
