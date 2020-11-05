#include <deque>

#include "haunted/core/Terminal.h"
#include "haunted/ui/Colored.h"

namespace Haunted::UI {
	Colored::~Colored() = default;

	ansi::color Colored::findColor(ansi::color_type type) const {
		Container *p = getParent();

		// If the control doesn't need to inherit a color, that would save us the effort of checking its ancestors.
		if (type == ansi::color_type::background && !inheritBackground)
			return background;

		if (type == ansi::color_type::foreground && !inheritForeground)
			return foreground;

		while (p != nullptr) {
			if (Colored *pcolored = dynamic_cast<Colored *>(p)) {
				// If we find a control that's also an instance of colored, let it determine the color for us.
				ansi::color found = pcolored->findColor(type);
				return found;
			} else if (Control *pcontrol = dynamic_cast<Control *>(p)) {
				if (pcontrol->getTerminal() == pcontrol->getParent()) {
					// If we've reached the terminal and still haven't found any control with a color preference,
					// give up.
					break;
				}

				// Otherwise, keep going up.
				p = pcontrol->getParent();
			} else {
				// At this point, because the parent of the previous control isn't null, but it (almost surely) also
				// isn't a terminal and it isn't a control, the parent must surely be a plain container or some unknown
				// subtype. This shouldn't happen. If it does, stop searching.
				DBG("Unknown container at " << p << "; returning default " <<
					(type == ansi::color_type::foreground? "fore" : "back") << "ground color.");
				break;
			}
		}

		// At this point, the parent is null (shouldn't be possible for anything that's ready to be drawn) or an unknown
		// type of container. This shouldn't happen, but if it does, we'll just return the terminal's default color.
		return ansi::color::normal;
	}

	void Colored::draw() {
		applyColors();
	}

	void Colored::focus() {
		applyColors();
	}

	Colored & Colored::applyColors() {
		if (Terminal *term = getTerminal())
			term->colors.setBoth(findColor(ansi::color_type::foreground), findColor(ansi::color_type::background));

		return *this;
	}

	Colored & Colored::tryColors(bool find) {
		if (Terminal *term = getTerminal()) {
			ansi::color fg = foreground, bg = background;
			if (find) {
				fg = findColor(ansi::color_type::foreground);
				bg = findColor(ansi::color_type::background);
			}

			term->colors.setForeground(fg);
			term->colors.setBackground(bg);
		}

		return *this;
	}

	Colored & Colored::uncolor() {
		if (Terminal *term = getTerminal())
			term->resetColors();
		return *this;
	}

	bool Colored::propagate(ansi::color_type type) {
		// If this isn't a container, there's nothing to propagate to.
		Container *cont = dynamic_cast<Container *>(this);
		if (cont == nullptr)
			return false;

		bool is_bg = (static_cast<int>(type) & static_cast<int>(ansi::color_type::background)) != 0;
		bool is_fg = (static_cast<int>(type) & static_cast<int>(ansi::color_type::foreground)) != 0;

		std::deque<Control *> queue(cont->getChildren().begin(), cont->getChildren().end());

		while (!queue.empty()) {
			Control *child = queue.front();
			queue.pop_front();

			Colored *colored_child = dynamic_cast<Colored *>(child);
			if (colored_child == nullptr) {
				if (Container *container_child = dynamic_cast<Container *>(child)) {
					queue.insert(queue.end(), container_child->getChildren().begin(),
					                          container_child->getChildren().end());
				}

				continue;
			}

			bool changed = false;

			if (is_bg && colored_child->inheritBackground && colored_child->getBackground() != background) {
				colored_child->background = background;
				changed = true;
			}
			
			if (is_fg && colored_child->inheritForeground && colored_child->getForeground() != foreground) {
				colored_child->foreground = foreground;
				changed = true;
			}

			if (changed) {
				if (Container *container_child = dynamic_cast<Container *>(child)) {
					queue.insert(queue.end(), container_child->getChildren().begin(),
					                          container_child->getChildren().end());
				}
			}
		}

		draw();
		return true;
	}

	bool Colored::setForeground(ansi::color foreground_) {
		if (foreground != foreground_) {
			foreground = foreground_;
			propagate(ansi::color_type::foreground);
			return true;
		}

		return false;
	}

	bool Colored::setBackground(ansi::color background_) {
		if (background != background_) {
			background = background_;
			propagate(ansi::color_type::background);
			return true;
		}

		return false;
	}

	bool Colored::setColors(ansi::color foreground_, ansi::color background_) {
		bool fg_changed = setForeground(foreground_);
		bool bg_changed = setBackground(background_);

		if (fg_changed || bg_changed) {
			draw();
			return true;
		}

		return false;
	}

	bool Colored::setColors(ansi::color foreground_, ansi::color background_, bool inherit_fg, bool inherit_bg) {
		bool inherit_changed = setInherit(inherit_fg, inherit_bg);
		bool colors_changed = setColors(foreground_, background_);
		return inherit_changed || colors_changed;
	}

	bool Colored::setInherit(bool inherit_fg, bool inherit_bg) {
		bool changed = false;

		if (inheritForeground != inherit_fg) {
			inheritForeground = inherit_fg;
			changed = true;
			setForeground(findColor(ansi::color_type::foreground));
		}

		if (inheritBackground != inherit_bg) {
			inheritBackground = inherit_bg;
			changed = true;
			setBackground(findColor(ansi::color_type::background));
		}
		
		return changed;
	}

	void swap(Colored &left, Colored &right) {
		std::swap(left.background, right.background);
		std::swap(left.foreground, right.foreground);
		std::swap(left.inheritForeground, right.inheritForeground);
		std::swap(left.inheritBackground, right.inheritBackground);
	}
}
