CXX = g++
CXXFLAGS = -std=c++11 -Wall

OBJS = file_system.o main.o
TARGET = filesystem

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

file_system.o: file_system.cpp file_system.h
	$(CXX) $(CXXFLAGS) -c file_system.cpp

main.o: main.cpp file_system.h
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f $(OBJS) $(TARGET)
