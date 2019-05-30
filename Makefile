COMPILER		:= clang++
CFLAGS			:= -std=c++2a -stdlib=libc++ -g -O0 -Wall -Wextra -fdiagnostics-color=always
CFLAGS_ORIG		:= $(CFLAGS)
LDFLAGS			:=
CC				 = $(COMPILER) $(CFLAGS) $(CHECKFLAGS)
MKBUILD			:= mkdir -p build
CHECK			:= asan
OUTPUT			:= build/tests

ifeq ($(CHECK), asan)
	CHECKFLAGS := -fsanitize=address -fno-common
else ifeq ($(CHECK), msan)
	CHECKFLAGS := -fsanitize=memory -fno-common
endif

.PHONY: all test clean depend spotless
all: Makefile

# Peter Miller, "Recursive Make Considered Harmful" (http://aegis.sourceforge.net/auug97.pdf)
MODULES			:= core ui ui/boxes
COMMONSRC		:=
SRC				:=
CFLAGS			+= -Iinclude
LDFLAGS			+= 
include $(patsubst %,src/%/module.mk,$(MODULES))
SRC				+= $(COMMONSRC)
COMMONOBJ		:= $(patsubst src/%.cpp,build/%.o, $(filter %.cpp,$(COMMONSRC)))
OBJ				:= $(patsubst src/%.cpp,build/%.o, $(filter %.cpp,$(SRC)))

OBJ_ALL			:= $(OBJ) $(OBJ_PP)
SRC_ALL			:= $(SRC) $(SRC_PP)

sinclude $(patsubst %,src/%/targets.mk,$(MODULES))

all: $(COMMONOBJ)

build/tests: build/tests/tests.o $(COMMONOBJ)
	@ $(MKBUILD)
	$(CC) $< $(filter-out $<,$+) -o $@ $(LDFLAGS) $(LDLIBS)

build/%.o: src/%.cpp
	@ mkdir -p "$(shell dirname "$@")"
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

test: $(OUTPUT)
	./$(OUTPUT)

grind: $(OUTPUT)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=no ./$(OUTPUT)

clean:
	rm -rf build

DEPFILE  = .dep
DEPTOKEN = "\# MAKEDEPENDS"
DEPFLAGS = -f $(DEPFILE) -s $(DEPTOKEN)

depend:
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend $(DEPFLAGS) -- $(CC) -- $(SRC_ALL) 2>/dev/null
	@ sed -i .sed 's/^src\//build\//' $(DEPFILE)
	@ rm $(DEPFILE).bak $(DEPFILE).sed

sinclude $(DEPFILE)
