UNISRC    := uniset uobject uvector uvectr32 uvectr64 unifilt unifunct unistr ustring cmemory bmpset unisetspan  \
             appendable stringpiece util patternprops uinvchar ustrtrns udataswp cstring umath uarrsort utf_impl \
             bytesinkutil charstr edits bytestream
INCLUDE   += -Iinclude/lib
COMMONSRC += $(patsubst %,src/lib/unicode/%.cpp,$(UNISRC))
COMMONSRC += src/lib/utf8.cpp
