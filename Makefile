
SRCS := main.c mesh.c block.c chunk.c world.c util.c
DIR := build
TARGET := fakecraft

OBJS := $(SRCS:%.c=$(DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CFLAGS := -c -g -O2
CPPFLAGS := -MD -MP
LDFLAGS := -lraylib -lm

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

$(DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $<

all:
	gcc $(SRCS) -lraylib -lm -g

-include $(DEPS)
