CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
TARGET = minic
SRCS = $(wildcard src/*.cpp)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean

