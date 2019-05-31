build/test: build/tests/test.o $(OBJ)
	@ $(MKBUILD)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS)

test: build/test
	./$^
