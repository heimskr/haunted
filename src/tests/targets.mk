EXTRASRC += src/tests/test.cpp

build/test: build/tests/test.o $(OBJ)
	@ $(MKBUILD)
	$(CC) $(INCLUDE) $^ -o $@ $(LDFLAGS) $(LDLIBS)

infer: build/tests/test.o $(OBJ)
	@ $(MKBUILD)
	infer run -- $(CC) $(INCLUDE) $^ -o $@ $(LDFLAGS) $(LDLIBS)

test: build/test
	./$^ 2> .log

iltest: build/test
	./$^ input 2> .log

itest: build/test
	./$^ input 2> .log

mtest: build/test
	./$^ margins

debug: build/test
	lldb build/test

ttest: build/test
	./$^ textbox 2> .log

uttest: build/test
	./$^ unittextbox 2> .log

uetest: build/test
	./$^ unitexpandobox 2> .log

utest: build/test
	./$^ unit 2> .log

etest: build/test
	./$^ expandobox 2> .log

