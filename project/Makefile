CC ?= g++
LD ?= g++

TARGETS = src/player src/GS
SOURCES := $(wildcard */*.cpp)
HEADERS := $(wildcard */*.hpp)
OBJECTS := $(SOURCES:.cpp=.o)

CPPFLAGS = -std=c++17 -O3 -Wall

#CPPFLAGS += -g
#LDFLAGS = -fsanitize=address -lasan

.PHONY: all clean fmt

all: $(TARGETS)

fmt: $(SOURCES) $(HEADERS)
	clang-format -i $^

fmt-check: $(SOURCES) $(HEADERS)
	clang-format -n --Werror $^

src/GS: src/packet.o
src/player: src/commands.o src/packet.o

clean:
	rm -f $(OBJECTS) $(TARGETS) project.zip

package:
	zip project.zip $(SOURCES) $(HEADERS) Makefile .clang-format
