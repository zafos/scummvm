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

#ifndef GOT_SOUND_H
#define GOT_SOUND_H

#include "got/musicdriver.h"
#include "got/musicparser.h"
#include "got/data/defines.h"
#include "got/gfx/gfx_chunks.h"

#include "audio/mixer.h"

namespace Got {

enum {
	OW,
	GULP,
	SWISH,
	YAH,
	ELECTRIC,
	THUNDER,
	DOOR,
	FALL,
	ANGEL,
	WOOP,
	DEAD,
	BRAAPP,
	WIND,
	PUNCH1,
	CLANG,
	EXPLODE,
	BOSS11,
	BOSS12,
	BOSS13
};

#define NUM_SOUNDS 19

class Sound {
private:
	static const uint8 MUSIC_TIMER_FREQUENCY_GAME = 120;
	static const uint8 MUSIC_TIMER_FREQUENCY_TITLE = 140;

	byte *_soundData = nullptr;
	byte *_bossSounds[3];
	Header _digiSounds[NUM_SOUNDS];
	Audio::SoundHandle _soundHandle;
	byte _currentPriority = 0;
	int8 _currentBossLoaded = 0;

	const char *_currentMusic = nullptr;
	byte *_musicData = nullptr;
	MusicDriver_Got *_musicDriver = nullptr;
	MusicParser_Got *_musicParser = nullptr;

	const char *getMusicName(int num) const;

public:
	Sound();
	~Sound();
	
	void load();
	void setupBoss(int num);

	void playSound(int index, bool override);
	void playSound(const Gfx::GraphicChunk &src);
	bool soundPlaying() const;

	void musicPlay(const int num, const bool override) {
		musicPlay(getMusicName(num), override);
	}
	void musicPlay(const char *name, bool override);
	void musicPause();
	void musicResume();
	void musicStop();
	bool musicIsOn() const;

	void syncSoundSettings();
};

extern void playSound(int index, bool override);
extern void playSound(const Gfx::GraphicChunk &src);
extern bool soundPlaying();
extern void musicPlay(int num, bool override);
extern void musicPlay(const char *name, bool override);
extern void musicPause();
extern void musicResume();
extern void setupBoss(int num);

} // namespace Got

#endif
