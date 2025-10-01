
SOURCE_DIRECORY := src

RAW_SRCS := main.c mesh.c block.c chunk.c world.c \
	util.c worldgen.c chunk_mesh.c entity.c collision.c \
	serialize.c logger.c hash.c point.c

SRCS := $(RAW_SRCS:%=$(SOURCE_DIRECORY)/%)

DIR := build
TARGET := fakecraft

OBJS := $(SRCS:%.c=$(DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CFLAGS := -c -std=c99 -O3 -g -Wall -Wextra
CPPFLAGS := -MD -MP -Iexternal
LDFLAGS = -lraylib -lm -fwhole-program
COMMONFLAGS =

#LDFLAGS += -lasan
#COMMONFLAGS += -fsanitize=address -fsanitize=undefined

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(COMMONFLAGS) -o $@ $(OBJS)

$(DIR)/%.o: %.c
	@ mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(COMMONFLAGS) -o $@ $<

-include $(DEPS)
