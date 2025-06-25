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

#include "mediastation/assets/canvas.h"

namespace MediaStation {

void Canvas::readParameter(Chunk &chunk, AssetHeaderSectionType paramType) {
	switch (paramType) {
	case kAssetHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kAssetHeaderDissolveFactor:
		_dissolveFactor = chunk.readTypedDouble();
		break;

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue Canvas::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	switch (methodId) {
	case kClearToPaletteMethod: {
		error("Canvas::callMethod(): BuiltInFunction::clearToPalette is not implemented yet");
	}

	default:
		return SpatialEntity::callMethod(methodId, args);
	}
}

} // End of namespace MediaStation
