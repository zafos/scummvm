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

#include "got/views/game_content.h"
#include "got/game/back.h"
#include "got/game/boss1.h"
#include "got/game/boss2.h"
#include "got/game/boss3.h"
#include "got/game/move.h"
#include "got/game/move_patterns.h"
#include "got/game/object.h"
#include "got/game/status.h"
#include "got/vars.h"

namespace Got {
namespace Views {

#define SPIN_INTERVAL 4
#define SPIN_COUNT 20
#define DEATH_THRESHOLD (SPIN_COUNT * SPIN_INTERVAL)

GameContent::GameContent() : View("GameContent") {
	_surface.create(320, 192);
}

void GameContent::draw() {
	GfxSurface s;
	if (_G(gameMode) == MODE_THUNDER || _G(gameMode) == MODE_AREA_CHANGE) {
		s.create(320, 192);
	} else {
		s = getSurface();
	}
	s.clear();

	drawBackground(s);
	drawObjects(s);
	drawActors(s);

	if ((GAME1 && _G(currentLevel) == BOSS_LEVEL1) ||
		(GAME2 && _G(currentLevel) == BOSS_LEVEL2) ||
		(GAME3 && _G(currentLevel) == BOSS_LEVEL3))
		drawBossHealth(s);

	// If we're shaking the screen, render the content with the shake X/Y
	if (_G(gameMode) == MODE_THUNDER) {
		GfxSurface win = getSurface();
		win.clear();
		win.simpleBlitFrom(s, _moveDelta);
	} else if (_G(gameMode) == MODE_LIGHTNING) {
		drawLightning(s);

	} else if (_G(gameMode) == MODE_AREA_CHANGE) {
		// Draw parts of the new scene along with parts of the old one
		// as it's scrolled off-screen
		GfxSurface win = getSurface();

		switch (_G(transitionDir)) {
		case DIR_LEFT:
			win.simpleBlitFrom(s, Common::Rect(320 - _transitionPos, 0, 320, 192), Common::Point(0, 0));
			win.simpleBlitFrom(_surface, Common::Rect(0, 0, 320 - _transitionPos, 192), Common::Point(_transitionPos, 0));
			break;
		case DIR_RIGHT:
			win.simpleBlitFrom(_surface, Common::Rect(_transitionPos, 0, 320, 192), Common::Point(0, 0));
			win.simpleBlitFrom(s, Common::Rect(0, 0, _transitionPos, 192), Common::Point(320 - _transitionPos, 0));
			break;
		case DIR_UP:
			win.simpleBlitFrom(s, Common::Rect(0, 192 - _transitionPos, 320, 192), Common::Point(0, 0));
			win.simpleBlitFrom(_surface, Common::Rect(0, 0, 320, 192 - _transitionPos), Common::Point(0, _transitionPos));
			break;
		case DIR_DOWN:
			win.simpleBlitFrom(_surface, Common::Rect(0, _transitionPos, 320, 192), Common::Point(0, 0));
			win.simpleBlitFrom(s, Common::Rect(0, 0, 320, _transitionPos), Common::Point(0, 192 - _transitionPos));
			break;
		case DIR_PHASED:
			win.simpleBlitFrom(_surface); // Copy old surface

			// Copy the randomly chosen blocks over from new scene
			for (int i = 0; i < 240; ++i) {
				int x = (i * 16) % 320;
				int y = ((i * 16) / 320) * 16;
				if (_phased[i])
					win.simpleBlitFrom(s, Common::Rect(x, y, x + 16, y + 16), Common::Point(x, y));
			}
			break;
		default:
			break;
		}
	}
}

#define MSG(STR, METHOD)         \
	else if (msg._name == STR) { \
		METHOD();                \
		return true;             \
	}

bool GameContent::msgGame(const GameMessage &msg) {
	if (msg._name == "PAUSE") {
		_G(gameMode) = MODE_PAUSE;
		_pauseCtr = msg._value;
		return true;
	}

	if (msg._name == "SCORE_INV") {
		_G(gameMode) = MODE_SCORE_INV;
		_pauseCtr = 0;
		return true;
	}
	MSG("THROW_LIGHTNING", throwLightning)
	MSG("THOR_DIES", thorDies)
	MSG("CLOSING", closingSequence)

	return false;
}

#undef MSG

bool GameContent::tick() {
	checkThunderShake();

	switch (_G(gameMode)) {
	case MODE_NORMAL:
	case MODE_THUNDER:
		checkSwitchFlag();
		checkForItem();
		moveActors();
		useItem();
		updateActors();
		checkForBossDead();
		checkForCheats();

		if (_G(endGame))
			endGameMovement();
		break;

	case MODE_THOR_DIES:
		if (_deathCtr < DEATH_THRESHOLD) {
			spinThor();
		} else if (_deathCtr < DEATH_THRESHOLD + 60) {
			_G(thor)->_active = false;
			++_deathCtr;
		} else {
			thorDead();
		}
		break;

	case MODE_LIGHTNING:
		if (--_lightningCtr == 0) {
			lightningCountdownDone();
		}
		break;

	case MODE_PAUSE:
		if (--_pauseCtr == 0)
			_G(gameMode) = MODE_NORMAL;
		break;

	case MODE_SCORE_INV:
		if (--_pauseCtr <= 0) {
			_pauseCtr = 2;

			if (_G(thor)->_health > 0) {
				_G(thor)->_health--;
				playSound(WOOP, true);
				addHealth(-1);
				addScore(10);

			} else if (_G(thorInfo)._magic > 0) {
				_G(thorInfo)._magic--;
				playSound(WOOP, true);
				addMagic(-1);
				addScore(10);

			} else if (_G(thorInfo)._jewels) {
				_G(thorInfo)._jewels--;
				playSound(WOOP, true);
				addJewels(-1);
				addScore(10);

			} else {
				_G(gameMode) = MODE_NORMAL;
				_pauseCtr = 0;
				send(GameMessage("CLOSING"));
			}
		}
		break;

	default:
		break;
	}

	if (_G(eyeballs) == 1) { // eyeballs movement animation
		if (!_G(setup).f25) {
			_G(thor)->_dir = 0;
		} else {
			_G(thor)->_dir = 1;
		}
	}

	checkForAreaChange();

	// Check for end of game area
	if (_G(endTile)) {
		_G(endTile) = false;
		Gfx::fadeOut();

		// Add name to high scores list if necessary, and then show it
		_G(highScores).add(_G(area), _G(playerName), _G(thorInfo)._score);
		g_events->send("HighScores", GameMessage("HIGH_SCORES", _G(area)));
	}

	return false;
}

void GameContent::drawBackground(GfxSurface &s) {
	const Level &screen = _G(scrn);

	for (int y = 0; y < TILES_Y; y++) {
		for (int x = 0; x < TILES_X; x++) {
			if (screen._iconGrid[y][x] != 0) {
				const Common::Point pt(x * TILE_SIZE, y * TILE_SIZE);
				s.simpleBlitFrom(_G(bgPics[screen._backgroundColor]), pt);
				s.simpleBlitFrom(_G(bgPics[screen._iconGrid[y][x]]), pt);
			}
		}
	}
}

void GameContent::drawObjects(GfxSurface &s) {
	for (int y = 0; y < TILES_Y; ++y) {
		for (int x = 0; x < TILES_X; ++x) {
			int p = (y * TILES_X) + x;

			byte currObjId = _G(objectMap[p]);
			if (currObjId) {
				s.simpleBlitFrom(_G(objects[currObjId - 1]), Common::Point(x * TILE_SIZE, y * TILE_SIZE));
			}
		}
	}
}

void GameContent::drawActors(GfxSurface &s) {
	Actor *actor_ptr = &_G(actor[MAX_ACTORS - 1]);
	Actor *actor2_storage = nullptr;

	for (int actor_num = 0; actor_num <= MAX_ACTORS;) {
		// Check for blinking flag
		if (actor_ptr && actor_ptr->_active && !(actor_ptr->_show & 2)) {
			actor_ptr->_lastX[_G(pge)] = actor_ptr->_x;
			actor_ptr->_lastY[_G(pge)] = actor_ptr->_y;

			const Graphics::ManagedSurface &frame = actor_ptr->pic[actor_ptr->_dir][actor_ptr->_frameSequence[actor_ptr->_nextFrame]];
			s.simpleBlitFrom(frame, Common::Point(actor_ptr->_x, actor_ptr->_y));
		}

		// Move to the next actor
		do {
			--actor_ptr;
			++actor_num;

			if (actor_num == MAX_ACTORS)
				actor_ptr = actor2_storage;
			else if (actor_num == (MAX_ACTORS - 3))
				actor2_storage = actor_ptr;
		} while (actor_num == (MAX_ACTORS - 3));
	}

	if (_G(gameMode) == MODE_THOR_DIES && _deathCtr >= DEATH_THRESHOLD)
		s.simpleBlitFrom(_G(objects[10]), Common::Point(_G(thor)->_x, _G(thor)->_y));
}

void GameContent::drawBossHealth(GfxSurface &s) {
	int c;

	int health = _G(actor[3])._health;

	s.fillRect(Common::Rect(304, 2, 317, 81), 0);
	s.fillRect(Common::Rect(305, 3, 316, 80), 28);
	s.fillRect(Common::Rect(306, 4, 315, 79), 26);
	s.fillRect(Common::Rect(307, 5, 314, 78), 24);

	for (int i = 10; i > 0; i--) {
		if (i * 10 > health)
			c = 0;
		else
			c = 32;

		s.fillRect(Common::Rect(308, 7 + (7 * (10 - i)), 313, 13 + (7 * (10 - i))), c);
	}
}

void GameContent::checkThunderShake() {
	if (_G(thunderSnakeCounter)) {
		_G(gameMode) = MODE_THUNDER;

		// Introduce a random screen shake by rendering screen 1 pixel offset randomly
		static const int8 DELTA_X[4] = {-1, 1, 0, 0};
		static const int8 DELTA_Y[4] = {0, 0, -1, 1};
		int delta = g_events->getRandomNumber(3);

		_moveDelta.x = DELTA_X[delta];
		_moveDelta.y = DELTA_Y[delta];

		_G(thunderSnakeCounter--);
		if ((_G(thunderSnakeCounter) < MAX_ACTORS) && _G(thunderSnakeCounter) > 2) {
			int thunderFl = _G(thunderSnakeCounter);
			if (_G(actor[thunderFl])._active) {
				_G(actor[thunderFl])._vulnerableCountdown = 0;
				actorDamaged(&_G(actor[thunderFl]), 20);
			}
		}

		if (!_G(thunderSnakeCounter)) {
			_G(gameMode) = MODE_NORMAL;
			_moveDelta = Common::Point(0, 0);
		}

		redraw();
	}
}

void GameContent::checkSwitchFlag() {
	if (!_G(switchUsed))
		return;
	
	switch (_G(switchUsed)) {
	case 1:
		switchIcons();
		break;
	case 2:
		rotateArrows();
		break;
	default:
		break;
	}

	_G(switchUsed) = 0;
}

void GameContent::checkForItem() {
	int thor_pos = _G(thor)->getPos();
	if (_G(objectMap[thor_pos]))
		pickUpObject(thor_pos);
}

void GameContent::moveActors() {
	for (int i = 0; i < MAX_ACTORS; i++) {
		if (_G(actor[i])._active) {
			_G(actor[i])._moveCount = _G(actor[i])._numMoves;
			while (_G(actor[i])._moveCount--)
				moveActor(&_G(actor[i]));

			if (i == 0)
				setThorVars();

			if (_G(newLevel) != _G(currentLevel))
				return;
		}
	}

	int thor_pos = _G(thor)->getPos();
	_G(thor)->_centerX = thor_pos % 20;
	_G(thor)->_centerY = thor_pos / 20;
}

void GameContent::updateActors() {
	for (int i = 0; i < MAX_ACTORS; ++i) {
		Actor *actor = &_G(actor[i]);

		if (!actor->_active && actor->_dead > 0)
			actor->_dead--;
	}
}

void GameContent::checkForBossDead() {
	if (_G(bossDead)) {
		int loop;
		for (loop = 3; loop < 7; loop++) {
			if (_G(actor[loop])._active)
				break;
		}

		if (loop == 7) {
			_G(bossDead) = false;

			_G(exitFlag) = 0;

			if (_G(bossActive)) {
				switch (_G(area)) {
				case 1:
					boss1ClosingSequence1();
					break;
				case 2:
					boss2ClosingSequence1();
					break;
				case 3:
					boss3ClosingSequence1();
					break;
				default:
					break;
				}

				_G(bossActive) = false;
			}
		}
	}
}

void GameContent::checkForAreaChange() {
	if (_G(gameMode) == MODE_AREA_CHANGE) {
		// Area transition is already in progress
		switch (_G(transitionDir)) {
		case DIR_LEFT:
		case DIR_RIGHT:
			_transitionPos += 32;
			if (_transitionPos == 320)
				_G(gameMode) = MODE_NORMAL;
			break;
		case DIR_UP:
		case DIR_DOWN:
			_transitionPos += 16;
			if (_transitionPos == 192)
				_G(gameMode) = MODE_NORMAL;
			break;
		case DIR_PHASED:
			_transitionPos += 10;
			if (_transitionPos == 240) {
				_G(gameMode) = MODE_NORMAL;
				Common::fill(_phased, _phased + 240, false);
			} else {
				// The screen is subdivided into 240 16x16 blocks. Picks ones
				// randomly to copy over from the new screen
				for (int i = 0; i < 10; ++i) {
					for (;;) {
						int idx = g_events->getRandomNumber(239);
						if (!_phased[idx]) {
							_phased[idx] = true;
							break;
						}
					}
				}
			}
			break;

		default:
			break;
		}

		if (_G(gameMode) == MODE_NORMAL) {
			_transitionPos = 0;
			showLevelDone();
		}

	} else if (_G(newLevel) != _G(currentLevel)) {
		// Area transition beginning
		_G(thor)->_show = 0;
		_G(thor)->_active = false;
		_G(hammer)->_active = false;
		_G(tornadoUsed) = false;

		// Draws the old area without Thor, and then save a copy of it.
		// This will be used to scroll old area off-screen as new area scrolls in
		draw();
		_surface.copyFrom(getSurface());

		// Set up new level
		_G(thor)->_active = true;
		showLevel(_G(newLevel));
	}
}

void GameContent::thorDies() {
	if (_G(gameMode) == MODE_SCORE_INV)
		return;

	// Stop any actors on-screen from moving
	for (int li = 0; li < MAX_ACTORS; li++)
		_G(actor[li])._show = 0;
	_G(actor[2])._active = false;

	// Set the state for showing death animation
	_G(gameMode) = MODE_THOR_DIES;
	_deathCtr = 0;
	_G(shieldOn) = false;

	playSound(DEAD, true);
}

void GameContent::spinThor() {
	static const byte DIRS[] = {0, 2, 1, 3};

	if (!_G(eyeballs)) {
		_G(thor)->_dir = DIRS[(_deathCtr / SPIN_INTERVAL) % 4];
		_G(thor)->_lastDir = DIRS[(_deathCtr / SPIN_INTERVAL) % 4];
	}

	++_deathCtr;
}

void GameContent::thorDead() {
	int li = _G(thorInfo)._selectedItem;
	int ln = _G(thorInfo)._inventory;

	_G(newLevel) = _G(thorInfo)._lastScreen;
	_G(thor)->_x = (_G(thorInfo)._lastIcon % 20) * 16;
	_G(thor)->_y = ((_G(thorInfo)._lastIcon / 20) * 16) - 1;
	if (_G(thor)->_x < 1)
		_G(thor)->_x = 1;
	if (_G(thor)->_y < 0)
		_G(thor)->_y = 0;
	_G(thor)->_lastX[0] = _G(thor)->_x;
	_G(thor)->_lastX[1] = _G(thor)->_x;
	_G(thor)->_lastY[0] = _G(thor)->_y;
	_G(thor)->_lastY[1] = _G(thor)->_y;
	_G(thor)->_dir = _G(thorInfo)._lastDir;
	_G(thor)->_lastDir = _G(thorInfo)._lastDir;
	_G(thor)->_health = _G(thorInfo)._lastHealth;
	_G(thorInfo)._magic = _G(thorInfo)._lastMagic;
	_G(thorInfo)._jewels = _G(thorInfo)._lastJewels;
	_G(thorInfo)._keys = _G(thorInfo)._lastKeys;
	_G(thorInfo)._score = _G(thorInfo)._lastScore;
	_G(thorInfo)._object = _G(thorInfo)._lastObject;
	_G(thorInfo)._objectName = _G(thorInfo)._lastObjectName;

	if (ln == _G(thorInfo)._lastInventory) {
		_G(thorInfo)._selectedItem = li;
	} else {
		_G(thorInfo)._selectedItem = _G(thorInfo)._lastItem;
		_G(thorInfo)._inventory = _G(thorInfo)._lastInventory;
	}

	_G(setup) = _G(lastSetup);

	_G(thor)->_numMoves = 1;
	_G(thor)->_vulnerableCountdown = 60;
	_G(thor)->_show = 60;
	_G(appleFlag) = false;
	_G(thunderSnakeCounter) = 0;
	_G(tornadoUsed) = false;
	_G(shieldOn) = false;
	musicResume();
	_G(actor[1])._active = false;
	_G(actor[2])._active = false;
	_G(thor)->_moveCountdown = 6;
	_G(thor)->_active = true;

	// Load saved data for new level back into scrn
	_G(scrn).load(_G(newLevel));

	_G(gameMode) = MODE_NORMAL;
	_deathCtr = 0;

	showLevel(_G(newLevel));
	setThorVars();
}

void GameContent::checkForCheats() {
	if (_G(cheats)._freezeHealth)
		_G(thor)->_health = 150;
	if (_G(cheats)._freezeMagic)
		_G(thorInfo)._magic = 150;
	if (_G(cheats)._freezeJewels)
		_G(thorInfo)._jewels = 999;
}

void GameContent::placePixel(GfxSurface &s, int dir, int num) {
	switch (dir) {
	case 0:
		_pixelY[dir][num] = _pixelY[dir][num - 1] - 1;
		_pixelX[dir][num] = _pixelX[dir][num - 1] +
							(1 - (g_events->getRandomNumber(2)));
		break;
	case 1:
		if (g_events->getRandomNumber(1)) {
			_pixelX[dir][num] = _pixelX[dir][num - 1] + 1;
			_pixelY[dir][num] = _pixelY[dir][num - 1] + (0 - (g_events->getRandomNumber(1)));
		} else {
			_pixelY[dir][num] = _pixelY[dir][num - 1] - 1;
			_pixelX[dir][num] = _pixelX[dir][num - 1] + (1 - (g_events->getRandomNumber(1)));
		}
		break;
	case 2:
		_pixelX[dir][num] = _pixelX[dir][num - 1] + 1;
		_pixelY[dir][num] = _pixelY[dir][num - 1] + (1 - (g_events->getRandomNumber(2)));
		break;
	case 3:
		if (g_events->getRandomNumber(1)) {
			_pixelX[dir][num] = _pixelX[dir][num - 1] + 1;
			_pixelY[dir][num] = _pixelY[dir][num - 1] + (1 - (g_events->getRandomNumber(1)));
		} else {
			_pixelY[dir][num] = _pixelY[dir][num - 1] + 1;
			_pixelX[dir][num] = _pixelX[dir][num - 1] + (1 - (g_events->getRandomNumber(1)));
		}
		break;
	case 4:
		_pixelY[dir][num] = _pixelY[dir][num - 1] + 1;
		_pixelX[dir][num] = _pixelX[dir][num - 1] + (1 - (g_events->getRandomNumber(2)));
		break;
	case 5:
		if (g_events->getRandomNumber(1)) {
			_pixelX[dir][num] = _pixelX[dir][num - 1] - 1;
			_pixelY[dir][num] = _pixelY[dir][num - 1] + (1 - (g_events->getRandomNumber(1)));
		} else {
			_pixelY[dir][num] = _pixelY[dir][num - 1] + 1;
			_pixelX[dir][num] = _pixelX[dir][num - 1] + (0 - (g_events->getRandomNumber(1)));
		}
		break;
	case 6:
		_pixelX[dir][num] = _pixelX[dir][num - 1] - 1;
		_pixelY[dir][num] = _pixelY[dir][num - 1] + (1 - (g_events->getRandomNumber(2)));
		break;
	case 7:
		if (g_events->getRandomNumber(1)) {
			_pixelX[dir][num] = _pixelX[dir][num - 1] - 1;
			_pixelY[dir][num] = _pixelY[dir][num - 1] + (0 - (g_events->getRandomNumber(1)));
		} else {
			_pixelY[dir][num] = _pixelY[dir][num - 1] - 1;
			_pixelX[dir][num] = _pixelX[dir][num - 1] + (0 - (g_events->getRandomNumber(1)));
		}
		break;
	default:
		return;
	}

	if (pointWithin(_pixelX[dir][num], _pixelY[dir][num], 0, 0, 319, 191)) {
		byte *pixel = (byte *)s.getBasePtr(_pixelX[dir][num],
										   _pixelY[dir][num]);
		*pixel = _pixelC[dir];
	}
}

void GameContent::throwLightning() {
	_G(gameMode) = MODE_LIGHTNING;
	_lightningCtr = 20;

	for (int i = 0; i < MAX_ACTORS; i++)
		_G(actor[i])._show = 0;

	playSound(ELECTRIC, true);
}

void GameContent::drawLightning(GfxSurface &s) {
	for (int i = 0; i < 8; i++) {
		_pixelX[i][0] = _G(thor)->_x + 7;
		_pixelY[i][0] = _G(thor)->_y + 7;
		_pixelC[i] = 14 + g_events->getRandomNumber(1);
	}

	for (int r = 0; r < 8; r++) {
		for (int i = 1; i < 25; i++) {
			placePixel(s, r, i);
		}
	}
}

void GameContent::lightningCountdownDone() {
	_G(gameMode) = MODE_NORMAL;

	int x = _G(thor)->_x + 7;
	int y = _G(thor)->_y + 7;

	for (int i = 3; i < MAX_ACTORS; i++) {
		if (!_G(actor[i])._active)
			continue;

		int ax = _G(actor[i])._x + (_G(actor[i])._sizeX / 2);
		int ay = _G(actor[i])._y + (_G(actor[i])._sizeY / 2);

		if ((ABS(ax - x) < 30) && (ABS(ay - y) < 30)) {
			_G(actor[i])._magicHit = 1;
			_G(actor[i])._vulnerableCountdown = 0;
			actorDamaged(&_G(actor[i]), 254);
		}
	}
}

void GameContent::closingSequence() {
	const int area = _G(area);

	switch (++_closingStateCtr) {
	case 1:
		// Convert health/magic/jewels to score
		_G(gameMode) = MODE_SCORE_INV;
		break;

	case 2:
		switch (area) {
		case 1:
			boss1ClosingSequence2();
			break;
		case 2:
			boss2ClosingSequence2();
			break;
		case 3:
			boss3ClosingSequence2();
			break;
		default:
			break;
		}
		break;

	case 3:
		switch (area) {
		case 1:
			boss1ClosingSequence3();
			break;
		case 2:
			boss2ClosingSequence3();
			break;
		case 3:
			boss3ClosingSequence3();
			break;
		default:
			break;
		}
		break;

	case 4:
		_closingStateCtr = 0;

		switch (area) {
		case 1:
			boss1ClosingSequence4();
			break;
		case 2:
			boss2ClosingSequence4();
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}

} // namespace Views
} // namespace Got
