CC ?= g++
LD ?= g++

TARGETS = src/player src/GS
SOURCES := $(wildcard */*.cpp)
HEADERS := $(wildcard */*.hpp)
OBJECTS := $(SOURCES:.cpp=.o)

CFLAGS = -O3 -Wall

.PHONY: all clean fmt

all: $(TARGETS)

fmt: $(SOURCES) $(HEADERS)
	clang-format -i $^

src/GS: src/packet.o
src/player: src/packet.o

clean:
	rm -f $(OBJECTS) $(TARGETS) project.zip

package:
	zip project.zip $(SOURCES) $(HEADERS) Makefile .clang-format
