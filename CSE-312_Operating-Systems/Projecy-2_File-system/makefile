CXX = g++
CXXFLAGS = -std=c++11 -Wall

OBJS = file_system.o main.o
TARGET = filesystem
TEST_OBJS = file_system.o test_file_system.o
TEST_TARGET = test_filesystem

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

$(TEST_TARGET): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TEST_TARGET) $(TEST_OBJS)

file_system.o: file_system.cpp file_system.h
	$(CXX) $(CXXFLAGS) -c file_system.cpp

main.o: main.cpp file_system.h
	$(CXX) $(CXXFLAGS) -c main.cpp

test_file_system.o: test_file_system.cpp file_system.h
	$(CXX) $(CXXFLAGS) -c test_file_system.cpp

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_OBJS) $(TEST_TARGET)
