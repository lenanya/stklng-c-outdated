all: stklng

stklng: stklng.c
	cc stklng.c -o stklng -Wall -Wextra -Werror -ggdb
