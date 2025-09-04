
all:
	gcc main.c mesh.c block.c chunk.c world.c util.c -lraylib -lm -g
