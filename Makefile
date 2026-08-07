pixelfire: pixelfire.c
	gcc -Wall -Wextra -O3 -o pixelfire pixelfire.c $(pkg-config --cflags --libs sdl3)
	strip pixelfire

clean:
	rm -f pixelfire
