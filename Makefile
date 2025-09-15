
SRCS := main.c mesh.c block.c chunk.c world.c \
	util.c worldgen.c chunk_mesh.c entity.c collision.c serialize.c

DIR := build
TARGET := fakecraft

OBJS := $(SRCS:%.c=$(DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CFLAGS := -c -g -O2 -Wall -Wextra -Wno-unused-parameter
CPPFLAGS := -MD -MP
LDFLAGS := -lraylib -lm -fwhole-program
COMMONFLAGS :=

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(COMMONFLAGS) -o $@ $(OBJS)

$(DIR)/%.o: %.c
	@ mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(COMMONFLAGS) -o $@ $<

-include $(DEPS)
