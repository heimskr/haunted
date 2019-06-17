COMPILER		:= clang++
CFLAGS			:= -std=c++2a -g -O0 -Wall -Wextra
CFLAGS_ORIG		:= $(CFLAGS)
LDFLAGS			:=
INCLUDE			:=
CC				 = $(COMPILER) $(CFLAGS) $(CHECKFLAGS)
MKBUILD			:= mkdir -p build
CHECK			:= asan
OUTPUT			:= build/tests

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
LDFLAGS			+= 
include $(patsubst %,%/module.mk,$(MODULES))
SRC				+= $(COMMONSRC)
COMMONOBJ		:= $(patsubst src/%.cpp,build/%.o, $(patsubst lib/%.cpp,build/lib/%.o, $(filter %.cpp,$(COMMONSRC))))
OBJ				:= $(patsubst src/%.cpp,build/%.o, $(patsubst lib/%.cpp,build/lib/%.o, $(filter %.cpp,$(SRC))))

OBJ_ALL			:= $(OBJ) $(OBJ_PP)
SRC_ALL			:= $(SRC) $(SRC_PP)

sinclude $(patsubst %,%/targets.mk,$(MODULES))

all: $(COMMONOBJ) build/test

build/tests: build/tests/tests.o $(COMMONOBJ)
	@ $(MKBUILD)
	$(CC) $< $(filter-out $<,$+) -o $@ $(LDFLAGS) $(LDLIBS)

build/%.o: src/%.cpp
	@ mkdir -p "$(shell dirname "$@")"
	$(CC) $(strip $(INCLUDE)) -c $< -o $@

grind: $(OUTPUT)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=no ./$(OUTPUT)

vars:
	@ echo COMMONOBJ: $(COMMONOBJ); echo
	@ echo OBJ: $(OBJ); echo
	@ echo COMMONSRC: $(COMMONSRC); echo
	@ echo SRC: $(SRC); echo

clean:
	rm -rf build .log

spotless:
	rm -rf build .log $(DEPFILE)

DEPFILE  = .dep
DEPTOKEN = "\# MAKEDEPENDS"
DEPFLAGS = -Y -f $(DEPFILE) -s $(DEPTOKEN)

depend:
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend $(DEPFLAGS) -- $(CC) -- $(SRC_ALL) 2>/dev/null
	@ sed -i .sed 's/^src\//build\//' $(DEPFILE)
	@ rm $(DEPFILE).bak $(DEPFILE).sed

sinclude $(DEPFILE)
