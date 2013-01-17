CFLAGS = -g -Wall `sdl-config --cflags` -O2
LIBS = -g -Wall `sdl-config --libs` -lm -lSDL_image -lSDL_gfx -lSDL_ttf

demo: demo.o
	$(CC) $(CFLAGS) -o demo demo.o $(LIBS)

clean:
	rm -f *~ *.o demo
