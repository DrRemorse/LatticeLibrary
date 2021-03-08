######################
#  library makefile  #
######################

# (1) select which components to use
#USE_COMPRESSION = library/compression
#USE_NETWORK = library/network
#USE_SCRIPT  = library/script
USE_SOUND   = library/sound
USE_OPENGL  = library/opengl library/voxels

BUILDOPT = -ggdb3 -march=native
# output file
OUTPUT   = ./libibrary.a
OUTTEST  = ./test

##############################################################

# code folders
SOURCE_DIRS  = src
LIBRARY_DIRS = library library/bitmap library/math library/math/kine \
				library/noise library/storage library/threading library/timing \
				$(USE_OPENGL) $(USE_COMPRESSION) $(USE_NETWORK) $(USE_SCRIPT) $(USE_SOUND)

# compiler
CC = g++
CC += $(BUILDOPT) -std=c++11 -mstackrealign
# compiler flags
CCFLAGS = -c -MMD -Wall -Wextra -Wno-write-strings -Iinc -Iinclude `pkg-config --static --cflags glfw3`
# linker flags
LDFLAGS = -Llib/linux -L. -llibrary -lbass -lGLEW -DGLEW_STATIC `pkg-config --static --libs glfw3` -Wl,-rpath,lib/linux
ifeq ($(OS),Windows_NT)
LDFLAGS = -m32 -Llib/win32 -L. -static -lbassdll -lglfw3 -lgdi32 -lglew32s -lopengl32
endif

##############################################################

# make pipeline
CCDIRS  = $(foreach dir, $(LIBRARY_DIRS), $(dir)/*.c)
CCMODS  = $(wildcard $(CCDIRS))
CXXDIRS = $(foreach dir, $(LIBRARY_DIRS), $(dir)/*.cpp)
CXXMODS = $(wildcard $(CXXDIRS))

# compile each .c to .o
.c.o:
	$(CC) $(CCFLAGS) $< -o $@

# compile each .cpp to .o
.cpp.o:
	$(CC) $(CCFLAGS) $< -o $@

# convert .c to .o
CCOBJS  = $(CCMODS:.c=.o)
# convert .cpp to .o
CXXOBJS = $(CXXMODS:.cpp=.o)
# convert .o to .d
DEPENDS = $(CXXOBJS:.o=.d) $(CCOBJS:.o=.d)
# test modules
TESTDIRS = $(foreach dir, $(SOURCE_DIRS), $(dir)/*.cpp)
TESTMODS = $(wildcard $(TESTDIRS))
TESTOBJS = $(TESTMODS:.cpp=.o)
TESTDEPS = $(TESTOBJS:.o=.d)

.PHONY: all testmods tests clean

# link all OBJS using CC and link with LFLAGS, then output to OUTPUT
all: $(CXXOBJS) $(CCOBJS)
	$(AR) cr $(OUTPUT) $(CXXOBJS) $(CCOBJS)

tests: all $(TESTOBJS)
	$(CC) $(TESTOBJS) $(LDFLAGS) -o $(OUTTEST)

# remove each known .o file, and output
clean:
	$(RM) $(CXXOBJS) $(CCOBJS) $(TESTOBJS) $(DEPENDS) $(TESTDEPS) $(OUTPUT) $(OUTTEST)

-include $(DEPENDS) $(TESTDEPS)
