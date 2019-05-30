#include "haunted/boxes/hbox.h"

namespace haunted::boxes {
	void hbox::resize(const position &new_pos) {
		if (control *left = (*this)[0])
			left->resize({new_pos.left, new_pos.top, size_one(), new_pos.height});
		
		if (control *right = (*this)[1])
			right->resize({new_pos.left + size_one(), new_pos.top, size_two(), new_pos.height});
	}

	int hbox::get_size() const {
		return pos.width;
	}
}
