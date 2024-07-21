# /*
#     * Author: Bratin Mondal
#     * Roll No: 21CS10016

#     * Deparment of Computer Science and Engineering
#     * Indian Institue of Technology, Kharagpur
# */

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -O2 -Wall -Wextra

# Target executable name
TARGET = 21CS10016_A1_Q2

TARGET_KEY = 21CS10016_A1_Q2_KeyGen

# Source file
SRC = 21CS10016_A1_Q2.cpp

# Source file for key generation
SRC_KEY = 21CS10016_A1_Q2_KeyGen.cpp

# Object file
OBJ = $(SRC:.cpp=.o)

# Object file for key generation
OBJ_KEY = $(SRC_KEY:.cpp=.o)

# Default rule
all: $(TARGET) $(TARGET_KEY)

# Rule to build the target executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# Rule to compile the source file into an object file
$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -c $(SRC)

# Rule to build the target executable for key generation
$(TARGET_KEY): $(OBJ_KEY)
	$(CXX) $(CXXFLAGS) -o $(TARGET_KEY) $(OBJ_KEY)

# Rule to compile the source file into an object file for key generation
$(OBJ_KEY): $(SRC_KEY)
	$(CXX) $(CXXFLAGS) -c $(SRC_KEY)

# Clean rule to remove the compiled files
clean:
	rm -f $(OBJ) $(TARGET) $(OBJ_KEY) $(TARGET_KEY)

# Phony targets to avoid file conflicts
.PHONY: all clean
