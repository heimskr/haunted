#include "core/terminal.h"
#include "ui/colored.h"

namespace haunted::ui {
	ansi::color colored::find_color(ansi::color_type type) const {
		container *p = parent;

		// If the control doesn't need to inherit a color, that would save us the effort of checking its ancestors.
		if (type == ansi::color_type::background && !inherit_background)
			return background;

		if (type == ansi::color_type::foreground && !inherit_foreground)
			return foreground;

		while (p != nullptr) {
			if (colored *pcolored = dynamic_cast<colored *>(p)) {
				// If we find a control that's also an instance of colored, let it determine the color for us.
				ansi::color found = pcolored->find_color(type);
				DBGT("Inheriting " << found << (type == ansi::color_type::foreground? "fore" : "back")
					<< "ground" << ansi::action::reset << " from " << pcolored->get_id());
				return found;
			} else if (control *pcontrol = dynamic_cast<control *>(p)) {
				if (&pcontrol->get_terminal() == pcontrol->get_parent()) {
					// If we've reached the terminal and still haven't found any control with a color preference,
					// give up.
					break;
				}

				// Otherwise, keep going up.
				p = pcontrol->get_parent();
			} else {
				// At this point, because the parent of the previous control isn't null, but it (almost surely) also
				// isn't a terminal and it isn't a control, the parent must surely be a plain container or some unknown
				// subtype. This shouldn't happen. If it does, stop searching.
				DBGT("Unknown container at " << p << "; returning default " <<
					(type == ansi::color_type::foreground? "fore" : "back") << "ground color.");
				break;
			}
		}

		// At this point, the parent is null (shouldn't be possible for anything that's ready to be drawn) or an unknown
		// type of container. This shouldn't happen, but if it does, we'll just return the terminal's default color.
		return ansi::color::normal;
	}

	void colored::draw() {
		if (!can_draw())
			return;
		
		apply_colors();
	}

	colored & colored::apply_colors() {
		if (term != nullptr) {
			DBGTFN();
			term->colors.set_both(find_color(ansi::color_type::foreground), find_color(ansi::color_type::background));
		}

		return *this;
	}

	colored & colored::try_colors(bool find) {
		if (term != nullptr) {
			DBGTFN();
			ansi::color fg = foreground, bg = background;
			if (find) {
				fg = find_color(ansi::color_type::foreground);
				bg = find_color(ansi::color_type::background);
			}

			DBGT("fg = " << ansi::get_name(fg) << ", last_fg = " << ansi::get_name(term->colors.get_foreground())
				<< ", bg = " << ansi::get_name(bg) << ", last_bg = " << ansi::get_name(term->colors.get_background()));

			if (term->colors.set_foreground(fg))
				DBGT("Applying foreground: " << ansi::get_name(fg));
			
			if (term->colors.set_background(bg))
				DBGT("Applying background: " << ansi::get_name(bg));
		}

		return *this;
	}

	colored & colored::uncolor() {
		if (term != nullptr)
			term->reset_colors();
		return *this;
	}

	bool colored::propagate(ansi::color_type type) {
		container *cont = dynamic_cast<container *>(this);
		if (cont == nullptr) {
			DBGT(ansi::color::red << "propagate() failed: not a container");
			return false;
		}

		DBGT("Propagating " << (type == ansi::color_type::foreground? "foreground" :
			(type == ansi::color_type::background? "background" : "both")) << ". Colors = ["
			<< ansi::get_name(foreground) << ", " << ansi::get_name(background) << "]");

		bool is_bg = (static_cast<int>(type) & static_cast<int>(ansi::color_type::background)) != 0;
		bool is_fg = (static_cast<int>(type) & static_cast<int>(ansi::color_type::foreground)) != 0;

		for (control *child: cont->get_children()) {
			colored *cchild = dynamic_cast<colored *>(child);
			if (cchild == nullptr) {
				DBGT("... child isn't colorable.");
				continue;
			}

			bool changed = false;

			if (is_bg && cchild->inherit_background && cchild->get_background() != background) {
				DBGT("Propagating " << ansi::get_name(background) << " background to " << cchild->get_id());
				cchild->set_background(background);
				changed = true;
			}
			
			if (is_fg && cchild->inherit_foreground && cchild->get_foreground() != foreground) {
				DBGT("Propagating " << ansi::get_name(foreground) << " foreground to " << cchild->get_id());
				cchild->set_foreground(foreground);
				changed = true;
			}

			if (changed) {
				DBGT("Propagating " << cchild->get_id() << ".");
				cchild->propagate(type);
			}
		}

		return true;
	}

	bool colored::set_foreground(ansi::color foreground_) {
		if (foreground != foreground_) {
			foreground = foreground_;
			propagate(ansi::color_type::foreground);
			return true;
		}

		return false;
	}

	bool colored::set_background(ansi::color background_) {
		if (background != background_) {
			background = background_;
			propagate(ansi::color_type::background);
			return true;
		}

		return false;
	}

	bool colored::set_colors(ansi::color foreground_, ansi::color background_) {
		bool fg_changed = set_foreground(foreground_);
		bool bg_changed = set_background(background_);

		if (fg_changed || bg_changed) {
			draw();
			return true;
		}

		return false;
	}

	bool colored::set_colors(ansi::color foreground_, ansi::color background_, bool inherit_fg, bool inherit_bg) {
		bool inherit_changed = set_inherit(inherit_fg, inherit_bg);
		bool colors_changed = set_colors(foreground_, background_);
		return inherit_changed || colors_changed;
	}

	bool colored::set_inherit(bool inherit_fg, bool inherit_bg) {
		bool changed = false;

		if (inherit_foreground != inherit_fg) {
			inherit_foreground = inherit_fg;
			changed = true;
			set_foreground(find_color(ansi::color_type::foreground));
		}

		if (inherit_background != inherit_bg) {
			inherit_background = inherit_bg;
			changed = true;
			set_background(find_color(ansi::color_type::background));
		}
		
		return changed;
	}

	void colored::focus() {
		DBGTFN();
		apply_colors();
		control::focus();
	}
}
