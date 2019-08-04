#include <stdexcept>

#include "ui/boxes/dualbox.h"
#include "ui/boxes/propobox.h"

namespace haunted::ui::boxes {
	propobox::propobox(const position &pos, double ratio): dualbox(pos), ratio(ratio) {
		if (ratio < 0)
			throw std::domain_error("Box ratio cannot be negative");
	}

	void propobox::set_ratio(const double ratio_) {
		ratio = ratio_;
	}

	int propobox::size_one() const {
		return get_size() - size_two();
	}

	int propobox::size_two() const {
		return get_size() / (1.0 + ratio);
	}

	int propobox::get_size() const {
		return orientation == box_orientation::horizontal? pos.width : pos.height;
	}
}
