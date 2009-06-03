#i#############################################################################
#
# Makefile
#
###############################################################################

HEADERS  = 
SOURCES  = main.cc
DEPENDS  = $(SOURCES:.cc=.d)
OBJECTS  = $(SOURCES:.cc=.o)
TARGET	 = main

###############################################################################

OPTFLAGS = -O3

CC      = g++
CFLAGS  = -Wall -Wno-format -arch i386 $(OPTFLAGS)
LDFLAGS = -lm -lfftw -framework OpenAL


###############################################################################

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	$(RM) $(OBJECTS) $(DEPENDS) $(TARGET)

.PHONY: all clean

###############################################################################

%.o: %.cc
	$(CC) -c $(CFLAGS) $< -o $@

%.d: %.cc
	$(CC) -MM $< > $@

###############################################################################

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif

###############################################################################
