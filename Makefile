CXX=g++
CXXFLAGS=-Wall -g -std=c++11
LDFLAGS=-g
LDLIBS=-lstdc++

all: huffman

huffman: huffman.o my_huffman.o

clean:
	rm -f *.o huffman