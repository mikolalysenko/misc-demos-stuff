#i#############################################################################
#
# Makefile
#
###############################################################################

HEADERS  = misc.h
SOURCES  = main.cpp misc.cpp
DEPENDS  = $(SOURCES:.cpp=.d)
OBJECTS  = $(SOURCES:.cpp=.o)
TARGET	 = main

###############################################################################

OPTFLAGS = -g

CC      = g++
CFLAGS  = -Wall -Wno-format -arch i386 $(OPTFLAGS)
LDFLAGS = -framework SDL -framework Cocoa -framework OpenGL -lm


###############################################################################

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -I/Library/Frameworks/SDL.framework/Headers -c $(CFLAGS) SDLMain.m -o SDLMain.o
	$(CC) $(CFLAGS) $(OBJECTS) SDLMain.o $(LDFLAGS) -o $@

clean:
	$(RM) $(OBJECTS) $(DEPENDS) $(TARGET) SDLMain.o

.PHONY: all clean

###############################################################################

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

%.d: %.cpp
	$(CC) -MM $< > $@

###############################################################################

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif

###############################################################################
