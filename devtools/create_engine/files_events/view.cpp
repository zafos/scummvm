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

#include "xyzzy/view.h"

namespace Xyzzy {

void View::checkFocusedControl(const Common::Point &mousePos) {
	if (_focusedElement) {
		if (!_focusedElement->getBounds().contains(mousePos)) {
			_focusedElement->send(MouseLeaveMessage());
			_focusedElement = nullptr;
		}

	} else {
		for (UIElement *child : _children) {
			if (child->getBounds().contains(mousePos)) {
				_focusedElement = child;
				child->send(MouseEnterMessage());
				break;
			}
		}
	}
}

UIElement *View::getElementAtPos(const Common::Point &pos) const {
	for (UIElement *child : _children) {
		if (child->getBounds().contains(pos))
			return child;
	}

	return nullptr;
}


bool View::msgFocus(const FocusMessage &msg) {
	_focusedElement = nullptr;
	return UIElement::msgFocus(msg);
}

bool View::msgUnfocus(const UnfocusMessage &msg) {
	if (_focusedElement)
		_focusedElement->send(MouseLeaveMessage());

	return UIElement::msgUnfocus(msg);
}

bool View::msgMouseMove(const MouseMoveMessage &msg) {
	checkFocusedControl(msg._pos);
	return true;
}

bool View::msgMouseDown(const MouseDownMessage &msg) {
	UIElement *child = getElementAtPos(msg._pos);
	return child ? child->send(msg) : false;
}

bool View::msgMouseUp(const MouseUpMessage &msg) {
	UIElement *child = getElementAtPos(msg._pos);
	return child ? child->send(msg) : false;
}

} // namespace Xyzzy
