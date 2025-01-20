EXECUTABLE=ormGenerate

DEFINES=
INCLUDES=

CC=gcc
CPP=g++

CFLAGS=-Wall -fsanitize=address -O3 -g
CPPFLAGS=-Wall -fsanitize=address -O3 -g
LDFLAGS=-fsanitize=address

SOURCES_CPP=./main.cpp
SOURCES_C=

OBJECTS_CPP=$(SOURCES_CPP:.cpp=.o)
OBJECTS_C=$(SOURCES_C:.c=.o)

.PHONY: clean example

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS_CPP) $(OBJECTS_C)
	$(CPP) -o $(EXECUTABLE) $(OBJECTS_CPP) $(OBJECTS_C) $(LDFLAGS)

%.o: %.cpp
	$(CPP) $(INCLUDES) $(DEFINES) $(CPPFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(INCLUDES) $(DEFINES) $(CFLAGS) -c -o $@ $<

clean:
	rm -f ./*.o
	rm -f ./$(EXECUTABLE)

example: all
	./ormGenerate ./DiamondPlate007D_2K-JPG_Color.jpg jpg
