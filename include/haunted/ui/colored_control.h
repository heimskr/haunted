#ifndef HAUNTED_UI_COLORED_CONTROL_H_
#define HAUNTED_UI_COLORED_CONTROL_H_

#include "haunted/ui/control.h"
#include "haunted/ui/colored.h"

namespace Haunted::UI {
	class colored_control: public Control, public colored {
		public:
			using Control::Control;

			virtual void clear_rect() override;
	};
}

#endif
