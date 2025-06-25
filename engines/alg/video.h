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

#ifndef ALG_VIDEO_H
#define ALG_VIDEO_H

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "common/file.h"

namespace Alg {

class AlgVideoDecoder {
public:
	AlgVideoDecoder();
	~AlgVideoDecoder();
	void getNextFrame();
	void loadVideoFromStream(uint32 offset);
	void skipNumberOfFrames(uint32 num);
	void setInputFile(Common::File *input) { _input = input; }
	bool isFinished() const { return _bytesLeft == 0; }
	Graphics::Surface *getVideoFrame() const { return _frame; }
	void setPalette(uint8 *palette) { _palette = palette; }
	bool isPaletteDirty() const { return _paletteDirty; }
	void pauseAudio(bool pause) const { g_system->getMixer()->pauseHandle(_audioHandle, pause); }
	uint16 getWidth() const { return _width; }
	uint16 getHeight() const { return _height; }
	uint32 getCurrentFrame() const { return _currentFrame; }

private:
	Common::File *_input;
	Graphics::Surface *_frame;
	Audio::PacketizedAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;
	uint8 *_palette;
	bool _paletteDirty;
	bool _gotVideoFrame;
	uint32 _currentFrame;
	uint32 _size;
	uint32 _bytesLeft;
	uint16 _currentChunk;

	uint16 _numChunks = 0;
	uint16 _frameRate = 0;
	uint16 _videoMode = 0;
	uint16 _width = 0;
	uint16 _height = 0;
	uint16 _audioType = 0;

	void readNextChunk();
	void decodeIntraFrame(uint32 size, uint8 hh, uint8 hv);
	void decodeInterFrame(uint32 size, uint8 hh, uint8 hv);
	void updatePalette(uint32 size, bool partial);
	void readAudioData(uint32 size, uint16 rate);
};

} // End of namespace Alg

#endif
