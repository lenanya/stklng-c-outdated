all: stklng

stklng: stklng.c
	cc stklng.c -o stklng -Wall -Wextra -ggdb

//stklng.exe: stklng.c
//	i686-w64-mingw32-gcc stklng.c -o stklng.exe	-I/usr/include/