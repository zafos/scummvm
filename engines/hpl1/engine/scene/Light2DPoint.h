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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_LIGHTPOINT2D_H
#define HPL_LIGHTPOINT2D_H

#include "hpl1/engine/scene/Light2D.h"

namespace hpl {

class cLight2DPoint : public iLight2D {
public:
	cLight2DPoint(tString asName);
	~cLight2DPoint();

	const cRect2f &GetBoundingBox();
	bool UpdateBoundingBox();

	int Render(iLowLevelGraphics *apLowLevel, int alFirstIndex);
};

}     // namespace hpl

#endif // HPL_LIGHTPOINT2D_H
