#ifndef HAUNTED_DEFS_H_
#define HAUNTED_DEFS_H_

namespace haunted {
	enum side {left, right, top, bottom};

	struct position {
		int left, top, width, height;
		position(int l, int t, int w, int h): left(l), top(t), width(w), height(h) {}
		position(): left(-1), top(-1), width(-1), height(-1) {}
		operator bool() const;
	};
}

#endif
