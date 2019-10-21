TARGET     := touphScript.exe
VERSION    := 0,0,4,3
STRVERSION := 0.4.3

SOURCES  := $(wildcard *.cpp)
OBJECTS  := $(SOURCES:.cpp=.o) $(TARGET).res

DEP      := make.dep

CXX := i686-w64-mingw32-g++
CC := $(CXX)
RM := del

CXXFLAGS := -Wall -Wextra -pedantic -Wabi -Weffc++ -Wold-style-cast \
	          -Wdouble-promotion -Winit-self -Wmissing-include-dirs \
	          -Woverloaded-virtual -Wsign-promo -Wctor-dtor-privacy \
	          -Wredundant-decls \
						-fstrict-enums -fno-nonansi-builtins -fabi-version=0 -std=c++0x \
						-mconsole -mwin32

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

clean: ; $(RM) $(TARGET) $(OBJECTS) msg.h msg.rc *msg_*.bin $(DEP) 

$(TARGET): $(OBJECTS)

$(TARGET).res: CPPFLAGS += -D APPVERSION=$(VERSION) -D STRVERSION=$(STRVERSION)
$(TARGET).res: $(TARGET).rc
	windres $(CPPFLAGS) $< -O coff -o $@
	
msg.h msg.rc: msg.mc
	windmc -b -c -C 65001 $<

$(DEP): CPPFLAGS += -MM
$(DEP): $(SOURCES) msg.h
	$(CC) $(CXXFLAGS) $(CPPFLAGS) $^ > $@

-include $(DEP)
