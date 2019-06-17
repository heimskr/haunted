INCLUDE		+= -Ilib
COMMONSRC	+= lib/utf8.cpp
SRC			+= lib/formicine/ansi.cpp

build/lib/%.o: lib/%.cpp
	@ mkdir -p "$(shell dirname "$@")"
	$(CC) $(strip $(INCLUDE)) -c $< -o $@
