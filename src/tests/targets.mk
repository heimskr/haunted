build/test: build/tests/test.o $(OBJ)
	@ $(MKBUILD)
	$(CC) $(INCLUDE) $^ -o $@ $(LDFLAGS) $(LDLIBS)

infer: build/tests/test.o $(OBJ)
	@ $(MKBUILD)
	infer run -- $(CC) $(INCLUDE) $^ -o $@ $(LDFLAGS) $(LDLIBS)

test: build/test
	./$^ 2>> .log

iltest: build/test
	./$^ input 2>> .log

itest: build/test
	./$^ input

debug: build/test
	lldb build/test
