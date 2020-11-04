#ifndef HAUNTED_UI_COLOREDCONTROL_H_
#define HAUNTED_UI_COLOREDCONTROL_H_

#include "haunted/ui/Control.h"
#include "haunted/ui/Colored.h"

namespace Haunted::UI {
	struct ColoredControl: public Control, public Colored {
		using Control::Control;
		virtual void clearRect() override;
	};
}

#endif
