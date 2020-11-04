EXTRASRC += src/tests/test.cpp

build/test: build/tests/Test.o $(OBJECTS)
	@ $(MKBUILD)
	$(CC) $(INCLUDE) $^ -o $@ $(LDFLAGS) $(LDLIBS)

infer: build/tests/Test.o $(OBJ)
	@ $(MKBUILD)
	infer run -- $(CC) $(INCLUDE) $^ -o $@ $(LDFLAGS) $(LDLIBS)

test: utest

dtest: build/test
	./$^

iltest: build/test
	./$^ input

itest: build/test
	./$^ input

mtest: build/test
	./$^ margins

debug: build/test
	lldb build/test

ttest: build/test
	./$^ textbox

uttest: build/test
	./$^ unittextbox

uetest: build/test
	./$^ unitexpandobox

utest: build/test
	./$^ unit

etest: build/test
	./$^ expandobox

LLDB := /Applications/Xcode.app/Contents/Developer/usr/bin/lldb

dbg: build/test
	echo "run expandobox" | $(LLDB) build/test

ktest: build/test
	./$^ key

k0test: build/test
	./$^ key none

kbtest: build/test
	./$^ key basic

kntest: build/test
	./$^ key normal

katest: build/test
	./$^ key any

khtest: build/test
	./$^ key highlight

kmtest: build/test
	./$^ key motion

ustest: build/test
	./$^ unitustring
