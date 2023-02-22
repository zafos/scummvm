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

#ifndef TETRAEDGE_TE_TE_TRS_H
#define TETRAEDGE_TE_TE_TRS_H

#include "common/stream.h"
#include "tetraedge/te/te_quaternion.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TeTRS {
public:
	TeTRS();

	static void deserialize(Common::ReadStream &stream, TeTRS &dest);
	static void serialize(Common::WriteStream &stream, const TeTRS &src);

	void setIdentity();

	void setRotation(const TeQuaternion &rot) {
		_rot = rot;
	}
	void setScale(const TeVector3f32 &scale) {
		_scale = scale;
	}
	void setTranslation(const TeVector3f32 &trans) {
		_trans = trans;
	}

	const TeVector3f32 &getTranslation() const { return _trans; }
	const TeQuaternion &getRotation() const { return _rot; }
	const TeVector3f32 &getScale() const { return _scale; }

	TeTRS lerp(const TeTRS &other, float amount);

private:
	TeVector3f32 _trans;
	TeQuaternion _rot;
	TeVector3f32 _scale;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_TRS_H
