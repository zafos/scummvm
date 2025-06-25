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

#include "mediastation/datafile.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

void ParameterReadStream::readAndVerifyType(DatumType type) {
	DatumType actualType = static_cast<DatumType>(readUint16LE());
	if (actualType != type) {
		error("Expected datum type %d, got %d (@0x%llx)", type, actualType, static_cast<long long int>(pos()));
	}
}

byte ParameterReadStream::readTypedByte() {
	readAndVerifyType(kDatumTypeUint8);
	return readByte();
}

uint16 ParameterReadStream::readTypedUint16() {
	readAndVerifyType(kDatumTypeUint16);
	return readUint16LE();
}

uint32 ParameterReadStream::readTypedUint32() {
	readAndVerifyType(kDatumTypeUint32);
	return readUint32LE();
}

int8 ParameterReadStream::readTypedSByte() {
	readAndVerifyType(kDatumTypeInt8);
	return readSByte();
}

int16 ParameterReadStream::readTypedSint16() {
	readAndVerifyType(kDatumTypeInt16);
	return readSint16LE();
}

int32 ParameterReadStream::readTypedSint32() {
	readAndVerifyType(kDatumTypeInt32);
	return readSint32LE();
}

float ParameterReadStream::readTypedFloat() {
	readAndVerifyType(kDatumTypeFloat);
	return readFloatLE();
}

double ParameterReadStream::readTypedDouble() {
	readAndVerifyType(kDatumTypeDouble);
	return readDoubleLE();
}

Common::String ParameterReadStream::readTypedFilename() {
	readAndVerifyType(kDatumTypeFilename);
	uint size = readTypedUint32();
	return readString('\0', size);
}

Common::Rect ParameterReadStream::readTypedRect() {
	readAndVerifyType(kDatumTypeRect);
	Common::Point leftTop = readTypedPoint();
	Common::Point dimensions = readTypedGraphicSize();
	return Common::Rect(leftTop, dimensions.x, dimensions.y);
}

Common::Point ParameterReadStream::readTypedPoint() {
	readAndVerifyType(kDatumTypePoint);
	int16 x = readTypedGraphicUnit();
	int16 y = readTypedGraphicUnit();
	return Common::Point(x, y);
}

Common::Point ParameterReadStream::readTypedGraphicSize() {
	readAndVerifyType(kDatumTypeGraphicSize);
	int16 width = readTypedGraphicUnit();
	int16 height = readTypedGraphicUnit();
	return Common::Point(width, height);
}

int16 ParameterReadStream::readTypedGraphicUnit() {
	readAndVerifyType(kDatumTypeGraphicUnit);
	return readSint16LE();
}

double ParameterReadStream::readTypedTime() {
	readAndVerifyType(kDatumTypeTime);
	return readDoubleLE();
}

Common::String ParameterReadStream::readTypedString() {
	readAndVerifyType(kDatumTypeString);
	uint size = readTypedUint32();
	return readString('\0', size);
}

VersionInfo ParameterReadStream::readTypedVersion() {
	readAndVerifyType(kDatumTypeVersion);
	VersionInfo version;
	version.major = readTypedUint16();
	version.minor = readTypedUint16();
	version.patch = readTypedUint16();
	return version;
}

uint32 ParameterReadStream::readTypedChunkReference() {
	readAndVerifyType(kDatumTypeChunkReference);
	// This one is always BE.
	return readUint32BE();
}

Polygon ParameterReadStream::readTypedPolygon() {
	Polygon polygon;
	uint totalPoints = readTypedUint16();
	for (uint i = 0; i < totalPoints; ++i) {
		Common::Point point = readTypedGraphicSize();
		polygon.push_back(point);
	}
	return polygon;
}

Chunk::Chunk(Common::SeekableReadStream *stream) : _parentStream(stream) {
	_id = _parentStream->readUint32BE();
	_length = _parentStream->readUint32LE();
	_dataStartOffset = pos();
	_dataEndOffset = _dataStartOffset + _length;
	debugC(5, kDebugLoading, "Chunk::Chunk(): Got chunk with ID \"%s\" and size 0x%x", tag2str(_id), _length);
	if (_length == 0)
		error("Encountered a zero-length chunk. This usually indicates corrupted data - maybe a CD-ROM read error.");
}

uint32 Chunk::bytesRemaining() {
	return _dataEndOffset - pos();
}

uint32 Chunk::read(void *dataPtr, uint32 dataSize) {
	if (pos() > _dataEndOffset) {
		uint overrun = pos() - _dataEndOffset;
		error("Attempted to read 0x%x bytes at a location 0x%x bytes past end of chunk (@0x%llx)", dataSize, overrun, static_cast<long long int>(pos()));
	} else {
		return _parentStream->read(dataPtr, dataSize);
	}
}

bool Chunk::seek(int64 offset, int whence) {
	bool result = _parentStream->seek(offset, whence);
	if (result == false)
		return false;

	if (pos() < _dataStartOffset) {
		uint overrun = _dataStartOffset - offset;
		error("Attempted to seek 0x%x bytes before start of chunk (@0x%llx)", overrun, static_cast<long long int>(pos()));
	} else if (pos() > _dataEndOffset) {
		uint overrun = offset - _dataEndOffset;
		error("Attempted to seek 0x%x bytes past end of chunk (@0x%llx)", overrun, static_cast<long long int>(pos()));
	}
	return true;
}

Subfile::Subfile(Common::SeekableReadStream *stream) : _stream(stream) {
	// Verify file signature.
	debugC(5, kDebugLoading, "\n*** Subfile::Subfile(): Got new subfile (@0x%llx) ***", static_cast<long long int>(_stream->pos()));
	_rootChunk = nextChunk();
	if (_rootChunk._id != MKTAG('R', 'I', 'F', 'F'))
		error("Subfile::Subfile(): Expected \"RIFF\" chunk, got %s (@0x%llx)", tag2str(_rootChunk._id), static_cast<long long int>(_stream->pos()));
	_stream->skip(4); // IMTS

	// Read the RATE chunk.
	// This chunk should  always contain just one piece of data,
	// the "rate" (whatever that is). Usually it is zero.
	// TODO: Figure out what this actually is.
	Chunk rateChunk = nextChunk();
	if (rateChunk._id != MKTAG('r', 'a', 't', 'e'))
		error("Subfile::Subfile(): Expected \"rate\" chunk, got %s (@0x%llx)", tag2str(_rootChunk._id), static_cast<long long int>(_stream->pos()));
	_rate = _stream->readUint32LE();

	// Queue up the first data chunk.
	// First, we need to read past the LIST chunk.
	nextChunk();
	if (_stream->readUint32BE() != MKTAG('d', 'a', 't', 'a'))
		error("Subfile::Subfile(): Expected \"data\" as first bytes of subfile, got %s @0x%llx)", tag2str(rateChunk._id), static_cast<long long int>(_stream->pos()));
}

Chunk Subfile::nextChunk() {
	// Chunks always start on even-indexed bytes.
	if (_stream->pos() & 1)
		_stream->skip(1);

	_currentChunk = Chunk(_stream);
	return _currentChunk;
}

bool Subfile::atEnd() {
	// TODO: Is this the best place to put this and approach to use?
	return _rootChunk.bytesRemaining() == 0;
}

Datafile::Datafile(const Common::Path &path) {
	if (!open(path)) {
		error("Datafile::Datafile(): Failed to open %s", path.toString().c_str());
	}
}

Subfile Datafile::getNextSubfile() {
	return Subfile(_handle);
}

} // End of namespace MediaStation
