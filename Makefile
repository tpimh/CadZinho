SRC_PATH=./src/
CC=gcc
COMPILER_FLAGS = -g -c
LINKER_FLAGS = `sdl2-config --cflags --libs` -llua -lm -framework OpenGL -lGLEW
INCLUDE_PATHS = -I. -I./src/ -I/usr/include/SDL2 -I/usr/local/Cellar/lua/5.4.3/include/lua/
LIBRARY_PATHS = -L/usr/lib -L.
EXE=cadzinho

SRC=$(wildcard $(SRC_PATH)*.c)
OBJ=$(subst ./src, ./obj, $(SRC:.c=.o))

all: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LIBRARY_PATHS) $(LINKER_FLAGS) $(OBJ) $(LINKER_FLAGS) -o $@

./obj/%.o: ./src/%.c
	$(CC) $(INCLUDE_PATHS) $(COMPILER_FLAGS) -o $@ $<

clean:
	rm -rf run $(OBJ)
