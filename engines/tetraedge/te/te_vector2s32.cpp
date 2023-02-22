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

#include "common/stream.h"

#include "tetraedge/te/te_vector2s32.h"

namespace Tetraedge {

TeVector2s32::TeVector2s32() : _x(0), _y(0) {
}

/*static*/
void TeVector2s32::deserialize(Common::ReadStream &stream, TeVector2s32 &dest) {
	dest._x = stream.readSint32LE();
	dest._y = stream.readSint32LE();
}

TeVector2s32 operator+(const TeVector2s32 &left, const TeVector2s32 &right) {
		return TeVector2s32(left._x + right._x, left._y + right._y);
}

} // end namespace Tetraedge
