UNISRC		:= uniset uobject uvector uvectr32 uvectr64 unifilt unifunct unistr ustring cmemory bmpset unisetspan  \
			   appendable stringpiece util patternprops uinvchar ustrtrns udataswp cstring umath uarrsort utf_impl \
			   bytesinkutil charstr edits bytestream
INCLUDE		+= -Ilib -Ilib/icu/icu4c/source/common
COMMONSRC	+= $(patsubst %,lib/icu/icu4c/source/common/%.cpp,$(UNISRC))
COMMONSRC	+= lib/utf8.cpp
SRC			+= lib/formicine/ansi.cpp
