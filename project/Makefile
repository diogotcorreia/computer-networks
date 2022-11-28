CC ?= g++
LD ?= g++

TARGETS = src/player src/GS
SOURCES := $(wildcard */*.cpp)
HEADERS := $(wildcard */*.hpp)
OBJECTS := $(SOURCES:.cpp=.o)

CPPFLAGS = -std=c++17 -Wall

# Run `make OPTIM=no` to disable -O3
ifeq ($(strip $(OPTIM)), no)
	CPPFLAGS += -O0
else
	CPPFLAGS += -O3
endif

# Run `make DEBUG=true` to run with debug symbols
ifeq ($(strip $(DEBUG)), yes)
	CPPFLAGS += -g
endif

#LDFLAGS = -fsanitize=address -lasan

.PHONY: all clean fmt

all: $(TARGETS)

fmt: $(SOURCES) $(HEADERS)
	clang-format -i $^

fmt-check: $(SOURCES) $(HEADERS)
	clang-format -n --Werror $^

src/GS: src/game.o src/packet.o
src/player: src/commands.o src/game.o src/packet.o src/player_state.o

clean:
	rm -f $(OBJECTS) $(TARGETS) project.zip

package:
	zip project.zip $(SOURCES) $(HEADERS) Makefile .clang-format
