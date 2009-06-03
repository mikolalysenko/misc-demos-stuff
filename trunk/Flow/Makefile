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
LDFLAGS = -framework OpenCV -framework OpenGL -framework GLUT -lm


###############################################################################

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	$(RM) $(OBJECTS) $(DEPENDS)
	$(RM) $(TARGET)

.PHONY: all clean

###############################################################################

%.o: %.C
	$(CC) -c $(CFLAGS) $< -o $@

%.d: %.C
	$(CC) -MM $< > $@

###############################################################################

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif

###############################################################################
