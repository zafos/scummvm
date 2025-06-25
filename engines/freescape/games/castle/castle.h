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

namespace Freescape {

struct RiddleText {
	int8 _dx;
	int8 _dy;
	Common::String _text;

	RiddleText(int8 dx, int8 dy, const Common::String &text) : _dx(dx), _dy(dy), _text(text) {}
};

struct Riddle {
	Common::Point _origin;
	Common::Array<RiddleText> _lines;
};

class CastleEngine : public FreescapeEngine {
public:
	CastleEngine(OSystem *syst, const ADGameDescription *gd);
	~CastleEngine();

	// Only in DOS
	Graphics::ManagedSurface *_option;
	Graphics::ManagedSurface *_menuButtons;
	Graphics::ManagedSurface *_menuCrawlIndicator;
	Graphics::ManagedSurface *_menuWalkIndicator;
	Graphics::ManagedSurface *_menuRunIndicator;
	Graphics::ManagedSurface *_menuFxOnIndicator;
	Graphics::ManagedSurface *_menuFxOffIndicator;
	Graphics::ManagedSurface *_menu;

	void initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) override;
	void initGameState() override;
	void endGame() override;

	void drawInfoMenu() override;
	void loadAssets() override;
	void loadAssetsDOSFullGame() override;
	void loadAssetsDOSDemo() override;
	void loadAssetsAmigaDemo() override;
	void loadAssetsZXFullGame() override;
	void loadAssetsCPCFullGame() override;
	void borderScreen() override;
	void selectCharacterScreen();
	void drawOption();

	void initZX();
	void initDOS();
	void initCPC();

	void drawDOSUI(Graphics::Surface *surface) override;
	void drawZXUI(Graphics::Surface *surface) override;
	void drawCPCUI(Graphics::Surface *surface) override;
	void drawAmigaAtariSTUI(Graphics::Surface *surface) override;
	void drawEnergyMeter(Graphics::Surface *surface, Common::Point origin);
	void pressedKey(const int keycode) override;
	void checkSensors() override;
	void updateTimeVariables() override;

	bool checkIfGameEnded() override;
	void drawSensorShoot(Sensor *sensor) override;

	void executePrint(FCLInstruction &instruction) override;
	void executeDestroy(FCLInstruction &instruction) override;
	void executeRedraw(FCLInstruction &instruction) override;
	void gotoArea(uint16 areaID, int entranceID) override;
	Common::Error saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStreamExtended(Common::SeekableReadStream *stream) override;

	Common::Array<Riddle> _riddleList;
	Common::BitArray _fontPlane1;
	Common::BitArray _fontPlane2;
	Common::BitArray _fontPlane3;

	void drawRiddleStringInSurface(const Common::String &str, int x, int y, uint32 fontColor, uint32 backColor, Graphics::Surface *surface);
	Graphics::ManagedSurface *loadFrameWithHeaderDOS(Common::SeekableReadStream *file);
	Common::Array <Graphics::ManagedSurface *>loadFramesWithHeaderDOS(Common::SeekableReadStream *file, int numFrames);

	Common::Array<Graphics::ManagedSurface *> loadFramesWithHeader(Common::SeekableReadStream *file, int pos, int numFrames, uint32 front, uint32 back);
	Graphics::ManagedSurface *loadFrameWithHeader(Common::SeekableReadStream *file, int pos, uint32 front, uint32 back);
	Graphics::ManagedSurface *loadFrame(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int width, int height, uint32 back);
	Graphics::ManagedSurface *loadFrameFromPlanes(Common::SeekableReadStream *file, int widthInBytes, int height);
	Graphics::ManagedSurface *loadFrameFromPlanesInternal(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int width, int height);

	Graphics::ManagedSurface *loadFrameFromPlanesVertical(Common::SeekableReadStream *file, int widthInBytes, int height);
	Graphics::ManagedSurface *loadFrameFromPlanesInternalVertical(Common::SeekableReadStream *file, Graphics::ManagedSurface *surface, int width, int height, int plane);

	Common::Array<Graphics::ManagedSurface *>_keysBorderFrames;
	Common::Array<Graphics::ManagedSurface *>_keysMenuFrames;
	Graphics::ManagedSurface *_spiritsMeterIndicatorBackgroundFrame;
	Graphics::ManagedSurface *_spiritsMeterIndicatorFrame;
	Graphics::ManagedSurface *_spiritsMeterIndicatorSideFrame;
	Graphics::ManagedSurface *_strenghtBackgroundFrame;
	Graphics::ManagedSurface *_strenghtBarFrame;
	Common::Array<Graphics::ManagedSurface *> _strenghtWeightsFrames;
	Common::Array<Graphics::ManagedSurface *> _flagFrames;
	Graphics::ManagedSurface *_thunderFrame;
	Graphics::ManagedSurface *_riddleTopFrame;
	Graphics::ManagedSurface *_riddleBackgroundFrame;
	Graphics::ManagedSurface *_riddleBottomFrame;

	Graphics::ManagedSurface *_endGameThroneFrame;
	Graphics::ManagedSurface *_endGameBackgroundFrame;
	Graphics::ManagedSurface *_gameOverBackgroundFrame;

	Common::Array<int> _keysCollected;
	bool _useRockTravel;
	int _spiritsMeter;
	int _spiritsMeterPosition;
	int _spiritsMeterMax;
	int _spiritsToKill;

	int _lastTenSeconds;

private:
	Common::SeekableReadStream *decryptFile(const Common::Path &filename);
	void loadRiddles(Common::SeekableReadStream *file, int offset, int number);
	void loadDOSFonts(Common::SeekableReadStream *file, int pos);
	void drawFullscreenRiddleAndWait(uint16 riddle);
	void drawFullscreenEndGameAndWait();
	void drawFullscreenGameOverAndWait();
	void drawRiddle(uint16 riddle, uint32 front, uint32 back, Graphics::Surface *surface);
	void tryToCollectKey();
	void addGhosts();
	bool ghostInArea();
	Texture *_optionTexture;
	Font _fontRiddle;
};

}
