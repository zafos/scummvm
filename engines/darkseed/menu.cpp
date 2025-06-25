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

#include "common/keyboard.h"
#include "common/config-manager.h"
#include "darkseed/menu.h"
#include "darkseed/darkseed.h"
#include "darkseed/langtext.h"

namespace Darkseed {

constexpr I18NTextWithPosition kMenu_load = {
	{127, 62, "Load"},
	{119, 62, "CARGAR"},
	{118, 62, "CHARGER"},
	{121, 62, "LADEN"},
	{120, 62, "\xA6\x89\x9C\xE1\x90\x91"},
	{118, 55, "\xa8\xfa\x20\xc0\xc9"}
};

constexpr I18NTextWithPosition kMenu_silent = {
	{123, 136, "Silent"},
	{115, 136, "SIN SON"},
	{118, 136, "SILENCE"},
	{129, 136, "RUHE"},
	{121, 136, "\xB7q\xAD\xA1\x88\xE1"},
	{118, 129, "\xc0\x52\x20\xad\xb5"}

};

constexpr I18NTextWithPosition kMenu_sounds = {
	{117, 136, "Sounds"},
	{121, 136, "SONIDO"},
	{126, 136, "SONS"},
	{123, 136, "SOUND"},
	{128, 136, "\xB7q\xD0\xB7"},
{118, 129, "\xad\xb5\x20\xae\xc4"}
};

constexpr I18NTextWithPosition kMenu_save = {
	{127, 99, "Save"},
	{119, 99, "GRABAR"},
	{121, 99, "SAUVER"},
	{115, 99, "SICHERN"},
	{128, 99, "\xB8\xE1\xB8w"},
	{118, 92, "\xa6\x73\x20\xc0\xc9"}
};

constexpr I18NTextWithPosition kMenu_resume = {
	{118, 173, "Resume"},
	{122, 173, "SEQUIR"},
	{124, 173, "JOUER"},
	{118, 173, "WEITER"},
	{128, 173, "\xA5\xA2\x8A\xE1"},
	{118, 166, "\xc4\x7e\x20\xc4\xf2"}
};

constexpr I18NTextWithPosition kMenu_quit = {
	{129, 210, "Quit"},
	{125, 210, "SALIR"},
	{117, 210, "QUITTER"},
	{129, 210, "ENDE"},
	{119, 210, "\x8F{\xA0""a\xC3\xB1"},
	{118, 203, "\xb5\xb2\x20\xa7\xf4"}
};

void Menu::drawMenuItem(const I18NTextWithPosition &menuText) {
	const TextWithPosition &textWithPosition = getI18NTextWithPosition(menuText);
	if (_zhFont) {
		_zhFont->drawString(g_engine->_screen, convertToU32String(textWithPosition.text, g_engine->getLanguage()), textWithPosition.x, textWithPosition.y, 640, 14);
	} else {
		g_engine->_console->drawStringAt(textWithPosition.x, textWithPosition.y, convertToU32String(textWithPosition.text, g_engine->getLanguage()));
	}
}

void Menu::drawSoundMenuItem() {
	if (g_engine->getLanguage() == Common::ZH_ANY) {
		g_engine->_screen->fillRect({{115, 129}, 62, 20}, 0);
	} else {
		g_engine->_screen->fillRect({{115, 136}, 62, 13}, 0);
	}
	if (g_engine->_sound->isMuted()) {
		drawMenuItem(kMenu_silent);
	} else {
		drawMenuItem(kMenu_sounds);
	}
}

Common::KeyCode Menu::getLocalisedConfirmToQuitKeycode() {
	switch (g_engine->getLanguage()) {
	case Common::ES_ESP : return Common::KeyCode::KEYCODE_s;
	case Common::FR_FRA : return Common::KeyCode::KEYCODE_o;
	case Common::DE_DEU : return Common::KeyCode::KEYCODE_j;
	default : return Common::KeyCode::KEYCODE_y;
	}
}

Menu::Menu() {
	if (g_engine->getLanguage() == Common::ZH_ANY) {
		_zhFont = new ZhMenuFont("zhmenufont_game.dat", ZhLargeFontType::InGame);
	}
}

Menu::~Menu() {
	delete _zhFont;
}

void Menu::loadMenu() {
	if (g_engine->_sound->isPlayingMusic()) {
		g_engine->_sound->startFadeOut();
		while (g_engine->_sound->isFading()) {
			g_engine->wait();
		}
		g_engine->_sound->stopMusic();
	}

	_open = true;
	Graphics::Surface screenCopy;
	screenCopy.copyFrom(*g_engine->_screen);

	g_engine->showFullscreenPic(g_engine->_room->isGiger() ? "glady.pic" : "clady.pic");
	g_engine->_frame.draw();
	g_engine->drawFullscreenPic();
	g_engine->_console->draw(true);

	drawMenuItem(kMenu_load);
	drawMenuItem(kMenu_save);
	drawSoundMenuItem();
	drawMenuItem(kMenu_resume);
	drawMenuItem(kMenu_quit);

	g_engine->_screen->makeAllDirty();
	g_engine->_screen->update();

	g_engine->zeroMouseButtons();

	g_engine->_lastKeyPressed = Common::KeyCode::KEYCODE_INVALID;

	while (!g_engine->shouldQuit()) {
		g_engine->updateEvents();
		int menuItemIdx = -1;

		if (g_engine->_isLeftMouseClicked) {
			g_engine->zeroMouseButtons();

			Common::Point cursorPos = g_engine->_cursor.getPosition();

			if (cursorPos.x > 111 && cursorPos.x < 178 && cursorPos.y > 50 && cursorPos.y < 235) {
				menuItemIdx = (cursorPos.y - 50) / 37;
			}
		}

		if (g_engine->_lastKeyPressed != Common::KeyCode::KEYCODE_INVALID) {
			switch (g_engine->_lastKeyPressed) {
				case Common::KeyCode::KEYCODE_l: menuItemIdx = 0; break;
				case Common::KeyCode::KEYCODE_s: menuItemIdx = 1; break;
				case Common::KeyCode::KEYCODE_r: menuItemIdx = 3; break;
				case Common::KeyCode::KEYCODE_q: menuItemIdx = 4; break;
				default: break;
			}
			g_engine->_lastKeyPressed = Common::KeyCode::KEYCODE_INVALID;
		}

		if (menuItemIdx > -1) {
			g_engine->_sound->playSfx(5, 0, -1);
			g_engine->waitForSpeechOrSfx();
		}

		if (menuItemIdx == 0) {
			g_engine->loadGameDialog();
			break;
		}
		if (menuItemIdx == 1) {
			// restore game screen back for the save game thumbnail
			g_engine->_screen->copyRectToSurface(screenCopy, 0, 0, {screenCopy.w, screenCopy.h});
			g_engine->_room->installPalette(); // restore room working palette
			g_engine->_screen->update();
			_open = false; // mark menu as closed to allow saving.
			g_engine->saveGameDialog();
			break;
		}
		if (menuItemIdx == 2) { // sound settings
			g_engine->flipMute();
			ConfMan.flushToDisk();
			drawSoundMenuItem();
		}
		if (menuItemIdx == 3) { // Resume
			break;
		}
		if (menuItemIdx == 4) {
			g_engine->_console->printTosText(16);
			g_engine->_console->draw();
			g_engine->_screen->update();
			g_engine->_lastKeyPressed = Common::KeyCode::KEYCODE_INVALID;
			while (!g_engine->shouldQuit()) {
				g_engine->updateEvents();
				if (g_engine->_lastKeyPressed == getLocalisedConfirmToQuitKeycode() || g_engine->_isLeftMouseClicked) {
					g_engine->quitGame();
					break;
				}
				if (g_engine->_lastKeyPressed == Common::KeyCode::KEYCODE_n ||
					g_engine->_lastKeyPressed == Common::KeyCode::KEYCODE_ESCAPE ||
					g_engine->_isRightMouseClicked) {
					g_engine->_console->printTosText(17);
					g_engine->_console->draw();
					g_engine->_screen->update();
					break;
				}
				g_engine->_screen->update();
				g_engine->wait();
			}
		}

		g_engine->_screen->update();
		g_engine->wait();
	}

	g_engine->_room->loadRoomMusic();

	g_engine->removeFullscreenPic();
	screenCopy.free();
	_open = false;
}

} // End of namespace Darkseed
