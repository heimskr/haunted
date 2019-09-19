COMPILER		:= g++
CFLAGS			:= -std=c++2a -g -ggdb -O0 -Wall -Wextra
INCLUDE			:= -Iinclude
LDFLAGS			:= -pthread
CC				 = $(COMPILER) $(strip $(CFLAGS) $(CHECKFLAGS))
VALGRIND		:= valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-reachable=no
MKBUILD			:= mkdir -p build
OUTPUT			:= build/tests

ifeq ($(shell uname -s), Darwin)
	SDKFLAGS	:= --sysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
endif

ifeq ($(CHECK), asan)
	CHECKFLAGS := -fsanitize=address -fno-common
else ifeq ($(CHECK), msan)
	CHECKFLAGS := -fsanitize=memory -fno-common
endif

.PHONY: all test clean depend spotless vars


SOURCES			:= $(shell find -L src -name '*.cpp' | sed -nE '/(tests?|test_.+)\.cpp$$/!p')
OBJECTS			:= $(patsubst src/%.cpp,build/%.o, $(SOURCES))

sinclude $(shell find src -name 'targets.mk')


all: $(OBJECTS) build/test

build/tests: build/tests/tests.o $(OBJECTS)
	@ $(MKBUILD)
	$(CC) $(SDKFLAGS) $< $(filter-out $<,$+) -o $@ $(LDFLAGS) $(LDLIBS)

build/%.o: src/%.cpp
	@ mkdir -p "$(shell dirname "$@")"
	$(CC) $(strip $(SDKFLAGS) $(INCLUDE)) -c $< -o $@

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
	@ sed -i.sed 's/^src\//build\//' $(DEPFILE)
	@ rm $(DEPFILE).bak $(DEPFILE).sed

sinclude $(DEPFILE)
