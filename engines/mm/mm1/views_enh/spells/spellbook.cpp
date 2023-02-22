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

#include "mm/mm1/views_enh/spells/spellbook.h"
#include "mm/mm1/game/spells_party.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Spells {

Spellbook::Spellbook() : PartyView("Spellbook") {
	_bounds = Common::Rect(27, 6, 208, 142);
	addButtons();
}

void Spellbook::addButtons() {
	_scrollSprites.load("scroll.icn");
	addButton(&g_globals->_mainIcons, Common::Point(187, 26), 0, Common::KEYCODE_UP);
	addButton(&g_globals->_mainIcons, Common::Point(187, 111), 2, Common::KEYCODE_DOWN);
	addButton(&_scrollSprites, Common::Point(100, 109), 5, KEYBIND_SELECT);

	addButton(Common::Rect(40, 28, 187, 36), Common::KEYCODE_1);
	addButton(Common::Rect(40, 37, 187, 45), Common::KEYCODE_2);
	addButton(Common::Rect(40, 46, 187, 54), Common::KEYCODE_3);
	addButton(Common::Rect(40, 55, 187, 63), Common::KEYCODE_4);
	addButton(Common::Rect(40, 64, 187, 72), Common::KEYCODE_5);
	addButton(Common::Rect(40, 73, 187, 81), Common::KEYCODE_6);
	addButton(Common::Rect(40, 82, 187, 90), Common::KEYCODE_7);
	addButton(Common::Rect(40, 91, 187, 99), Common::KEYCODE_8);
	addButton(Common::Rect(40, 100, 187, 108), Common::KEYCODE_9);
	addButton(Common::Rect(40, 109, 187, 117), Common::KEYCODE_0);
	addButton(Common::Rect(174, 123, 198, 133), KEYBIND_ESCAPE);
	addButton(Common::Rect(187, 35, 198, 73), Common::KEYCODE_PAGEUP);
	addButton(Common::Rect(187, 74, 198, 112), Common::KEYCODE_PAGEDOWN);
	addButton(Common::Rect(132, 123, 168, 133), KEYBIND_SELECT);
}

bool Spellbook::msgFocus(const FocusMessage &msg) {
	PartyView::msgFocus(msg);

	// In this view we don't want 1 to 6 mapping to char selection
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	updateChar();
	return true;
}

bool Spellbook::canSwitchChar() const {
	return !g_events->isInCombat();
}

void Spellbook::draw() {
	PartyView::draw();

	Graphics::ManagedSurface s = getSurface();
	const Character &c = *g_globals->_currCharacter;

	// Draw the scrolling area frame
	_scrollSprites.draw(&s, 4, Common::Point(14, 20));
	_scrollSprites.draw(&s, 0, Common::Point(162, 20));
	_scrollSprites.draw(&s, 2, Common::Point(162, 105));

	// Title line
	_fontReduced = true;
	Common::String title = Common::String::format("%s %s",
		STRING["enhdialogs.spellbook.title"].c_str(),
		c._name
	);
	writeString(0, 0, title, ALIGN_MIDDLE);

	// Write current spell points
	Common::String sp = Common::String::format("%s - %d",
		STRING["enhdialogs.spellbook.spell_points"].c_str(), c._sp._current);
	writeString(7, 111, sp);

	// Iterate over the lines
	for (int i = 0; i < 10; ++i) {
		// Left gutter row number
		setTextColor(0);
		const int yp = 15 + 9 * i;
		writeString(0, yp, Common::String::format("%c", (i == 9) ? '0' : '1' + i));

		const int spellIndex = _topIndex + i;
		setTextColor((spellIndex == _selectedIndex) ? 15 : 37);

		if (_count == 0) {
			if (i == 0)
				writeString(12, yp, STRING["enhdialogs.spellbook.non_caster"]);

		} else if (spellIndex < _count) {
			// Spell name
			Common::String spellName = STRING[Common::String::format(
				"spells.%s.%d",
				_isWizard ? "wizard" : "cleric",
				spellIndex
			)];
			writeString(12, yp, spellName);

			// Spell requirements
			int lvl, num;
			getSpellLevelNum(CATEGORY_SPELLS_COUNT * (_isWizard ? 1 : 0) + spellIndex, lvl, num);
			setSpell(g_globals->_currCharacter, lvl, num);

			writeString(152, yp, Common::String::format("%d/%d",
				_requiredSp, _requiredGems), ALIGN_RIGHT);
		}
	}
}

bool Spellbook::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode >= Common::KEYCODE_0 && msg.keycode <= Common::KEYCODE_9) {
		int newIndex = _topIndex + (msg.keycode == Common::KEYCODE_0 ?
			9 : msg.keycode - Common::KEYCODE_1);
		if (newIndex < _count) {
			_selectedIndex = newIndex;
			redraw();
		}
	} else if (msg.keycode == Common::KEYCODE_PAGEUP) {
		if (_topIndex > 0) {
			_topIndex = MAX(_topIndex - 10, 0);
			redraw();
		}
	} else if (msg.keycode == Common::KEYCODE_PAGEDOWN) {
		int newTopIndex = _topIndex + 10;
		if (newTopIndex < _count) {
			_topIndex = newTopIndex;
			redraw();
		}
	} else if (msg.keycode == Common::KEYCODE_UP) {
		if (_topIndex > 0) {
			--_topIndex;
			redraw();
		}
	} else if (msg.keycode == Common::KEYCODE_DOWN) {
		if ((_topIndex + 10) < _count) {
			++_topIndex;
			redraw();
		}
	} else if (msg.keycode == Common::KEYCODE_s) {
		// Alternate alias for Select button
		msgAction(ActionMessage(KEYBIND_SELECT));

	} else {
		return PartyView::msgKeypress(msg);
	}

	return true;
}

bool Spellbook::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		close();
		return true;

	case KEYBIND_SELECT:
		close();
		spellSelected();
		return true;

	default:
		break;
	}

	return false;
}

bool Spellbook::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE") {
		updateChar();
		return true;
	} else {
		return PartyView::msgGame(msg);
	}
}

void Spellbook::updateChar() {
	// Refresh the cast spell side dialog for new character
	send("CastSpell", GameMessage("UPDATE"));

	// Update the highlighted char in the party display
	g_events->send(GameMessage("CHAR_HIGHLIGHT", (int)true));

	// Update fields
	const Character &c = *g_globals->_currCharacter;
	_isWizard = c._class == SORCERER || c._class == ARCHER;

	_selectedIndex = (g_events->isInCombat() ? c._combatSpell : c._nonCombatSpell) % CATEGORY_SPELLS_COUNT;
	if (_selectedIndex == -1)
		_selectedIndex = 0;
	_topIndex = (_selectedIndex / 10) * 10;

	if (c._spellLevel._current == 0) {
		_count = 0;
	} else {
		_count = (c._spellLevel._current < 5) ?
			c._spellLevel * 8 - 1 : 31 + (c._spellLevel - 4) * 5;
	}

	// And finally, update the display
	redraw();
}

void Spellbook::spellSelected() {
	Character &c = *g_globals->_currCharacter;
	int spellIndex = (_isWizard ? CATEGORY_SPELLS_COUNT : 0) + _selectedIndex;

	// Set the selected spell for the character
	if (g_events->isInCombat())
		c._combatSpell = spellIndex;
	else
		c._nonCombatSpell = spellIndex;

	// Update the cast spell dialog with the new spell
	send("CastSpell", GameMessage("UPDATE"));
}

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
