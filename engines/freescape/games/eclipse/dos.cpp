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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "freescape/freescape.h"
#include "freescape/games/eclipse/eclipse.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

extern byte kEGADefaultPalette[16][3];
extern byte kCGAPaletteRedGreen[4][3];
extern byte kCGAPalettePinkBlue[4][3];

void EclipseEngine::initDOS() {
	_viewArea = Common::Rect(40, 33, 280, 133);
	_soundIndexShoot = 18;
}

void EclipseEngine::loadAssetsDOSFullGame() {
	Common::File file;
	if (_renderMode == Common::kRenderEGA) {
		file.open("SCN1E.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		}
		file.close();
		file.open("TOTEE.EXE");

		if (!file.isOpen())
			error("Failed to open TOTEE.EXE");

		loadMessagesFixedSize(&file, 0x710f, 16, 20);
		loadSoundsFx(&file, 0xd670, 5);
		loadSpeakerFxDOS(&file, 0x7396 + 0x200, 0x72a1 + 0x200);
		loadFonts(&file, 0xd403);
		load8bitBinary(&file, 0x3ce0, 16);

		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte *)&kEGADefaultPalette, 0, 16);

		_indicators.push_back(loadBundledImage("eclipse_ankh_indicator"));

		for (auto &it : _indicators)
			it->convertToInPlace(_gfx->_texturePixelFormat);

	} else if (_renderMode == Common::kRenderCGA) {
		file.open("SCN1C.DAT");
		if (file.isOpen()) {
			_title = load8bitBinImage(&file, 0x0);
			_title->setPalette((byte *)&kCGAPaletteRedGreen, 0, 4);
		}
		file.close();
		file.open("TOTEC.EXE");

		if (!file.isOpen())
			error("Failed to open TOTEC.EXE");

		loadMessagesFixedSize(&file, 0x594f, 16, 20);
		loadSoundsFx(&file, 0xb9f0, 5);
		loadFonts(&file, 0xb785);
		load8bitBinary(&file, 0x2530, 4);
		_border = load8bitBinImage(&file, 0x210);
		_border->setPalette((byte *)&kCGAPaletteRedGreen, 0, 4);
		swapPalette(_startArea);
	} else
		error("Invalid or unsupported render mode %s for Total Eclipse", Common::getRenderModeDescription(_renderMode));
}

void EclipseEngine::drawDOSUI(Graphics::Surface *surface) {
	int score = _gameStateVars[k8bitVariableScore];
	int shield = _gameStateVars[k8bitVariableShield] * 100 / _maxShield;
	shield = shield < 0 ? 0 : shield;

	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x55);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);
	uint32 red = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0x00, 0x00);
	uint32 blue = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x55, 0x55, 0xFF);
	uint32 green = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x55, 0xFF, 0x55);
	uint32 redish = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0x55, 0x55);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 102, 135, black, yellow, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else if (!_currentAreaMessages.empty())
		drawStringInSurface(_currentArea->_name, 102, 135, black, yellow, surface);

	Common::String encodedScoreStr = getScoreString(score);
	drawStringInSurface(encodedScoreStr, 136, 6, black, white, surface);

	int x = 171;
	if (shield < 10)
		x = 179;
	else if (shield < 100)
		x = 175;

	Common::String shieldStr = Common::String::format("%d", shield);
	drawStringInSurface(shieldStr, x, 162, black, redish, surface);

	drawStringInSurface(shiftStr("0", 'Z' - '$' + 1 - _angleRotationIndex), 79, 135, black, yellow, surface);
	drawStringInSurface(shiftStr("3", 'Z' - '$' + 1 - _playerStepIndex), 63, 135, black, yellow, surface);
	drawStringInSurface(shiftStr("7", 'Z' - '$' + 1 - _playerHeightNumber), 240, 135, black, yellow, surface);

	if (_shootingFrames > 0) {
		drawStringInSurface(shiftStr("4", 'Z' - '$' + 1), 232, 135, black, yellow, surface);
		drawStringInSurface(shiftStr("<", 'Z' - '$' + 1), 240, 135, black, yellow, surface);
	}
	drawAnalogClock(surface, 90, 172, black, red, white);

	Common::Rect jarBackground(124, 165, 148, 192);
	surface->fillRect(jarBackground, black);

	Common::Rect jarWater(124, 192 - _gameStateVars[k8bitVariableEnergy], 148, 192);
	surface->fillRect(jarWater, blue);

	drawIndicator(surface, 41, 4, 16);
	drawEclipseIndicator(surface, 228, 0, yellow, green);
	surface->fillRect(Common::Rect(225, 168, 235, 187), white);
	drawCompass(surface, 229, 177, _yaw, 10, black);
}

soundFx *EclipseEngine::load1bPCM(Common::SeekableReadStream *file, int offset) {
	soundFx *sound = (soundFx *)malloc(sizeof(soundFx));
	file->seek(offset);
	sound->size = file->readUint16LE();
	debugC(1, kFreescapeDebugParser, "size: %d", sound->size);
	sound->sampleRate = file->readUint16LE();
	debugC(1, kFreescapeDebugParser, "sample rate?: %f", sound->sampleRate);

	uint8 *data = (uint8 *)malloc(sound->size * sizeof(uint8) * 8);
	for (int i = 0; i < sound->size; i++) {
		uint8 byte = file->readByte();
		for (int j = 0; j < 8; j++) {
			data[8 * i + j] = byte & 1 ? 255 : 0;
			byte = byte >> 1;
		}
	}
	sound->size = sound->size * 8;
	sound->data = (byte *)data;
	return sound;
}

void EclipseEngine::loadSoundsFx(Common::SeekableReadStream *file, int offset, int number) {
	if (isAmiga() || isAtariST()) {
		FreescapeEngine::loadSoundsFx(file, offset, number);
		return;
	}

	for (int i = 0; i < number; i++) {
		_soundsFx[i] = load1bPCM(file, offset);
		offset += (_soundsFx[i]->size / 8) + 4;
	}
}


void EclipseEngine::playSoundFx(int index, bool sync) {
	if (isAmiga() || isAtariST()) {
		FreescapeEngine::playSoundFx(index, sync);
		return;
	}

	if (_soundsFx.size() == 0) {
		debugC(1, kFreescapeDebugMedia, "WARNING: Sounds are not loaded");
		return;
	}

	int size = _soundsFx[index]->size;
	//int sampleRate = _soundsFx[index]->sampleRate;
	byte *data = _soundsFx[index]->data;

	Audio::SeekableAudioStream *stream = Audio::makeRawStream(data, size, 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, stream, -1, kFreescapeDefaultVolume / 10);
}


} // End of namespace Freescape
