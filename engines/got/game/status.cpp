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

#include "got/game/status.h"
#include "got/events.h"
#include "got/vars.h"

namespace Got {

void addJewels(int num) {
	_G(thorInfo)._jewels = CLIP(_G(thorInfo)._jewels + num, 0, 999);
}

void addScore(int num) {
	_G(thorInfo)._score = CLIP(_G(thorInfo)._score + num, 0l, 999999l);
}

void addMagic(int num) {
	_G(thorInfo)._magic = CLIP(_G(thorInfo)._magic + num, 0, 150);
}

void addHealth(int num) {
	_G(thor)->_health = CLIP(_G(thor)->_health + num, 0, 150);

	if (_G(thor)->_health < 1)
		g_events->send(GameMessage("THOR_DIES"));
}

void addKeys(int num) {
	_G(thorInfo)._keys = CLIP(_G(thorInfo)._keys + num, 0, 99);
}

void fillHealth() {
	addHealth(150);
}

void fillMagic() {
	addMagic(150);
}

void fillScore(int num, const char *endMessage) {
	GameMessage msg("FILL_SCORE");
	if (endMessage)
		msg._stringValue = endMessage;
	msg._value = num;
	g_events->send("GameStatus", msg);
}

} // namespace Got
