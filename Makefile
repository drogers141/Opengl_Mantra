# Makefile for opengl using glew

EXE = show_mantra

VPATH = src:./build

CC = gcc
CPP = g++

CFLAGS =  -Wall 
INCLUDES = -Iinclude
LIBS   = -lGLEW  -lglut -lGLU -lGL 

HOST_PLATFORM := $(shell $(CPP) -dumpmachine)
$(info $(HOST_PLATFORM))

ifeq   "$(HOST_PLATFORM)" "i686-apple-darwin10"
INCLUDES = -Iinclude -I/opt/local/include
LIBS = -L/opt/local/lib -lGLEW  -framework OpenGL -framework GLUT
endif

OBJ = $(addprefix build/, $(filter %.o, $(SRC:.cpp=.o) $(SRC:.c=.o)))

SRC = $(notdir $(wildcard src/*) )

all: $(EXE)

$(EXE): $(OBJ)
	$(CPP) $(CFLAGS) $(LIBS) $(OBJ) -o $@
	
build/%.o : %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

build/%.o : %.cpp
	$(CPP) -c $(CFLAGS) $(INCLUDES) $< -o $@


clean: 
	rm -f build/*


test: 
	@echo "platform:" $(HOST_PLATFORM)
	@echo "SRC = " $(SRC)
	@echo "OBJ = " $(OBJ)


