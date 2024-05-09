# Compiler
CC = g++

# Compiler flags
CFLAGS = -Wall -Wextra -Wconversion -Wshadow -Wpedantic -std=c++20 -O2

# Define the target executable
TARGET = gps_read

# Define source file
SRCS = gps_read.cpp

# Define the linker flags
LDFLAGS = -li2c -lwiringPi

# Define the object files
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

# Link the object files into the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile the source files into object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OUTPUT_FILE)