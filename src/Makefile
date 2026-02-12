CFLAGS  ?= -std=c17 -g \
    -D_POSIX_SOURCE -D_DEFAULT_SOURCE \
    -Wall -Werror -pedantic \
        -fsanitize=address \
        -fno-omit-frame-pointer \
    -I../local/include/SDL2 \

LDFLAGS ?= -fsanitize=address -L../local/lib -lSDL2 -lSDL2main -Wl,-Bstatic -lSDL2_image -Wl,-Bdynamic -lm -ldl -lpthread 
#-lpigpio

.SUFFIXES: .c .o
.PHONY: all clean

all: main

main: main.o input.o player.o renderer.o combat.o timer.o keyboard.o sword.o
		$(CC) $^ $(LDFLAGS) -o $@

clean:
		$(RM) *.o main

