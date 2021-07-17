CXX=g++
CXXFLAGS=-g -std=c++17 -Wall -pedantic
LDFLAGS=-lstdc++fs
BIN=run

SRC=$(wildcard *.cpp)
OBJ=$(SRC:%.cpp=%.o)

all: $(OBJ)
	$(CXX) -o $(BIN) $^ $(LDFLAGS)

%.o: %.c
	$(CXX) $@ -c $<

clean:
	rm -f *.o
	rm $(BIN)
