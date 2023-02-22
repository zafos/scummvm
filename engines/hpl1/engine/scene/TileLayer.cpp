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

#include "hpl1/engine/scene/TileLayer.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cTileLayer::cTileLayer(unsigned int alW, unsigned int alH, bool abCollision, bool abLit,
					   eTileLayerType aType, float afZ) {
	mvSize = cVector2l(alW, alH);
	mbCollision = abCollision;
	mbLit = abLit;
	mType = aType;
	mfZ = afZ;

	mvTile.resize(alW * alH);
	Common::fill(mvTile.begin(), mvTile.end(), nullptr);
}

//-----------------------------------------------------------------------

cTileLayer::~cTileLayer() {
	Log(" Deleting tilelayer.\n");

	for (int i = 0; i < (int)mvTile.size(); i++)
		if (mvTile[i])
			hplDelete(mvTile[i]);

	mvTile.clear();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cTileLayer::SetTile(unsigned int alX, unsigned int alY, cTile *aVal) {
	int lNum = alY * mvSize.x + alX;
	if (lNum >= (int)mvTile.size())
		return false;

	if (aVal == NULL) {
		if (mvTile[lNum])
			hplDelete(mvTile[lNum]);
		mvTile[lNum] = NULL;
	} else {
		if (mvTile[lNum] == NULL)
			mvTile[lNum] = aVal;
		else
			*mvTile[lNum] = *aVal;
	}

	return true;
}

//-----------------------------------------------------------------------

cTile *cTileLayer::GetAt(int alX, int alY) {
	if (alX < 0 || alX >= mvSize.x || alY < 0 || alY >= mvSize.y)
		return NULL;

	int lNum = alY * mvSize.x + alX;
	if (lNum >= (int)mvTile.size())
		return NULL;

	return mvTile[lNum];
}

//-----------------------------------------------------------------------

cTile *cTileLayer::GetAt(int alNum) {
	return mvTile[alNum];
}

//-----------------------------------------------------------------------

} // namespace hpl
