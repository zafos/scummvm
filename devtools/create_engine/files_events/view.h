/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef XYZZY_VIEW_H
#define XYZZY_VIEW_H

#include "xyzzy/events.h"

namespace Xyzzy {

/**
 * Base view class for screens and dialogs that appear on-screen.
 * The View class takes care of two important things:
 * 1) By default events get sent to all controls on a view until one
 * handles it. For mouse events, we instead want only the control the
 * mouse cursor is over to receive the events, saving the individual
 * controls from having to check if the mouse is within their bounds.
 * 2) Individual elements will get a Focus/Unfocus message as the
 * mouse enters and leaves them. This allows, for example, buttons
 * that have been pressed to de-select if the mouse leaves their bounds.
 */
class View : public UIElement {
private:
	UIElement *_focusedElement = nullptr;

	/**
	 * Checks if a control is entered or left
	 */
	void checkFocusedControl(const Common::Point &mousePos);

	/**
	 * Check for an element at the given position
	 */
	UIElement *getElementAtPos(const Common::Point &pos) const;

public:
	View(const Common::String &name, UIElement *uiParent) :
		UIElement(name, uiParent) {
	}
	View(const Common::String &name) :
		UIElement(name) {
	}
	virtual ~View() {
	}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
};

} // namespace Xyzzy

#endif
