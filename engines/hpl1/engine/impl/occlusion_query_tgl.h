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

#ifndef HPL_OCCLUSION_QUERY_TGL_H
#define HPL_OCCLUSION_QUERY_TGL_H

#include "hpl1/engine/graphics/OcclusionQuery.h"
#include "common/scummsys.h"

#ifdef USE_TINYGL

namespace hpl {

class OcclusionQueryTGL : public iOcclusionQuery {
public:
	OcclusionQueryTGL() {}
	~OcclusionQueryTGL() {}

	void Begin() override {}
	void End() override {}
	bool FetchResults() override {return true;}
	unsigned int GetSampleCount() override {return 0;}
};

}     // namespace hpl

#endif // USE_TINYGL

#endif // HPL_OCCLUSION_QUERY_TGL_H
