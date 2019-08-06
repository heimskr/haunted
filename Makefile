COMPILER		:= clang++
CFLAGS			:= -std=c++2a -g -ggdb -O0 -Wall -Wextra
CFLAGS_ORIG		:= $(CFLAGS)
LDFLAGS			:=
INCLUDE			:=
CC				 = $(COMPILER) $(CFLAGS) $(CHECKFLAGS)
MKBUILD			:= mkdir -p build
CHECK			:= none
OUTPUT			:= build/tests
VALGRIND		:= valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=no
SDKFLAGS		:= --sysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

ifeq ($(CHECK), asan)
	CHECKFLAGS := -fsanitize=address -fno-common
else ifeq ($(CHECK), msan)
	CHECKFLAGS := -fsanitize=memory -fno-common
endif

.PHONY: all test clean depend spotless vars
all: Makefile

# Peter Miller, "Recursive Make Considered Harmful" (http://aegis.sourceforge.net/auug97.pdf)
MODULES			:= src/core src/ui lib src/ui/boxes src/tests
COMMONSRC		:=
SRC				:=
CFLAGS			+= -Iinclude
LDFLAGS			+= -L/usr/local/opt/binutils/lib
EXTRASRC		:=
include $(patsubst %,%/module.mk,$(MODULES))
SRC				+= $(COMMONSRC)
COMMONOBJ		:= $(patsubst src/%.cpp,build/%.o, $(patsubst lib/%.cpp,build/lib/%.o, $(filter %.cpp,$(COMMONSRC))))
OBJ				:= $(patsubst src/%.cpp,build/%.o, $(patsubst lib/%.cpp,build/lib/%.o, $(filter %.cpp,$(SRC))))

OBJ_ALL			= $(OBJ) $(OBJ_PP)
SRC_ALL			= $(SRC) $(SRC_PP)

sinclude $(patsubst %,%/targets.mk,$(MODULES))

all: $(COMMONOBJ) build/test

build/tests: build/tests/tests.o $(COMMONOBJ)
	@ $(MKBUILD)
	$(CC) $(SDKFLAGS) $< $(filter-out $<,$+) -o $@ $(LDFLAGS) $(LDLIBS)

build/%.o: src/%.cpp
	@ mkdir -p "$(shell dirname "$@")"
	$(CC) $(SDKFLAGS) $(strip $(INCLUDE)) -c $< -o $@

grind: $(OUTPUT)
	$(VALGRIND) ./$(OUTPUT)

vars:
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend $(DEPFLAGS) -- $(CC) -- $(SRC_ALL) 2>/dev/null
	@ sed -i .sed 's/^src\//build\//' $(DEPFILE)
	@ rm $(DEPFILE).bak $(DEPFILE).sed

clean:
	rm -rf build
	if [ -e .log ]; then > .log; fi

spotless:
	rm -rf build .log $(DEPFILE)

DEPFILE  = .dep
DEPTOKEN = "\# MAKEDEPENDS"
DEPFLAGS = -Y -f $(DEPFILE) -s $(DEPTOKEN)

depend:
	@ echo $(SRC_ALL); echo

	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend $(DEPFLAGS) -- $(CC) -- $(SRC_ALL) $(EXTRASRC) 2>/dev/null
	@ sed -i .sed 's/^src\//build\//' $(DEPFILE)
	@ rm $(DEPFILE).bak $(DEPFILE).sed

sinclude $(DEPFILE)
