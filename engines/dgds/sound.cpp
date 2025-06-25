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

#include "common/debug.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"

#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"

#include "dgds/decompress.h"
#include "dgds/dgds.h"
#include "dgds/includes.h"
#include "dgds/resource.h"
#include "dgds/sound.h"
#include "dgds/sound/music.h"
#include "dgds/sound/resource/sci_resource.h"

namespace Dgds {

static const uint16 SIGNAL_OFFSET = 0xffff;

// Offsets from sound/music num to playing ID.
// This is to make a fake SCI-style "resource ID"
static const int SND_RESOURCE_OFFSET = 4096;
static const int MUSIC_RESOURCE_OFFSET = 8192;

static const uint16 FLAG_LOOP = 1;

static void _readHeader(const byte* &pos, uint32 &sci_header) {
	sci_header = 0;
	if (READ_LE_UINT16(pos) == 0x0084)
		sci_header = 2;

	pos += sci_header;
	if (pos[0] == 0xF0) {
		debug(1, "SysEx transfer = %d bytes", pos[1]);
		pos += 2;
		pos += 6;
	}
}

static void _readPartHeader(const byte* &pos, uint16 &off, uint16 &siz) {
	pos += 2;
	off = READ_LE_UINT16(pos);
	pos += 2;
	siz = READ_LE_UINT16(pos);
	pos += 2;
}

static void _skipPartHeader(const byte* &pos) {
	pos += 6;
}

static uint32 _availableSndTracks(const byte *data, uint32 size) {
	const byte *pos = data;

	uint32 sci_header;
	_readHeader(pos, sci_header);

	uint32 tracks = 0;
	while (pos[0] != 0xFF) {
		byte drv = *pos++;

		//debug(1, "(%d)", drv);

		while (pos[0] != 0xFF) {
			uint16 off, siz;
			_readPartHeader(pos, off, siz);
			off += sci_header;

			//debug(1, "%06d:%d ", off, siz);

			//debug(1, "Header bytes");
			//debug(1, "[%06X]  ", data[off]);
			//debug(1, "[%02X]  ", data[off+0]);
			//debug(1, "[%02X]  ", data[off+1]);

			bool digital_pcm = false;
			if (READ_LE_UINT16(&data[off]) == 0x00FE) {
				digital_pcm = true;
			}

			switch (drv) {
			case 0:	if (digital_pcm) {
					//debug(1, "- Soundblaster");
					tracks |= DIGITAL_PCM;
				} else {
					//debug(1, "- Adlib");
					tracks |= TRACK_ADLIB;
				}
				break;
			case 7:
				//debug(1, "- General MIDI");
				tracks |= TRACK_GM;
				break;
			case 9:
				//debug(1, "- CMS");
				tracks |= TRACK_CMS;
				break;
			case 12:
				//debug(1, "- MT-32");
				tracks |= TRACK_MT32;
				break;
			case 18:
				//debug(1, "- PC Speaker");
				tracks |= TRACK_PCSPK;
				break;
			case 19:
				//debug(1, "- Tandy 1000");
				tracks |= TRACK_TANDY;
				break;
			default:
				//debug(1, "- Unknown %d", drv);
				warning("Unknown music type %d", drv);
				break;
			}
		}

		pos++;
	}
	pos++;
	return tracks;
}


static byte _loadSndTrack(uint32 track, const byte** trackPtr, uint16* trackSiz, const byte *data, uint32 size) {
	byte matchDrv;
	switch (track) {
	case DIGITAL_PCM:
	case TRACK_ADLIB: matchDrv = 0;    break;
	case TRACK_GM:	  matchDrv = 7;    break;
	case TRACK_MT32:  matchDrv = 12;   break;
	default:			   return 0;
	}

	const byte *pos = data;

	uint32 sci_header;
	_readHeader(pos, sci_header);

	while (pos[0] != 0xFF) {
		byte drv = *pos++;

		byte part;
		const byte *ptr;

		part = 0;
		for (ptr = pos; *ptr != 0xFF; _skipPartHeader(ptr))
			part++;

		if (matchDrv == drv) {
			part = 0;
			while (pos[0] != 0xFF) {
				uint16 off, siz;
				_readPartHeader(pos, off, siz);
				off += sci_header;

				trackPtr[part] = data + off;
				trackSiz[part] = siz;
				part++;
			}
			debug(1, "- (%d) Play parts = %d", drv, part);
			return part;
		} else {
			pos = ptr;
		}
		pos++;
	}
	pos++;
	return 0;
}


Sound::Sound(Audio::Mixer *mixer, ResourceManager *resource, Decompressor *decompressor) :
	_mixer(mixer), _resource(resource), _decompressor(decompressor), _music(nullptr),
	_isMusicMuted(false), _isSfxMuted(false) {
	ARRAYCLEAR(_channels);
	_music = new SciMusic(true);
	_music->init();
}

Sound::~Sound() {
	unloadMusic();

	for (auto &data: _sfxData)
		delete [] data._data;

	delete _music;
}

void Sound::playAmigaSfx(const Common::String &filename, byte channel, byte volume) {
	if (!filename.hasSuffixIgnoreCase(".ins"))
		error("Unhandled SFX file type: %s", filename.c_str());

	Common::SeekableReadStream *sfxStream = _resource->getResource(filename);
	if (!sfxStream) {
		warning("SFX file %s not found", filename.c_str());
		return;
	}

	byte *dest = new byte[sfxStream->size()];
	sfxStream->read(dest, sfxStream->size());
	Common::MemoryReadStream *soundData = new Common::MemoryReadStream(dest, sfxStream->size(), DisposeAfterUse::YES);
	delete sfxStream;

	stopSfxForChannel(channel);

	Channel *ch = &_channels[channel];
	Audio::AudioStream *input = Audio::makeAIFFStream(soundData, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
}

void Sound::stopAllSfx() {
	_music->stopSFX();
	for (uint i = 0; i < ARRAYSIZE(_channels); i++)
		stopSfxForChannel(i);
}

void Sound::stopSfxForChannel(byte channel) {
	if (_mixer->isSoundHandleActive(_channels[channel].handle)) {
		_mixer->stopHandle(_channels[channel].handle);
		_channels[channel].stream = 0;
	}
}

bool Sound::playPCM(const byte *data, uint32 size) {
	_mixer->stopAll();

	if (!data)
		return false;

	const byte *trackPtr[0xFF];
	uint16 trackSiz[0xFF];
	byte numParts = _loadSndTrack(DIGITAL_PCM, trackPtr, trackSiz, data, size);
	if (numParts == 0)
		return false;

	for (byte part = 0; part < numParts; part++) {
		const byte *ptr = trackPtr[part];

		bool digital_pcm = false;
		if (READ_LE_UINT16(ptr) == 0x00FE) {
			digital_pcm = true;
		}
		ptr += 2;

		if (!digital_pcm)
			continue;

		uint16 rate, length, first, last;
		rate = READ_LE_UINT16(ptr);
		length = READ_LE_UINT16(ptr + 2);
		first = READ_LE_UINT16(ptr + 4);
		last = READ_LE_UINT16(ptr + 6);
		ptr += 8;

		ptr += first;
		debug(1, " - Digital PCM: %u Hz, [%u]=%u:%u",
			  rate, length, first, last);
		trackPtr[part] = ptr;
		trackSiz[part] = length;

		Channel *ch = &_channels[part];
		byte volume = 127;
		Audio::AudioStream *input = Audio::makeRawStream(trackPtr[part], trackSiz[part],
														 rate, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume, 0, DisposeAfterUse::YES);
	}
	return true;
}

static void _readStrings(Common::SeekableReadStream *stream) {
	uint16 count = stream->readUint16LE();
	debug(1, "        %u strs:", count);

	for (uint16 k = 0; k < count; k++) {
		uint16 idx = stream->readUint16LE();
		Common::String str = stream->readString();

		debug(1, "        %2u: %2u, \"%s\"", k, idx, str.c_str());
	}
}

bool Sound::loadSXSoundData(const Common::String &filename, Common::Array<SoundData> &dataArray, Common::HashMap<uint16, uint16> &idMap) {
	if (!filename.hasSuffixIgnoreCase(".sx"))
		error("Unhandled SX file type: %s", filename.c_str());

	Common::SeekableReadStream *resStream = _resource->getResource(filename);

	if (!resStream) {
		warning("SX file %s not found", filename.c_str());
		return false;
	}

	DgdsChunkReader chunk(resStream);

	while (chunk.readNextHeader(EX_SX, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(_decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();

		if (chunk.isSection(ID_INF)) {
			uint16 type = stream->readUint16LE();
			uint16 count = stream->readUint16LE();

			debug(1, "  SX INF %u [%u entries]:  (%s)", type, count, filename.c_str());
			for (uint16 k = 0; k < count; k++) {
				uint16 idx = stream->readUint16LE();
				debug(10, "        %2u: %u", k, idx);
				idMap[idx] = k;
			}
		} else if (chunk.isSection(ID_TAG) || chunk.isSection(ID_FNM)) {
			_readStrings(stream);
		} else if (chunk.isSection(ID_DAT)) {
			// TODO: Should we record the indexes?
			/*uint16 idx = */ stream->readUint16LE();
			/*uint16 type = */ stream->readUint16LE();
			SoundData soundData;
			soundData._data = _decompressor->decompress(stream, stream->size() - stream->pos(), soundData._size);
			dataArray.push_back(soundData);
		}
	}

	delete resStream;
	return true;
}

bool Sound::loadMusic(const Common::String &filename) {
	if (filename == _currentMusic)
		return false;

	unloadMusic();
	if (filename.hasSuffixIgnoreCase(".sx")) {
		loadSXSoundData(filename, _musicData, _musicIdMap);
	} else if (filename.hasSuffixIgnoreCase(".sng")) {
		_musicIdMap.clear();
		loadSNGSoundData(filename, _musicData);
	} else {
		error("Unhandled music file type: %s", filename.c_str());
	}

	_currentMusic = filename;
	debug(1, "Sound: Loaded music %s with %d entries", filename.c_str(), _musicData.size());
	return true;
}

void Sound::loadSFX(const Common::String &filename) {
	if (_sfxData.size())
		error("Sound: SFX data should only be loaded once");
	if (filename.hasSuffixIgnoreCase(".sx")) {
		loadSXSoundData(filename, _sfxData, _sfxIdMap);
	} else if (filename.hasSuffixIgnoreCase(".sng")) {
		loadSNGSoundData(filename, _sfxData);
	} else {
		error("Unhandled SFX file type: %s", filename.c_str());
	}

	debug(1, "Sound: Loaded sfx %s with %d entries", filename.c_str(), _sfxData.size());
}

void Sound::loadSNGSoundData(const Common::String &filename, Common::Array<SoundData> &dataArray) {
	if (!filename.hasSuffixIgnoreCase(".sng"))
		error("Unhandled SNG file type: %s", filename.c_str());

	Common::SeekableReadStream *resStream = _resource->getResource(filename);
	if (!resStream)
		error("Music file %s not found", filename.c_str());

	DgdsChunkReader chunk(resStream);

	uint16 songNum = 0;
	while (chunk.readNextHeader(EX_SNG, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(_decompressor);

		Common::SeekableReadStream *stream = chunk.getContent();
		if (chunk.isSection(ID_SNG)) {
			SoundData soundData;
			soundData._size = stream->size();
			byte *data = new byte[soundData._size];
			stream->read(data, soundData._size);
			patchSoundData(filename, songNum++, data, soundData._size);
			soundData._data = data;
			dataArray.push_back(soundData);
		} else if (chunk.isSection(ID_INF)) {
			uint32 count = stream->size() / 2;
			if (count > dataArray.size())
				error("Sound: %s has more flags in INF than SNG entries.", filename.c_str());
			debug(1, "  SNG INF [%u entries]", count);
			for (uint32 k = 0; k < count; k++) {
				uint16 flags = stream->readUint16LE();
				debug(10, "        %2u: 0x%04x", k, flags);
				dataArray[k]._flags = flags;
			}
		} else {
			warning("loadPCSound: skip unused chunk %s in %s", chunk.getIdStr(), filename.c_str());
		}
	}

	delete resStream;
}

void Sound::patchSoundData(const Common::String& filename, uint16 soundNumber, byte* data, uint32 size) {
	// TODO Can we check here if the game that's currently playing is Heart of China?
	if (filename.equalsC("SOUNDS1.SNG") && soundNumber == 59) {
		// Heart of China running water sound effect. This is broken on MT-32.
		// This is an original game bug.
		//
		// MT-32 sound effect MIDI data:
		// ...
		// 0x91 0x30 0x09	Note on 0x30 velocity 0x09
		// 0x05				Delta 5
		// 0x24 0x1B		Note on 0x24 velocity 0x1B (running status)
		// 0x10				Delta 16
		// 0x30 0x00		Note off 0x30 (running status)
		// 0x00				Delta 0
		// 0x24 0x00		Note off 0x24 (running status)
		// ...
		// The control channel sets the loop point at delta 11 and then ends.
		// Because the MT-32 sound effect track has not ended yet, playback
		// will continue and the notes will be turned off; the note offs are
		// then looped. This is fixed by replacing the delta 16 and first note
		// off by delta 6 and end of track (0xFC), which will end the sound
		// effect track at delta 11 as well. The sound effect will then loop
		// the last tick, effectively sustaining the two active notes.
		if (size > 0x3B && data[0x39] == 0x10 && data[0x3A] == 0x30 && data[0x3B] == 0x00) {
			data[0x39] = 0x06;
			data[0x3A] = 0xFC;
		}
		// The original interpreter expects the first 3 events of every track
		// to be program change, volume and panning. It will just read the
		// values of these events (bytes 0x2, 0x6 and 0x9), use them to
		// initialize the controllers, then start playback at offset 0xA.
		// ScummVM does not do this and instead starts playback at offset 0.
		// Usually this has the same effect. However, in this sound effect,
		// the MT-32 control channel has a reverb (0x50) control change instead
		// of a volume control change as the second event. This will cause
		// ScummVM to change the reverb instead of the volume. This should be
		// fixed by properly implementing the original behavior, but for now
		// the data is patched by changing the reverb controller to the volume
		// controller.
		if (size > 0x49 && data[0x47] == 0xBF && data[0x48] == 0x50 && data[0x49] == 0x7F) {
			data[0x48] = 0x07;
		}
	}
}

int Sound::mapSfxNum(int num) const {
	// Fixed offset in Dragon and HoC?
	if (DgdsEngine::getInstance()->getGameId() == GID_DRAGON || DgdsEngine::getInstance()->getGameId() == GID_HOC)
		return num - 24;
	else if (_sfxIdMap.contains(num))
		return _sfxIdMap[num];
	return num;
}

int Sound::mapMusicNum(int num) const {
	if (_musicIdMap.contains(num))
		return _musicIdMap[num];
	return num;
}

void Sound::playSFX(int num) {
	int mappedNum = mapSfxNum(num);
	debug(1, "Sound: Play SFX %d (-> %d), have %d entries", num, mappedNum, _sfxData.size());
	playPCSound(mappedNum, _sfxData, Audio::Mixer::kSFXSoundType);
}

void Sound::stopSfxByNum(int num) {
	int mappedNum = mapSfxNum(num);
	debug(1, "Sound: Stop SFX %d (-> %d)", num, mappedNum);

	MusicEntry *musicSlot = _music->getSlot(mappedNum + SND_RESOURCE_OFFSET);
	if (!musicSlot) {
		debug(1, "stopSfxByNum: Slot for sfx num %d not found.", mappedNum);
		return;
	}

	musicSlot->dataInc = 0;
	musicSlot->signal = SIGNAL_OFFSET;
	_music->soundStop(musicSlot);
}

void Sound::playMusic(int num) {
	int mappedNum = mapMusicNum(num);
	debug(1, "Sound: Play music %d (-> %d, %s), have %d entries", num, mappedNum, _currentMusic.c_str(), _musicData.size());
	playPCSound(mappedNum, _musicData, Audio::Mixer::kMusicSoundType);
}

void Sound::playMusicOrSFX(int num) {
	if (_musicIdMap.contains(num)) {
		playMusic(num);
	} else {
		playSFX(num);
	}
}

void Sound::stopMusicOrSFX(int num) {
	if (_musicIdMap.contains(num)) {
		stopMusic();
	} else {
		stopSfxByNum(num);
	}
}

void Sound::pauseMusicOrSFX(int num) {
	if (_musicIdMap.contains(num)) {
		// NOTE: We assume there is only ever one music here..
		_music->pauseMusic();
	} else {
		warning("Sound: TODO: Implement pause SFX %d", num);
	}
}

void Sound::unpauseMusicOrSFX(int num) {
	if (_musicIdMap.contains(num)) {
		// NOTE: We assume there is only ever one music here..
		_music->resumeMusic();
	} else {
		warning("Sound: TODO: Implement unpause SFX %d", num);
	}
}


void Sound::processInitSound(uint32 obj, const SoundData &data, Audio::Mixer::SoundType soundType) {
	// Check if a track with the same sound object is already playing
	MusicEntry *oldSound = _music->getSlot(obj);
	if (oldSound) {
		processDisposeSound(obj);
	}

	MusicEntry *newSound = new MusicEntry();
	newSound->resourceId = obj;
	newSound->soundObj = obj;
	newSound->loop = 0; // set in processPlaySound
	newSound->overridePriority = true;
	newSound->priority = 255;
	newSound->volume = MUSIC_VOLUME_DEFAULT;
	newSound->reverb = -1;	// initialize to SCI invalid, it'll be set correctly in soundInitSnd() below

	//debug(10, "processInitSound: %08x number %d, loop %d, prio %d, vol %d", obj,
	//		obj, newSound->loop, newSound->priority, newSound->volume);

	initSoundResource(newSound, data, soundType);

	_music->pushBackSlot(newSound);
}

void Sound::initSoundResource(MusicEntry *newSound, const SoundData &data, Audio::Mixer::SoundType soundType) {
	if (newSound->resourceId) {
		// Skip the header.
		const byte *dataPtr = data._data;
		uint32 hdrSize = 0;
		_readHeader(dataPtr, hdrSize);
		newSound->soundRes = new SoundResource(newSound->resourceId, dataPtr, data._size - hdrSize);
		if (!newSound->soundRes->exists()) {
			delete newSound->soundRes;
			newSound->soundRes = nullptr;
		}
	} else {
		newSound->soundRes = nullptr;
	}

	if (!newSound->isSample && newSound->soundRes)
		_music->soundInitSnd(newSound);

	newSound->soundType = soundType;
}

void Sound::processDisposeSound(uint32 obj) {
	// Mostly copied from SCI soundcmd.
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("processDisposeSound: Slot not found (%08x)", obj);
		return;
	}

	processStopSound(obj, false);

	_music->soundKill(musicSlot);
}

void Sound::processStopSound(uint32 obj, bool sampleFinishedPlaying) {
	// Mostly copied from SCI soundcmd.
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("processStopSound: Slot not found (%08x)", obj);
		return;
	}

	musicSlot->dataInc = 0;
	musicSlot->signal = SIGNAL_OFFSET;
	_music->soundStop(musicSlot);
}

void Sound::processPlaySound(uint32 obj, bool playBed, bool restoring, const SoundData &data) {
	// Mostly copied from SCI soundcmd.
	MusicEntry *musicSlot = _music->getSlot(obj);

	if (!musicSlot) {
		error("kDoSound(play): Slot not found (%08x)", obj);
	}

	int32 resourceId;
	if (!restoring)
		resourceId = obj;
	else
		// Handle cases where a game was saved while track A was playing, but track B was initialized, waiting to be played later.
		// In such cases, musicSlot->resourceId contains the actual track that was playing (A), while getSoundResourceId(obj)
		// contains the track that's waiting to be played later (B) - bug #10907.
		resourceId = musicSlot->resourceId;

	if (musicSlot->resourceId != resourceId) { // another sound loaded into struct
		processDisposeSound(obj);
		processInitSound(obj, data, Audio::Mixer::kSFXSoundType);
		// Find slot again :)
		musicSlot = _music->getSlot(obj);
	}

	assert(musicSlot);

	musicSlot->loop = (data._flags & FLAG_LOOP) ? 1 : 0;

	// Get song priority from either obj or soundRes
	byte resourcePriority = 0xFF;
	if (musicSlot->soundRes)
		resourcePriority = musicSlot->soundRes->getSoundPriority();
	if (!musicSlot->overridePriority && resourcePriority != 0xFF) {
		musicSlot->priority = resourcePriority;
	} else {
		// Set higher priority on music than sounds.
		musicSlot->priority = (musicSlot->soundType == Audio::Mixer::kMusicSoundType ? 255 : 127);
	}

	// Reset hold when starting a new song. kDoSoundSetHold is always called after
	// kDoSoundPlay to set it properly, if needed. Fixes bug #5851.
	musicSlot->hold = -1;
	musicSlot->playBed = playBed;
	musicSlot->volume = MUSIC_VOLUME_DEFAULT;

	debug(10, "processPlaySound: %08x number %d, sz %d, loop %d, prio %d, vol %d, bed %d", obj,
			resourceId, data._size, musicSlot->loop, musicSlot->priority, musicSlot->volume, playBed ? 1 : 0);

	_music->soundPlay(musicSlot, restoring);

	// Reset any left-over signals
	musicSlot->signal = 0;
	musicSlot->fadeStep = 0;
}

void Sound::playPCSound(int num, const Common::Array<SoundData> &dataArray, Audio::Mixer::SoundType soundType) {
	if (num >= 0 && num < (int)dataArray.size()) {
		const SoundData &data = dataArray[num];
		uint32 tracks = _availableSndTracks(data._data, data._size);
		if (tracks & DIGITAL_PCM) {
			playPCM(data._data, data._size);
		} else {
			int idOffset = soundType == Audio::Mixer::kSFXSoundType ? SND_RESOURCE_OFFSET : MUSIC_RESOURCE_OFFSET;
			int soundId = num + idOffset;

			// Only play one music at a time, don't play sfx if sfx muted.
			if (soundType == Audio::Mixer::kMusicSoundType) {
				MusicEntry *currentMusic = _music->getSlot(soundId);
				//
				// Don't change music if we are already playing the same track.  This happens
				// when walking through the house in Willy Beamish where all the rooms have
				// the same music track.
				//
				if (currentMusic && currentMusic->status == kSoundPlaying)
					return;
				stopMusic();
			} else if (soundType == Audio::Mixer::kSFXSoundType && _isSfxMuted) {
				return;
			}

			processInitSound(soundId, data, soundType);
			processPlaySound(soundId, false, false, data);

			// Immediately pause new music if muted
			if (_isMusicMuted && soundType == Audio::Mixer::kMusicSoundType)
				_music->pauseMusic();
		}
	} else {
		warning("Sound: Requested to play %d but only have %d tracks", num, dataArray.size());
	}
}

void Sound::stopMusic() {
	debug(1, "Sound: Stop music.");
	_music->stopMusic();
}

void Sound::muteSoundType(Audio::Mixer::SoundType soundType) {
	if (soundType == Audio::Mixer::kMusicSoundType) {
		_isMusicMuted = true;
		_music->pauseMusic();
	} else if (soundType == Audio::Mixer::kSFXSoundType) {
		_isSfxMuted = true;
		stopAllSfx();
	} else {
		error("Sound: Can only mute music or sfx, not sound type %d", soundType);
	}
}

void Sound::unmuteSoundType(Audio::Mixer::SoundType soundType) {
	if (soundType == Audio::Mixer::kMusicSoundType) {
		_isMusicMuted = false;
		_music->resumeMusic();
	} else if (soundType == Audio::Mixer::kSFXSoundType) {
		_isSfxMuted = false;
	} else {
		error("Sound: Can only unmute music or sfx, not sound type %d", soundType);
	}
}

void Sound::unloadMusic() {
	stopMusic();
	for (auto &data: _musicData)
		delete [] data._data;
	_musicData.clear();
	_currentMusic.clear();
	// Don't unload sfxData.
}


} // End of namespace Dgds

