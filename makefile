ifeq ($(OS),Windows_NT)
	COMPILE_WINDOWS=1
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		COMPILE_LINUX=1
	endif
	ifeq ($(UNAME_S),Darwin)
		COMPILE_MAC=1
	endif
endif

CC = gcc
# CC = clang

C_FLAGS += -pg
C_FLAGS += -g
C_FLAGS += -W -ansi -pedantic -static
C_FLAGS += -Wall -Wextra -Wwrite-strings -Winit-self -Wcast-align -Wcast-qual
C_FLAGS += -Wpointer-arith -Wformat=2 -Wlogical-op
#CFLAGS += -Wno-unused-variable -Wno-unused-parameter -Wno-unused

C_FLAGS += -O2
# C_FLAGS += -std=c99
C_FLAGS += -std=c11
C_FLAGS += -march=native

ifdef COMPILE_WINDOWS
	C_FLAGS += -lmingw32
	# C_FLAGS += -lSDLmain
	C_FLAGS += -lSDL2
	# C_FLAGS += -lopengl32
	# C_FLAGS += -lglu32
	# LIBS += -lmingw32
	# LIBS += -pg -g
	# LIBS += -lm
	# LIBS += -lvorbisfile
	# LIBS += -lopenal32
	# LIBS += -lSDLmain
	# LIBS += -lSDL
	# LIBS += -lmingw32
	# LIBS += -lopengl32
	# LIBS += -lglu32
endif
ifdef COMPILE_LINUX
	C_FLAGS += -lSDL2
	LIBS += -pg -g
	LIBS += -lm
	# LIBS += -lvorbisfile
	# LIBS += -lopenal
	LIBS += -lSDL2
	LIBS += -lGLEW
	# LIBS += -lglfw
	LIBS += -lGL
	# LIBS += -lGLU
endif
# LIBS += -lpthread
SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:src/%.c=build/%.o)

PROG = program

all:$(OBJECTS)
	$(CC) $(LIBS) $(OBJECTS) $(LIBS) -o $(PROG)
	
# To obtain object files
build/%.o: src/%.c
	mkdir -p build; $(CC) $(C_FLAGS) -c $< -o $@

run:all
	./$(PROG)

run-softwae:all
	LIBGL_DEBUG=verbose LIBGL_SHOW_FPS=1 LIBGL_ALWAYS_SOFTWARE=1 ./$(PROG)

run-profiling:all
	LIBGL_DEBUG=verbose LIBGL_SHOW_FPS=1 ./$(PROG) ;gprof $(PROG) gmon.out

clean:
	rm -f $(OBJECTS)

mrproper:
	rm -f $(PROG) $(OBJECTS)
