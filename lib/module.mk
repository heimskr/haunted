INCLUDE		+= -Ilib
COMMONSRC	+= lib/utf8.cpp lib/superstring.cpp
SRC			+= lib/formicine/ansi.cpp

build/lib/%.o: lib/%.cpp
	@ mkdir -p "$(shell dirname "$@")"
	$(CC) $(SDKFLAGS) $(strip $(INCLUDE)) -c $< -o $@
