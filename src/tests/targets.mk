EXTRASRC += src/tests/test.cpp

build/test: build/tests/test.o $(OBJ)
	@ $(MKBUILD)
	$(CC) $(INCLUDE) $^ -o $@ $(LDFLAGS) $(LDLIBS)

infer: build/tests/test.o $(OBJ)
	@ $(MKBUILD)
	infer run -- $(CC) $(INCLUDE) $^ -o $@ $(LDFLAGS) $(LDLIBS)

test: build/test
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

