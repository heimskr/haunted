COMPILER		:= clang++
CFLAGS			:= -std=c++2a -stdlib=libc++ -g -O0 -Wall -Wextra -fdiagnostics-color=always
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

.PHONY: all test clean depend spotless
all: Makefile

# Peter Miller, "Recursive Make Considered Harmful" (http://aegis.sourceforge.net/auug97.pdf)
MODULES			:= core lib ui ui/boxes tests
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
	$(CC) $(strip $(INCLUDE)) -c $< -o $@

grind: $(OUTPUT)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=no ./$(OUTPUT)

clean:
	rm -f .log
	@ tmpdir=".build.$$RANDOM"; mkdir "$$tmpdir";                                \
	  if [ -e build/lib/unicode ]; then echo "Saving Unicode directory.";        \
	      mv build/lib/unicode "$$tmpdir/unicode";  fi;                          \
	  rm -rf build;                                                              \
	  if [ -e "$$tmpdir/unicode" ];  then echo "Restoring Unicode directory.";   \
	      mkdir -p build/lib;      mv "$$tmpdir/unicode"  build/lib/unicode; fi; \
	  rm -rf "$$tmpdir";

spotless:
	rm -rf build $(DEPFILE)

DEPFILE  = .dep
DEPTOKEN = "\# MAKEDEPENDS"
DEPFLAGS = -Y -f $(DEPFILE) -s $(DEPTOKEN)

depend:
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend $(DEPFLAGS) -- $(CC) -- $(SRC_ALL) 2>/dev/null
	@ sed -i .sed 's/^src\//build\//' $(DEPFILE)
	@ rm $(DEPFILE).bak $(DEPFILE).sed

sinclude $(DEPFILE)
