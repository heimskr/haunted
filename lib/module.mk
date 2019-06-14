UNISRC		:= uniset uobject uvector uvectr32 uvectr64 unifilt unifunct unistr ustring cmemory bmpset unisetspan  \
			   appendable stringpiece util patternprops uinvchar ustrtrns udataswp cstring umath uarrsort utf_impl \
			   bytesinkutil charstr edits bytestream
INCLUDE		+= -Ilib -Ilib/unicode
COMMONSRC	+= $(patsubst %,src/lib/unicode/%.cpp,$(UNISRC))
COMMONSRC	+= lib/utf8.cpp
SRC			+= lib/formicine/ansi.cpp

build/lib/%.o: lib/%.cpp
	@ mkdir -p "$(shell dirname "$@")"
	$(CC) $(strip $(INCLUDE)) -c $< -o $@

build/lib/unicode/%.o: lib/unicode/%.cpp
	@ mkdir -p "$(shell dirname "$@")"
	$(CC) $(strip $(INCLUDE)) -c $< -o $@