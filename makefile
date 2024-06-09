# Compiler
CC = g++

# Compiler flags
CFLAGS = -std=c++11 -Wall

# Executable name
EXECUTABLE = Memory_Simulator

# Main program
MAIN = main.cpp

# Source files
SOURCES = sim_mem.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Compile and link the program
$(EXECUTABLE): $(OBJECTS) $(MAIN)
	$(CC) $(CFLAGS) $(OBJECTS) $(MAIN) -o $(EXECUTABLE)

# Compile the source files
%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean the object files and the executable
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
