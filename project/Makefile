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

fmt-check: $(SOURCES) $(HEADERS)
	clang-format -n --Werror $^

src/GS: src/GS.o
src/player: src/player.o

clean:
	rm -f $(OBJECTS) $(TARGETS) project.zip

package:
	zip project.zip $(SOURCES) $(HEADERS) Makefile .clang-format
