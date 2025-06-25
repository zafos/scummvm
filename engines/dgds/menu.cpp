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

#include "common/system.h"

#include "audio/mixer.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/managed_surface.h"
#include "graphics/surface.h"

#include "dgds/includes.h"
#include "dgds/font.h"
#include "dgds/globals.h"
#include "dgds/menu.h"
#include "dgds/request.h"
#include "dgds/scene.h"
#include "dgds/sound.h"
#include "dgds/minigames/china_train.h"

namespace Dgds {


// TODO: These are the IDs for Dragon, this code needs updates for China/Beamish/etc
enum MenuButtonIds {
	kMainMenuWillySave = 110,
	kMainMenuWillyLoad = 111,
	kMainMenuWillyRestart = 112,
	kMainMenuWillyQuit = 113,
	kMainMenuWillyVCRHelp = 114,
	kMainMenuWillyHelp = 120,
	kMainMenuWillySoundsOnOff = 115,
	kMainMenuWillyMusicOnOff = 116,

	kMenuWillyCreditsDone = 176,
	kMenuWillyHelpDone = 174,

	kMenuMainPlay = 120,
	kMenuMainControls = 20,
	kMenuMainOptions = 121,
	kMenuMainCalibrate = 118, // or Credits in Willy Beamish
	kMenuMainFiles = 119, // or Play in Willy Beamish
	kMenuMainQuit = 122,

	kMenuControlsVCR = 127,
	kMenuControlsPlay = 128,

	kMenuSliderControlsDifficulty = 123,
	kMenuSliderControlsTextSpeed = 125,
	kMenuSliderControlsDetailLevel = 131,

	kMenuOptionsJoystickOnOff = 139,
	kMenuOptionsJoystickOnOffHoC = 174,
	kMenuOptionsMouseOnOff = 138,
	kMenuOptionsMouseOnOffHoC = 173,
	kMenuOptionsSoundsOnOff = 137,
	kMenuOptionsMusicOnOff = 140,
	kMenuOptionsSoundsOnOffHoC = 175,
	kMenuOptionsSoundsOnOffDE = 172, // German version
	kMenuOptionsMusicOnOffHoC = 171,
	kMenuOptionsVCR = 135,
	kMenuOptionsPlay = 136,

	//kMenuCalibrateJoystickBtn = 145,
	//kMenuCalibrateMouseBtn = 146,
	//kMenuCalibrateVCR = 144,
	//kMenuCalibratePlay = 147,
	//kMenuCalibrateVCRHoC = 159,
	//kMenuCalibratePlayHoC = 158,

	kMenuFilesSave = 107,
	kMenuFilesRestore = 106,
	kMenuFilesRestart = 105,
	kMenuFilesVCR = 103,
	kMenuFilesPlay = 130,

	//kMenuSavePrevious = 58,
	//kMenuSaveNext = 59,
	//kMenuSaveSave = 53,
	//kMenuSaveCancel = 54,
	//kMenuSaveChangeDirectory = 55,
	//kMenuChangeDirectoryOK = 95,
	//kMenuChangeDirectoryCancel = 96,

	//kMenuMouseCalibrationCalibrate = 157,
	//kMenuMouseCalibrationPlay = 155,

	//kMenuJoystickCalibrationOK = 132,

	kMenuQuitYes = 134,
	kMenuQuitNo = 133,

	kMenuMaybeBetterSaveYes = 137,
	kMenuMaybeBetterSaveNo = 138,

	// Intro menu in Rise of the Dragon
	kMenuIntroSkip = 143,
	kMenuIntroPlay = 144,

	// Intro menu in Heart of China / Willy Beamish
	kMenuIntroJumpToIntroduction = 156,
	kMenuIntroJumpToGame = 157,
	kMenuIntroRestore = 150,

	kMenuRestartYes = 163,
	kMenuRestartNo = 164,

	// For Dragon/HOC arcade
	kMenuReplayArcadeYes = 139,
	kMenuReplayArcadeNo = 140,

	kMenuFrustratedArcadeWin = 147,
	kMenuFrustratedArcadeKeepTrying = 148,

	kMenuGameOverQuit = 169,
	kMenuGameOverRestart = 168,
	kMenuGameOverRestore = 170,

	// Tank/train menu in Heart of China
	kMenuTankTrainSkipArcade = 153,
	kMenuTankTrainPlayArcade = 154,

	kMenuButtonNone = 0,
};

Menu::Menu() : _curMenu(kMenuNone), _dragGadget(nullptr), _selectedItem(0), _numSelectable(0), _creditsOffset(0),
	_vcrHelpMode(false) {
_screenBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
}

Menu::~Menu() {
	_screenBuffer.free();
}

uint16 Menu::mapMenuNum(uint16 num) const {
	// The DE version of Heart of China used different menu IDs for some things.
	// Instead of putting mappings all through the game, just translate them on
	// load to match the other game versions.
	DgdsEngine *engine = DgdsEngine::getInstance();
	if (engine->getGameId() == GID_HOC && engine->getGameLang() == Common::DE_DEU) {
		switch (num) {
		case 50: return kMenuMain;
		case 80: return kMenuSkipPlayIntro;
		case 82: return kMenuSkipArcade;
		default: return num;
		}
	} else {
		return num;
	}
}

void Menu::setRequestData(const REQFileData &data) {
	for (auto &req : data._requests) {
		_menuRequests[mapMenuNum(req._fileNum)] = req;
	}
}

void Menu::loadVCRHelp(const char *fname) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	Common::SeekableReadStream *s = engine->getResourceManager()->getResource(fname);

	if (!s)
		error("Couldn't open RES file for menu help %s", fname);

	static const MenuButtonIds helpIds[] = {
		kMenuButtonNone, // Help not available right now
		kMainMenuWillySave,
		kMainMenuWillyLoad,
		kMainMenuWillyRestart,
		kMainMenuWillyQuit,
		kMainMenuWillyVCRHelp,
		kMainMenuWillySoundsOnOff,
		kMainMenuWillyMusicOnOff,
		kMainMenuWillyHelp,
		kMenuMainCalibrate, // Same ID as Credits in Willy Beamish
		kMenuMainFiles, 	// Same ID as Resume in Willy Beamish
	};

	int menuId = 0;
	_helpStrings.clear();
	Common::Array<Common::String> message;
	while (!s->eos() && !s->err()) {
		Common::String str = s->readLine();
		if (str == "!" || s->eos()) {
			_helpStrings[helpIds[menuId]] = message;
			message.clear();
			menuId++;
		} else {
			message.push_back(str);
		}
	}

	delete s;
}

void Menu::loadCredits() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	Common::SeekableReadStream *s = engine->getResourceManager()->getResource("CREDITS.RES");

	if (!s)
		error("Couldn't open CREDITS.RES");

	_credits.clear();
	while (!s->eos() && !s->err()) {
		_credits.push_back(s->readLine());
	}
	_creditsOffset = 0;
}

void Menu::setScreenBuffer() {
	Graphics::Surface *dst = g_system->lockScreen();
	_screenBuffer.copyFrom(*dst);
	g_system->unlockScreen();
}

bool Menu::updateOptionsGadget(Gadget *gadget) {
	Audio::Mixer *mixer = DgdsEngine::getInstance()->_mixer;
	const char *mouseStr, *soundStr, *musicStr, *onStr, *offStr;
	if (DgdsEngine::getInstance()->getGameLang() == Common::EN_ANY) {
		mouseStr = "MOUSE";
		soundStr = "SOUND";
		musicStr = "MUSIC";
		onStr = "ON";
		offStr = "OFF";
	} else if (DgdsEngine::getInstance()->getGameLang() == Common::DE_DEU) {
		mouseStr = "MAUS";
		soundStr = "TON";
		musicStr = "MUSIK";
		onStr = "AN";
		offStr = "AUS";
	} else {
		error("Unsupported language %d", DgdsEngine::getInstance()->getGameLang());
	}

	switch (gadget->_gadgetNo) {
	case kMenuOptionsJoystickOnOff:
	case kMenuOptionsJoystickOnOffHoC:
		gadget->_buttonName = Common::String::format("JOYSTICK %s", onStr);
		return true;
	case kMenuOptionsMouseOnOff:
	case kMenuOptionsMouseOnOffHoC:
		gadget->_buttonName = Common::String::format("%s %s", mouseStr, onStr);
		return true;
	case kMenuOptionsSoundsOnOff: // same id as kMenuMaybeBetterSaveYes
	case kMenuOptionsSoundsOnOffDE:
	case kMenuOptionsSoundsOnOffHoC: {
		bool isMuted = mixer->isSoundTypeMuted(Audio::Mixer::kSFXSoundType);
		gadget->_buttonName = Common::String::format("%s %s", soundStr, isMuted ? offStr : onStr);
		return true;
	}
	case kMenuOptionsMusicOnOff:
	case kMenuOptionsMusicOnOffHoC: {
		bool isMuted = mixer->isSoundTypeMuted(Audio::Mixer::kMusicSoundType);
		gadget->_buttonName = Common::String::format("%s %s", musicStr, isMuted ? offStr : onStr);
		return true;
	}
	default:
		return false;
	}
}

void Menu::configureGadget(MenuId menu, Gadget *gadget) {
	DgdsEngine *engine = DgdsEngine::getInstance();

	// a bit of a hack - set up the gadget with the correct value before we draw it.
	if (menu == kMenuControls) {
		SliderGadget *slider = dynamic_cast<SliderGadget *>(gadget);
		if (!slider)
			return;
		switch (gadget->_gadgetNo) {
		case kMenuSliderControlsDifficulty:
			slider->setSteps(3, false);
			slider->setValue(engine->getDifficulty()); // TODO: set a difficulty value
			break;
		case kMenuSliderControlsTextSpeed:
			slider->setSteps(10, false);
			slider->setValue(engine->getTextSpeed());
			break;
		case kMenuSliderControlsDetailLevel:
			slider->setSteps(2, true);
			slider->setValue(engine->getDetailLevel());
			break;
		default:
			break;
			// do nothing.
		}
	} else if (menu == kMenuOptions) {
		updateOptionsGadget(gadget);
	} else if (menu == kMenuMain && engine->getGameId() == GID_WILLY) {
		// HACK: Enable this button which for some reason is disabled
		// by default in data?
		if (gadget->_gadgetNo == kMainMenuWillyHelp)
			toggleGadget(kMainMenuWillyHelp, true);
	}
}

void Menu::onTick() {
	if (_curMenu == kMenuWillyCredits)
		drawMenu(kMenuWillyCredits);
}

void Menu::drawMenu(MenuId menu, bool clearScreen /* = true*/) {
	bool firstDraw = (_curMenu != menu);
	_curMenu = menu;

	Common::Array<Common::SharedPtr<Gadget> > gadgets = _menuRequests[_curMenu]._gadgets;

	// Restore background when drawing submenus
	if (clearScreen)
		g_system->copyRectToScreen(_screenBuffer.getPixels(), _screenBuffer.pitch, 0, 0, _screenBuffer.w, _screenBuffer.h);

	// This is not very efficient, but it only happens once when the menu is opened.
	Graphics::Surface *screen = g_system->lockScreen();
	Graphics::ManagedSurface managed(screen->w, screen->h, screen->format);
	managed.blitFrom(*screen);
	_menuRequests[_curMenu].drawBg(&managed);

	_numSelectable = 0;
	for (Common::SharedPtr<Gadget> &gptr : gadgets) {
		Gadget *gadget = gptr.get();
		if (gadget->_gadgetType == kGadgetButton || gadget->_gadgetType == kGadgetSlider) {
			if (firstDraw)
				configureGadget(menu, gadget);
			gadget->draw(&managed);
			_numSelectable++;
		}
	}
	if (firstDraw) {
		_selectedItem = _numSelectable - 1;
		putMouseOnSelectedItem();
	}

	drawMenuText(managed);

	if (_curMenu == kMenuWillyCredits)
		drawCreditsText(managed);

	// Can't use transparent blit here as the font is often color 0.
	screen->copyRectToSurface(*managed.surfacePtr(), 0, 0, Common::Rect(screen->w, screen->h));

	g_system->unlockScreen();
	g_system->updateScreen();
}

void Menu::hideMenu() {
	_curMenu = kMenuNone;
	if (_vcrHelpMode) {
		_vcrHelpMode = false;
		DgdsEngine::getInstance()->setMouseCursor(-1);
	}
}

Gadget *Menu::getSelectedItem() {
	int item = 0;
	Common::Array<Common::SharedPtr<Gadget> > gadgets = _menuRequests[_curMenu]._gadgets;
	for (Common::SharedPtr<Gadget> &gptr : gadgets) {
		Gadget *gadget = gptr.get();
		if (gadget->_gadgetType == kGadgetButton || gadget->_gadgetType == kGadgetSlider) {
			if (item == _selectedItem)
				return gadget;
			item++;
		}
	}
	return nullptr;
}

void Menu::putMouseOnSelectedItem() {
	Gadget *selected = getSelectedItem();
	if (!selected)
		return;

	const Common::Point midPt = selected->midPoint();
	// put the mouse on the first button/slider
	g_system->warpMouse(midPt.x, midPt.y);
	return;
}


void Menu::drawMenuText(Graphics::ManagedSurface &dst) {
	Common::Array<Common::SharedPtr<Gadget> > gadgets = _menuRequests[_curMenu]._gadgets;
	Common::Array<TextItem> textItems = _menuRequests[_curMenu]._textItemList;

	if (gadgets.empty())
		return;

	// TODO: Get the parent coordinates properly
	uint16 parentX = gadgets[0].get()->_parentX;
	uint16 parentY = gadgets[0].get()->_parentY;
	uint16 pos = 0;

	for (TextItem &textItem : textItems) {
		// HACK: Skip the first entry, which corresponds to the header
		if (pos == 0) {
			pos++;
			continue;
		}

		const DgdsFont *font = RequestData::getMenuFont();
		int w = font->getStringWidth(textItem._txt);
		font->drawString(dst.surfacePtr(), textItem._txt, parentX + textItem._x, parentY + textItem._y, w, 0);
		pos++;
	}
}

static byte _creditsColor(int y) {
	if (y < 31 || y > 86)
		return 17; // Cyan
	else if (y < 32 || y > 85)
		return 18; // Slightly darker cyan
	else if (y < 33 || y > 84)
		return 19; // Darker cyan
	else if (y < 34 || y > 83)
		return 30; // Grey
	return 0; // Black.
}

void Menu::drawCreditsText(Graphics::ManagedSurface &dst) {
	const DgdsFont *font = RequestData::getMenuFont();
	const int lineHeight = font->getFontHeight();

	DgdsRect dlgRect = _menuRequests[_curMenu]._rect;

	const int dlgWidth = dlgRect.width;

	const int yMin = 30;
	const int yMax = 87;
	for (uint i = 0; i < _credits.size(); i++) {
		int dstY = i * lineHeight + yMax - _creditsOffset / 4;
		if (dstY > yMax)
			break;
		if (dstY > yMin) {
			int lineW = font->getStringWidth(_credits[i]);
			int xoff = dlgRect.x + (dlgWidth - lineW) / 2;
			font->drawString(dst.surfacePtr(), _credits[i], xoff, dlgRect.y + dstY, dlgRect.width, _creditsColor(dstY));
		}
	}

	_creditsOffset++;

	if ((uint)_creditsOffset / 4 > lineHeight * (_credits.size() + 1) + (yMax - yMin))
		_creditsOffset = 0;
}

Gadget *Menu::getClickedMenuItem(const Common::Point &mouseClick) {
	if (_curMenu == kMenuNone)
		return nullptr;

	Common::Array<Common::SharedPtr<Gadget> > gadgets = _menuRequests[_curMenu]._gadgets;

	for (Common::SharedPtr<Gadget> &gptr : gadgets) {
		Gadget *gadget = gptr.get();
		if (gadget->_gadgetType == kGadgetButton || gadget->_gadgetType == kGadgetSlider) {
			if (gadget->containsPoint(mouseClick)) {
				return gadget;
			}
		}
	}

	return nullptr;
}

void Menu::onMouseLDown(const Common::Point &mouse) {
	SliderGadget *slider = dynamic_cast<SliderGadget *>(getClickedMenuItem(mouse));
	if (slider) {
		_dragGadget = slider;
		_dragStartPt = mouse;
	}
}

void Menu::onMouseMove(const Common::Point &mouse) {
	if (!_dragGadget)
		return;
	_dragGadget->onDrag(mouse);
	drawMenu(_curMenu);
}

void Menu::onMouseLUp(const Common::Point &mouse) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	if (_dragGadget && mouse != _dragStartPt) {
		int16 setting = _dragGadget->onDragFinish(mouse);
		switch (_dragGadget->_gadgetNo) {
		case kMenuSliderControlsDifficulty:
			engine->setDifficulty(setting);
			break;
		case kMenuSliderControlsTextSpeed:
			engine->setTextSpeed(setting);
			break;
		case kMenuSliderControlsDetailLevel:
			engine->setDetailLevel(static_cast<DgdsDetailLevel>(setting));
			break;
		default:
			break;
		}
		drawMenu(_curMenu);
		_dragGadget = nullptr;
		_dragStartPt = Common::Point();
		return;
	}
	_dragGadget = nullptr;

	Gadget *gadget = getClickedMenuItem(mouse);
	bool isToggle = false;
	if (!gadget)
		return;

	// Click animation
	if (dynamic_cast<ButtonGadget *>(gadget) && !_vcrHelpMode) {
		gadget->toggle(false);
		if (_curMenu == kMenuOptions)
			/*isToggle = */updateOptionsGadget(gadget);
		drawMenu(_curMenu, false);
		g_system->delayMillis(300);
		gadget->toggle(true);
		isToggle = true;
	}

	if (_curMenu == kMenuOptions)
		handleClickOptionsMenu(mouse);
	else if (_curMenu == kMenuSkipPlayIntro)
		handleClickSkipPlayIntroMenu(mouse);
	else
		isToggle &= handleClick(mouse);

	if (isToggle)
		drawMenu(_curMenu);
}


static void _toggleSoundType(Audio::Mixer::SoundType soundType) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	Audio::Mixer *mixer = engine->_mixer;
	//const char *typeStr = (soundType == Audio::Mixer::kMusicSoundType) ? "music" : "sfx";
	if (!mixer->isSoundTypeMuted(soundType)) {
		mixer->muteSoundType(soundType, true);
		engine->_soundPlayer->muteSoundType(soundType);
	} else {
		mixer->muteSoundType(soundType, false);
		engine->_soundPlayer->unmuteSoundType(soundType);
	}
}

bool Menu::handleClick(const Common::Point &mouse) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	int currentScene = engine->getScene()->getNum();
	Gadget *gadget = getClickedMenuItem(mouse);
	int16 clickedMenuItem = gadget->_gadgetNo;

	if (_vcrHelpMode) {
		doVcrHelp(clickedMenuItem);
		return false;
	}

	switch (clickedMenuItem) {
	case kMenuMainPlay:
	case kMenuControlsPlay:
	case kMenuOptionsPlay:
	case kMenuFilesPlay:
	case kMenuMaybeBetterSaveNo:
	//case kMenuCalibratePlay:
	//case kMenuCalibratePlayHoC:
	//case kMenuMouseCalibrationPlay:
		hideMenu();
		if (engine->getGameId() == GID_WILLY && clickedMenuItem == kMainMenuWillyHelp) {
			// TODO: Based on some variable this should instead:
			//drawMenu(kMenuWillyHelp); // with the first message from WVCR.RES
			// The OnLine help system
			// is not available now.
			engine->changeScene(80);
		} else {
			CursorMan.showMouse(false);
		}
		break;
	case kMenuMainControls:
		drawMenu(kMenuControls);
		break;
	case kMenuMainOptions:
		drawMenu(kMenuOptions);
		break;
	case kMenuMainCalibrate: // same as credits button in Willy Beamish
		if (engine->getGameId() == GID_WILLY) {
			hideMenu();
			loadCredits();
			drawMenu(kMenuWillyCredits);
		} else {
			debug("Ignoring calibration request");
		}
		break;
	//case kMenuJoystickCalibrationOK:
	//case kMenuMouseCalibrationCalibrate: // NOTE: same ID as kMenuIntroJumpToGame (for HOC)
	case kMenuIntroJumpToGame:
		if (_curMenu == kMenuSkipPlayIntro) {
			hideMenu();
			engine->setShowClock(true);
			engine->changeScene(24);
		} else {
			// Do nothing - the calibrate menu doesn't offer
			// any functionality in ScummVM
			//drawMenu(kMenuCalibrate);
			drawMenu(_curMenu);
		}
		break;
	case kMenuMainFiles: // Same ID as Play in Willy Beamish
	//case kMenuSaveCancel:
		if (engine->getGameId() == GID_WILLY)
			hideMenu();
		else
			drawMenu(kMenuFiles);
		break;
	case kMenuMainQuit:
		drawMenu(kMenuReallyQuit);
		break;
	case kMainMenuWillyVCRHelp:
		startVcrHelp();
		break;
	//case kMenuCalibrateVCR: // NOTE: same ID as kMenuIntroPlay
	case kMenuIntroPlay:
	case kMenuControlsVCR:
	case kMenuOptionsVCR:
	//case kMenuCalibrateVCRHoC:
	case kMenuFilesVCR:
	case kMenuQuitNo:
	case kMenuRestartNo:
		drawMenu(kMenuMain);
		break;
	//case kMenuCalibrateJoystickBtn:
	//	drawMenu(kMenuCalibrateJoystick);
	//	break;
	//case kMenuCalibrateMouseBtn:
	//	drawMenu(kMenuCalibrateMouse);
	//	break;
	//case kMenuChangeDirectoryCancel:
	//	drawMenu(kMenuSaveDlg);
	//	break;
	case kMenuFilesRestore:
	case kMenuGameOverRestore:
	case kMenuIntroRestore:
	case kMainMenuWillyLoad:
		if (g_engine->loadGameDialog()) {
			hideMenu();
			return false;
		} else {
			drawMenu(_curMenu);
		}
		break;
	case kMenuFilesRestart:
	case kMainMenuWillyRestart:
		drawMenu(kMenuRestart);
		break;
	case kMenuFilesSave: // TODO: Add an option to support original save/load dialogs?
	//case kMenuSavePrevious:
	//case kMenuSaveNext:
	//case kMenuSaveSave:
	case kMenuMaybeBetterSaveYes:
	case kMainMenuWillySave:
		if (g_engine->saveGameDialog())
			hideMenu();
		else
			drawMenu(_curMenu);
		break;
	//case kMenuSaveChangeDirectory:
	//	drawMenu(kMenuChangeDir);
	//	break;
	//case kMenuChangeDirectoryOK:
	//	// Do nothing - the change directory menu doesn't offer
	//	// any functionality in ScummVM
	//	drawMenu(_curMenu);
	//	break;
	case kMenuQuitYes:
		g_engine->quitGame();
		return false;
	case kMenuRestartYes:
		hideMenu();
		engine->restartGame();
		// don't draw the button coming back up.
		return false;
	case kMenuGameOverQuit:
	case kMainMenuWillyQuit:
		drawMenu(kMenuReallyQuit);
		break;
	case kMenuGameOverRestart:
		drawMenu(kMenuRestart);
		break;
	case kMenuSliderControlsDifficulty: {
		SliderGadget *slider = dynamic_cast<SliderGadget *>(gadget);
		if (!slider)
			break;
		int16 setting = slider->onClick(mouse);
		engine->setDifficulty(setting);
		// redraw for update.
		drawMenu(_curMenu);
		break;
	}
	case kMenuSliderControlsTextSpeed: {
		SliderGadget *slider = dynamic_cast<SliderGadget *>(gadget);
		if (!slider)
			break;
		int16 setting = slider->onClick(mouse);
		engine->setTextSpeed(setting);
		drawMenu(_curMenu);
		break;
	}
	case kMenuSliderControlsDetailLevel: {
		SliderGadget *slider = dynamic_cast<SliderGadget *>(gadget);
		if (!slider)
			break;
		int16 setting = slider->onClick(mouse);
		engine->setDetailLevel(static_cast<DgdsDetailLevel>(setting));
		drawMenu(_curMenu);
		break;
	}
	case kMenuReplayArcadeYes:
	case kMenuReplayArcadeNo:
		if (_curMenu != kMenuReplayArcade)
			break;
		if (engine->getGameId() == GID_DRAGON) {
			DragonGlobals *dragonGlobals = static_cast<DragonGlobals *>(engine->getGameGlobals());
			dragonGlobals->setArcadeState(clickedMenuItem == kMenuReplayArcadeYes ? 20 : 10);
		} else if (engine->getGameId() == GID_HOC) {
			engine->getChinaTrain()->setMenuResult(clickedMenuItem == kMenuReplayArcadeYes);
		}
		hideMenu();
		break;
	case kMenuFrustratedArcadeWin:
	case kMenuFrustratedArcadeKeepTrying:
		if (_curMenu != kMenuArcadeFrustrated)
			break;
		if (engine->getGameId() == GID_DRAGON) {
			DragonGlobals *dragonGlobals = static_cast<DragonGlobals *>(engine->getGameGlobals());
			dragonGlobals->setArcadeState(clickedMenuItem == kMenuFrustratedArcadeWin ? 6 : 20);
		} else if (engine->getGameId() == GID_HOC) {
			engine->getChinaTrain()->setMenuResult(clickedMenuItem == kMenuFrustratedArcadeKeepTrying);
		}
		hideMenu();
		break;
	case kMenuTankTrainSkipArcade:
		hideMenu();
		if (currentScene == 73)
			engine->changeScene(12);	// skip tank mini-game
		else if (currentScene == 84)
			engine->changeScene(57);	// skip train mini-game, return to travel map
		break;
	case kMenuTankTrainPlayArcade:
		// Open the "save before arcade" menu before playing the tank
		// or train mini-games.
		drawMenu(kMenuSaveBeforeArcade);
		break;
	case kMainMenuWillySoundsOnOff:
		_toggleSoundType(Audio::Mixer::kSFXSoundType);
		updateOptionsGadget(gadget);
		break;
	case kMainMenuWillyMusicOnOff:
		_toggleSoundType(Audio::Mixer::kMusicSoundType);
		updateOptionsGadget(gadget);
		break;
	case kMenuWillyCreditsDone:
		hideMenu();
		break;
	case kMenuWillyHelpDone:
		drawMenu(kMenuMain, false);
		break;
	default:
		debug(1, "Clicked ID %d", clickedMenuItem);
		break;
	}
	return true;
}

void Menu::startVcrHelp() {
	_vcrHelpMode = true;
	Common::SharedPtr<Graphics::ManagedSurface> iconSurf = RequestData::getCorners()->getSurface(37);
	int hotspotX = iconSurf->w / 2;
	int hotspotY = iconSurf->h / 2;
	CursorMan.replaceCursor(*(iconSurf->surfacePtr()), hotspotX, hotspotY, 0, 0);
}

void Menu::doVcrHelp(int16 button) {
	if (_helpStrings.contains(button)) {
		RequestData &helpRequest = _menuRequests[kMenuWillyVCRHelp];
		const Common::Array<Common::String> msg = _helpStrings[button];
		const DgdsFont *font = RequestData::getMenuFont();
		const Gadget *doneButton = helpRequest.findGadgetByNumWithFlags3Not0x40(174);
		int16 msgHeight = font->getFontHeight() * msg.size();
		int16 msgAreaTop = helpRequest._textItemList.front()._y + font->getFontHeight();
		int16 msgAreaHeight = doneButton->_y - msgAreaTop;
		int16 msgAreaLeft = 0;
		int16 msgAreaWidth = helpRequest._rect.width;

		// Vertically centre the message
		int16 msgTop = msgAreaTop + (msgAreaHeight - msgHeight) / 2;

		while (helpRequest._textItemList.size() > 1)
			helpRequest._textItemList.pop_back();

		for (const Common::String &line : msg) {
			TextItem helpText;
			int msgLen = font->getStringWidth(line);
			// Horizontally centre the line
			helpText._x = msgAreaLeft + (msgAreaWidth - msgLen) / 2;
			helpText._y = msgTop;
			helpText._txt = line;
			ARRAYCLEAR(helpText._vals);
			msgTop += font->getFontHeight();
			helpRequest._textItemList.push_back(helpText);
		}

		// HACK: Set a different cursor to force an update.
		DgdsEngine::getInstance()->setMouseCursor(1);
		DgdsEngine::getInstance()->setMouseCursor(-1);
		drawMenu(kMenuWillyVCRHelp, false);
		_vcrHelpMode = false;
	}
}

void Menu::handleClickOptionsMenu(const Common::Point &mouse) {
	Gadget *gadget = getClickedMenuItem(mouse);
	int16 clickedMenuItem = gadget->_gadgetNo;

	switch (clickedMenuItem) {
	case kMenuOptionsJoystickOnOff:
	case kMenuOptionsJoystickOnOffHoC:
	case kMenuOptionsMouseOnOff:  // same id as kMenuMaybeBetterSaveNo
	case kMenuOptionsMouseOnOffHoC:
		// Do nothing - we don't toggle joystick or mouse functionality
		break;
	case kMenuOptionsSoundsOnOff: // same id as kMenuMaybeBetterSaveYes
	case kMenuOptionsSoundsOnOffDE:
	case kMenuOptionsSoundsOnOffHoC:
	case kMainMenuWillySoundsOnOff:
		_toggleSoundType(Audio::Mixer::kSFXSoundType);
		updateOptionsGadget(gadget);
		break;
	case kMenuOptionsMusicOnOff:
	case kMenuOptionsMusicOnOffHoC:
	case kMainMenuWillyMusicOnOff:
		_toggleSoundType(Audio::Mixer::kMusicSoundType);
		updateOptionsGadget(gadget);
		break;
	default:
		handleClick(mouse);
		break;
	}
}

void Menu::handleClickSkipPlayIntroMenu(const Common::Point &mouse) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	Gadget *gadget = getClickedMenuItem(mouse);
	int16 clickedMenuItem = gadget->_gadgetNo;

	switch (clickedMenuItem) {
	case kMenuIntroPlay:
		hideMenu();
		break;
	case kMenuIntroSkip:
		hideMenu();
		engine->setShowClock(true);
		engine->changeScene(5);
		break;
	case kMenuIntroJumpToIntroduction:
		hideMenu();
		if (engine->getGameId() == GID_HOC)
			engine->changeScene(100);
		else if (engine->getGameId() == GID_WILLY)
			engine->changeScene(24);
		break;
	case kMenuIntroJumpToGame:
		hideMenu();
		if (engine->getGameId() == GID_HOC)
			engine->changeScene(24);
		else if (engine->getGameId() == GID_WILLY)
			engine->changeScene(4);
		break;
	default:
		handleClick(mouse);
		break;
	}
}

void Menu::toggleGadget(int16 gadgetId, bool enable) {
	Common::Array<Common::SharedPtr<Gadget> > gadgets = _menuRequests[_curMenu]._gadgets;

	for (Common::SharedPtr<Gadget> &gptr : gadgets) {
		Gadget *gadget = gptr.get();
		if (gadget->_gadgetNo == gadgetId) {
			gadget->toggle(enable);
			return;
		}
	}
}

/**
 * Choose the next menu item via keyboard - items are numbered backwards
 * so this *decreases* the counter.
 */
void Menu::nextChoice() {
	_selectedItem--;
	if (_selectedItem < 0)
		_selectedItem = _numSelectable - 1;
	putMouseOnSelectedItem();
}

void Menu::prevChoice() {
	_selectedItem++;
	if (_selectedItem >= _numSelectable)
		_selectedItem = 0;
	putMouseOnSelectedItem();
}

void Menu::activateChoice() {
	Gadget *selected = getSelectedItem();
	if (!selected)
		return;

	handleClick(selected->midPoint());
}

} // End of namespace Dgds
