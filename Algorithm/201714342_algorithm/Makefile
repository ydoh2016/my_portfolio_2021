CXX := g++

CXXFLAGS := -std=c++0x -O3
CPPFLAGS := -Iinclude

SRC := src
OBJ := obj

SRCS := $(wildcard $(SRC)/*.cpp)
OBJS := $(SRCS:$(SRC)/%.cpp=$(OBJ)/%.o)

TARGET := program


.PHONY: all run clean

all: $(TARGET)

run: $(TARGET)
	./$(TARGET) input/5t1.in
	./$(TARGET) input/5t2.in
	./$(TARGET) input/5t3.in
	./$(TARGET) input/6t1.in
	./$(TARGET) input/6t2.in
	./$(TARGET) input/6t3.in
	./$(TARGET) input/7t1.in
	./$(TARGET) input/7t2.in
	./$(TARGET) input/7t3.in

time: $(TARGET)
	./$(TARGET) input/test4.in
	./$(TARGET) input/test5.in
	./$(TARGET) input/test6.in
	./$(TARGET) input/test7.in

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.cpp | $(OBJ)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(OBJ):
	mkdir -p $@

clean:
	rm -rv $(TARGET) $(OBJ)

