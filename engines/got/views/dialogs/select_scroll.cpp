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

#include "got/views/dialogs/select_scroll.h"
#include "got/got.h"

namespace Got {
namespace Views {
namespace Dialogs {

SelectScroll::SelectScroll() : SelectOption("SelectScroll", "Scroll Between Screens?", YES_NO) {
}

bool SelectScroll::msgFocus(const FocusMessage &msg) {
	_selectedItem = 1 - (_G(setup)._scrollFlag ? 1 : 0);
	return true;
}

void SelectScroll::selected() {
	_G(setup)._scrollFlag = (_selectedItem == 0);
	_G(lastSetup) = _G(setup);
}

} // namespace Dialogs
} // namespace Views
} // namespace Got
