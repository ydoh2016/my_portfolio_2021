CXX=g++
CXXFLAGS=-std=c++17

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

all: hello.o word.o
	$(CXX) -o hello hello.o
	$(CXX) -o word word.o

clean:
	rm -f hello word *.o

