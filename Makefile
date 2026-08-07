pixelfire: pixelfire.c
	gcc -Wall -Wextra -O3 -o pixelfire pixelfire.c -lSDL2
	strip pixelfire

clean:
	rm -f pixelfire