TARGET     := touphScript.exe
VERSION    := 0,0,6,0
STRVERSION := 0.6.0

SOURCES  := $(wildcard *.cpp)
OBJECTS  := $(SOURCES:.cpp=.o) $(TARGET).res

DEP      := make.dep

CXX := g++
CC := $(CXX)
RM := del
SHELL := $(ComSpec)

CXXFLAGS := -Wall -Wextra -Wpedantic -Werror -std=c++23

LDLIBS := libz.a -lshlwapi -lShell32 

.PHONY: all static debug profile clean

all: CXXFLAGS += -O3
all: LDFLAGS += -s
all: $(TARGET)

static: LDFLAGS += -static
static: all

debug: CXXFLAGS += -g -Og
debug: $(TARGET)

profile: CXXFLAGS += -pg -O3
profile: LDFLAGS += -pg
profile: $(TARGET)

clean: ; $(RM) $(TARGET) $(OBJECTS) $(DEP)
$(TARGET): $(OBJECTS)

$(TARGET).res: CPPFLAGS += -D APPVERSION=$(VERSION) -D STRVERSION=$(STRVERSION)
$(TARGET).res: $(TARGET).rc
	windres $(CPPFLAGS) $< -O coff -o $@

$(DEP): CPPFLAGS += -MM
$(DEP): $(SOURCES)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) $^ > $@

-include $(DEP)
