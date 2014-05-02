UNAME := $(shell uname)

# Directories
SOURCEDIR = src/
BUILDDIR = build/
INC = -Iinclude/

# Compiler options
CC = clang++
DEBUG = -g
CFLAGS = -std=c++11 -Wall -c $(DEBUG) -DOM_STATIC_BUILD
LFLAGS = -Wall $(DEBUG)

# Files
SRC = $(wildcard $(SOURCEDIR)*.cpp) $(wildcard $(SOURCEDIR)*/*.cpp) $(wildcard $(SOURCEDIR)*/*.c)
OBJS = $(SRC:$(SOURCEDIR)%.cpp=$(BUILDDIR)%.o)

ifeq ($(UNAME), Darwin)
	LIBS = -lglfw3 -framework OpenGL -lglew -framework IOKit \
	-framework CoreFoundation -framework ApplicationServices \
	-framework Foundation -framework AppKit \
	lib/osx/libfreetype-gl.a -lfreetype \
	lib/osx/libOpenMeshCored.a \
	lib/osx/libOpenMeshToolsd.a
	BUILDDIR = ./build/osx/
endif
ifeq ($(UNAME), Linux)
	LIBS = -lglfw -lGL -lGLEW -lfreetype \
	lib/freetype-gl-master/libfreetype-gl.a\
	lib/OpenMesh-2.4/build/Build/lib/OpenMesh/libOpenMeshCored.a \
	lib/OpenMesh-2.4/build/Build/lib/OpenMesh/libOpenMeshToolsd.a
	BUILDDIR = ./build/linux/
endif

# Build target
TARGET = test

all: $(TARGET)

$(TARGET): $(OBJS) $(C_OBJS)
	$(CC) $(LFLAGS) $^ -o $(TARGET) $(LIBS) $(INC)

$(OBJS): $(BUILDDIR)%.o : $(SOURCEDIR)%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@ $(INC)

clean:
	rm -rf $(BUILDDIR)*.o $(BUILDDIR)*/*.o $(TARGET)