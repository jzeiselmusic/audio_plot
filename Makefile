# Compiler
CC = clang

# Target architecture
ARCH = -arch arm64

# Source file
SRC = main.c

# Output binary
TARGET = out

# Library directories and libraries
LIBS = -L/usr/local/lib -lcsoundlib -lsoundio -lraylib -framework CoreAudio

# Compiler flags
CFLAGS = $(ARCH)

# Linker flags
LDFLAGS = $(LIBS)

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRC)
	cp ../CSoundLib/inc/csoundlib.h ./csoundlib.h
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)
	install_name_tool -add_rpath /usr/local/lib ./out

# Clean target
clean:
	rm -f $(TARGET)