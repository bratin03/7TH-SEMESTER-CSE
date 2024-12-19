# /*
#  * CS60038: Advances in Operating Systems Design
#  * Assignment 2
#  * Members:
#  * > 21CS30037 - Datta Ksheeraj
#  * > 21CS10016 - Bratin Mondal
#  *
#  * Department of Computer Science and Engineering,
#  * Indian Institute of Technology Kharagpur
#  */


# Define variables
CC = gcc
CFLAGS = -Ilibgettaskinfo
LDFLAGS = -L. -lgettaskinfo -Wl,-rpath,.
LIB_DIR = libgettaskinfo
LIB_SRC = $(LIB_DIR)/libgettaskinfo.c
LIB = libgettaskinfo.so

# Targets
TARGET1 = test_gettaskinfo
TARGET2 = raw_test

# Default target
all: $(TARGET1) $(TARGET2)

# Build the shared library
$(LIB): $(LIB_SRC)
	$(CC) -o $@ -fPIC -shared $(LIB_SRC)

# Build the test program (test_gettaskinfo)
$(TARGET1): test/test_gettaskinfo.c $(LIB)
	$(CC) -o $(TARGET1) test/test_gettaskinfo.c $(CFLAGS) $(LDFLAGS)

# Build the raw_test program
$(TARGET2): test/raw_test.c
	$(CC) -o $(TARGET2) test/raw_test.c

# Clean up build artifacts
clean:
	rm -f $(TARGET1) $(TARGET2) $(LIB)
