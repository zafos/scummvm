/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property _G(of) its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms _G(of) the GNU General Public License as published by
 * the Free Software Foundation, either version 3 _G(of) the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty _G(of)
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy _G(of) the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "got/game/object.h"
#include "got/data/flags.h"
#include "got/events.h"
#include "got/game/back.h"
#include "got/game/move.h"
#include "got/game/status.h"
#include "got/gfx/image.h"
#include "got/vars.h"

namespace Got {

void notEnoughMagic();
void cannotCarryMore();

void showObjects() {
	Common::fill(_G(objectMap), _G(objectMap) + TILES_COUNT, 0);
	Common::fill(_G(objectIndex), _G(objectIndex) + TILES_COUNT, 0);

	for (int i = 0; i < OBJECTS_COUNT; i++) {
		if (_G(scrn)._staticObject[i]) {
			const int p = _G(scrn)._staticX[i] + (_G(scrn)._staticY[i] * TILES_X);
			_G(objectIndex[p]) = i;
			_G(objectMap[p]) = _G(scrn)._staticObject[i];
		}
	}
}

void pickUpObject(int p) {
	switch (_G(objectMap[p])) {
	case 1: // Red jewel
		if (_G(thorInfo)._jewels >= 999) {
			cannotCarryMore();
			return;
		}
		addJewels(10);
		break;
	case 2: // Blue jewel
		if (_G(thorInfo)._jewels >= 999) {
			cannotCarryMore();
			return;
		}
		addJewels(1);
		break;
	case 3: // Red potion
		if (_G(thorInfo)._magic >= 150) {
			cannotCarryMore();
			return;
		}
		addMagic(10);
		break;
	case 4: // Blue potion
		if (_G(thorInfo)._magic >= 150) {
			cannotCarryMore();
			return;
		}
		addMagic(3);
		break;
	case 5: // Good apple
		if (_G(thor)->_health >= 150) {
			cannotCarryMore();
			return;
		}
		playSound(GULP, false);
		addHealth(5);
		break;
	case 6: // Bad apple
		playSound(OW, false);
		addHealth(-10);
		break;
	case 7: // Key (reset on exit)
		addKeys(1);
		break;
	case 8: // Treasure
		if (_G(thorInfo)._jewels >= 999) {
			cannotCarryMore();
			return;
		}
		addJewels(50);
		break;
	case 9: // Trophy
		addScore(100);
		break;
	case 10: // Crown
		addScore(1000);
		break;
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
		if (_G(objectMap[p]) == 13 && HERMIT_HAS_DOLL)
			return;
		_G(thor)->_numMoves = 1;
		_G(hammer)->_numMoves = 2;
		_G(actor[2])._active = false;
		_G(shieldOn) = false;
		_G(tornadoUsed) = false;
		_G(thorInfo)._inventory |= 64;
		_G(thorInfo)._selectedItem = 7;
		_G(thorInfo)._object = _G(objectMap[p]) - 11;
		_G(thorInfo)._objectName = OBJECT_NAMES[_G(thorInfo)._object - 1];
		odinSpeaks((_G(objectMap[p]) - 12) + 501, _G(objectMap[p]) - 1);
		break;
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32: {
		_G(thunderSnakeCounter) = 0;
		_G(shieldOn) = false;
		_G(tornadoUsed) = false;
		_G(hammer)->_numMoves = 2;
		_G(thor)->_numMoves = 1;
		_G(actor[2])._active = false;
		const int s = 1 << (_G(objectMap[p]) - 27);
		_G(thorInfo)._inventory |= s;
		odinSpeaks((_G(objectMap[p]) - 27) + 516, _G(objectMap[p]) - 1);
		_G(thorInfo)._selectedItem = _G(objectMap[p]) - 26;
		addMagic(150);
		fillScore(5);
		}
		break;

	default:
		break;
	}

	playSound(YAH, false);
	_G(objectMap[p]) = 0;

	// Reset so it doesn't reappear on reentry to screen
	if (_G(objectIndex[p]) < 30)
		_G(scrn)._staticObject[_G(objectIndex[p])] = 0;
	
	_G(objectIndex[p]) = 0;
}

void dropRandomObject(Actor *actor) {
	int objId;

	const int rnd1 = g_events->getRandomNumber(99);
	const int rnd2 = g_events->getRandomNumber(99);

	if (rnd1 < 25)
		objId = 5; // Apple
	else if (rnd1 & 1) {
		// Jewels
		if (rnd2 < 10)
			objId = 1; // Red
		else
			objId = 2; // Blue
	} else {
		// Potion
		if (rnd2 < 10)
			objId = 3; // Red
		else
			objId = 4; // Blue
	}

	dropObject(actor, objId);
}

bool dropObject(Actor *actor, const int objId) {
	const int p = (actor->_x + (actor->_sizeX / 2)) / 16 + (((actor->_y + (actor->_sizeY / 2)) / 16) * 20);
	if (!_G(objectMap[p]) && _G(scrn)._iconGrid[p / 20][p % 20] >= 140) { //nothing there and solid
		_G(objectMap[p]) = objId;
		_G(objectIndex[p]) = 27 + actor->_actorNum; //actor is 3-15

		return true;
	}

	return false;
}

bool useApple(int flag) {
	if (_G(thor)->_health == 150)
		return false;

	if (flag && _G(thorInfo)._magic > 0) {
		if (!_G(appleFlag)) {
			_G(magicCounter) = 0;
			addMagic(-2);
			addHealth(1);
			playSound(ANGEL, false);
		} else if (_G(magicCounter) > 8) {
			_G(magicCounter) = 0;
			addMagic(-2);
			addHealth(1);
			if (!soundPlaying())
				playSound(ANGEL, false);
		}
		_G(appleFlag) = true;
		return true;
	}

	_G(appleFlag) = false;
	if (flag)
		notEnoughMagic();

	return false;
}

bool useThunder(int flag) {
	if (flag && _G(thorInfo)._magic > 29) {
		if (!_G(thunderSnakeCounter)) {
			addMagic(-30);
			playSound(THUNDER, false);
			_G(thunderSnakeCounter) = 60;
		}
		return true;
	}

	if (flag && !_G(thunderSnakeCounter))
		notEnoughMagic();

	if (_G(thunderSnakeCounter))
		return true;
	
	return false;
}

bool useBoots(int flag) {
	if (flag) {
		if (_G(thorInfo)._magic > 0) {
			if (_G(thor)->_numMoves == 1 || _G(magicCounter) > 8) {
				_G(magicCounter) = 0;
				addMagic(-1);
			}
			
			_G(thor)->_numMoves = 2;
			_G(hammer)->_numMoves = 3;
			return true;
		}

		notEnoughMagic();
		_G(thor)->_numMoves = 1;
		_G(hammer)->_numMoves = 2;

	} else {
		_G(thor)->_numMoves = 1;
		_G(hammer)->_numMoves = 2;
	}
	return false;
}

bool useShield(int flag) {
	if (flag) {
		if (_G(thorInfo)._magic) {
			if (!_G(shieldOn)) {
				_G(magicCounter) = 0;
				addMagic(-1);
				setupMagicItem(1);

				_G(actor[2]) = _G(magicItem[1]);
				setupActor(&_G(actor[2]), 2, 0, _G(thor)->_x, _G(thor)->_y);
				_G(actor[2])._moveCountdown = 1;
				_G(actor[2])._speed = 1;
				_G(shieldOn) = true;
			} else if (_G(magicCounter) > 8) {
				_G(magicCounter) = 0;
				addMagic(-1);
			}

			return true;
		}

		notEnoughMagic();
	}

	if (_G(shieldOn)) {
		_G(actor[2])._dead = 2;
		_G(actor[2])._active = false;
		_G(shieldOn) = false;
	}

	return false;
}

bool useLightning(int flag) {
	if (flag) {
		if (_G(thorInfo)._magic > 14) {
			addMagic(-15);
			g_events->send("Game", GameMessage("THROW_LIGHTNING"));
		} else {
			notEnoughMagic();
			return false;
		}
	}
	return true;
}

bool useTornado(int flag) {
	if (flag) {
		if (_G(thorInfo)._magic > 10) {
			if (!_G(tornadoUsed) && !_G(actor[2])._dead && _G(magicCounter) > 20) {
				_G(magicCounter) = 0;
				addMagic(-10);
				setupMagicItem(0);
				_G(actor[2]) = _G(magicItem[0]);

				setupActor(&_G(actor[2]), 2, 0, _G(thor)->_x, _G(thor)->_y);
				_G(actor[2])._lastDir = _G(thor)->_dir;
				_G(actor[2])._moveType = 16;
				_G(tornadoUsed) = true;
				playSound(WIND, false);
			}
		} else if (!_G(tornadoUsed)) {
			notEnoughMagic();
			return false;
		}
		if (_G(magicCounter) > 8) {
			if (_G(tornadoUsed)) {
				_G(magicCounter) = 0;
				addMagic(-1);
			}
		}
		if (_G(thorInfo)._magic < 1) {
			actorDestroyed(&_G(actor[2]));
			_G(tornadoUsed) = false;
			notEnoughMagic();
			return false;
		}
		return true;
	}
	return false;
}

bool useObject(int flag) {
	if (!flag || !(_G(thorInfo)._inventory & 64))
		return false;

	odinSpeaks((_G(thorInfo)._object - 1) + 5501, _G(thorInfo)._object - 1);
	return true;
}

void useItem() {
	bool ret = false;

	const int kf = _G(keyFlag[key_magic]);

	if (!kf && _G(tornadoUsed)) {
		actorDestroyed(&_G(actor[2]));
		_G(tornadoUsed) = false;
	}

	const bool mf = _G(magicMissingInform);
	switch (_G(thorInfo)._selectedItem) {
	case 1:
		ret = useApple(kf);
		break;
	case 2:
		ret = useLightning(kf);
		break;
	case 3:
		ret = useBoots(kf);
		break;
	case 4:
		ret = useTornado(kf);
		break;
	case 5:
		ret = useShield(kf);
		break;
	case 6:
		ret = useThunder(kf);
		break;
	case 7:
		ret = useObject(kf);
		break;
	default:
		break;
	}
	
	if (kf) {
		if (!ret && !_G(useItemFlag)) {
			if (mf)
				playSound(BRAAPP, false);
			_G(useItemFlag) = true;
		}
	} else {
		_G(useItemFlag) = false;
	}
}

void notEnoughMagic() {
	if (_G(magicMissingInform))
		return;

	odinSpeaks(2006, 0);
	_G(magicMissingInform) = true;
}

void cannotCarryMore() {
	if (_G(cantCarryInform))
		return;

	odinSpeaks(2007, 0);
	_G(cantCarryInform) = true;
}

void deleteObject() {
	_G(thorInfo)._inventory &= 0xbf;
	_G(thorInfo)._selectedItem = 1;
}

} // namespace Got
