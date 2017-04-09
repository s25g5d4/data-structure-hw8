CXX=g++
CXXFLAGS=-Wall -g -std=c++11
LDFLAGS=-g
LDLIBS=-lstdc++

all: hw8-B023040011

hw8: hw8-B023040011.o

clean:
	rm -f *.o hw8-B023040011