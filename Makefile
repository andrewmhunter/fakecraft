CXX = clang++
SOURCE_DIRECORY := src

SRCS := main.cpp mesh.cpp block.cpp chunk.cpp world.cpp \
	util.cpp worldgen.cpp chunk_mesh.cpp entity.cpp collision.cpp \
	serialize.cpp logger.cpp hash.cpp point.cpp fileio.cpp timer.cpp graphics.cpp \
	input.cpp text.cpp ini_parser.cpp config.cpp

SRCS_CXX := $(SRCS:%=$(SOURCE_DIRECORY)/%)

EXTERNAL_SRCS := external/glad.c external/stb_implementation.c

DIR := build
TARGET := fakecraft

OBJS := $(SRCS_CXX:%.cpp=$(DIR)/%.o) $(EXTERNAL_SRCS:%.c=$(DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CFLAGS := -c -O3 -g -Wall -Wextra -fdiagnostics-color=always # -march=native
CXXFLAGS := $(CFLAGS) -std=c++23
CPPFLAGS := -MD -MP -Iexternal -I$(SOURCE_DIRECORY)
LDFLAGS = -fwhole-program -lm -lglfw -lGL # -lX11 -lpthread -lXrandr -lXi -ldl # -lraylib
COMMONFLAGS =

#LDFLAGS += -lasan
#COMMONFLAGS += -fsanitize=address -fsanitize=undefined

$(TARGET): $(OBJS)
	$(CXX) $(COMMONFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(DIR)/%.o: %.cpp
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(COMMONFLAGS) -o $@ $<

$(DIR)/%.o: %.c
	@ mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(COMMONFLAGS) -o $@ $<

.PHONY: debug
debug:
	@ echo SRCS: $(SRCS)
	@ echo SRCS_CXX: $(SRCS_CXX)
	@ echo EXTERNAL_SRCS: $(EXTERNAL_SRCS)
	@ echo OBJS: $(OBJS)

.PHONY: clean
clean:
	rm -r $(DIR)

-include $(DEPS)
