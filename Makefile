# Compiler
CC = clang

# Target architecture
ARCH = -arch arm64

# Source file
SRC = main.c

# Output binary
TARGET = out

# Library directories and libraries
LIBS = -L/usr/local/lib -lraylib /usr/local/lib/libcsoundlib.dylib

# Compiler flags
CFLAGS = $(ARCH)

# Linker flags
LDFLAGS = $(LIBS)

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

# Clean target
clean:
	rm -f $(TARGET)