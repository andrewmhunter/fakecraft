CXX = clang++

SOURCE_DIRECORY := src_cpp

RAW_SRCS := main.c mesh.c block.c chunk.c world.c \
	util.c worldgen.c chunk_mesh.c entity.c collision.c \
	serialize.c logger.c hash.c point.c fileio.c timer.c graphics.c input.c text.c

SRCS := $(RAW_SRCS:%=$(SOURCE_DIRECORY)/%) external/glad.c external/stb_implementation.c

DIR := build
TARGET := fakecraft

OBJS := $(SRCS:%.c=$(DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CFLAGS := -c -O3 -g -Wall -Wextra -fdiagnostics-color=always # -march=native
CXXFLAGS := $(CFLAGS) -std=gnu++23
CPPFLAGS := -MD -MP -Iexternal
LDFLAGS = -fwhole-program -lm -lglfw -lGL # -lX11 -lpthread -lXrandr -lXi -ldl # -lraylib
COMMONFLAGS =

#LDFLAGS += -lasan
#COMMONFLAGS += -fsanitize=address -fsanitize=undefined

$(TARGET): $(OBJS)
	$(CXX) $(COMMONFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(DIR)/%.o: %.c
	@ mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(COMMONFLAGS) -o $@ $<

$(DIR)/%.o: %.cpp
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(COMMONFLAGS) -o $@ $<

-include $(DEPS)
