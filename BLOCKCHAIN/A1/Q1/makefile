# /*
#     * Author: Bratin Mondal
#     * Roll No: 21CS10016

#     * Deparment of Computer Science and Engineering
#     * Indian Institue of Technology, Kharagpur
# */

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -O2 -Wall -Wextra -pthread

# Target executable name
TARGET = 21CS10016_A1_Q1

# Source file
SRC = 21CS10016_A1_Q1.cpp

# Object file 
OBJ = $(SRC:.cpp=.o)

# Default rule
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# Rule to compile the source file into an object file
$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -c $(SRC)

# Clean rule to remove the compiled files
clean:
	rm -f $(OBJ) $(TARGET)

# Phony targets to avoid file conflicts
.PHONY: all clean

