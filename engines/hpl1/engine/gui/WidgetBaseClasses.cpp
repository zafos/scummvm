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

#include "hpl1/engine/gui/WidgetBaseClasses.h"

#include "hpl1/engine/system/low_level_system.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/gui/GuiGfxElement.h"
#include "hpl1/engine/gui/GuiSet.h"
#include "hpl1/engine/gui/GuiSkin.h"
#include "hpl1/engine/gui/Widget.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// ITEM CONTAINER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void iWidgetItemContainer::AddItem(const tWString &asItem) {
	mvItems.push_back(asItem);

	UpdateProperties();
}

void iWidgetItemContainer::RemoveItem(int alX) {
	int lCount = 0;
	tWStringVecIt it = mvItems.begin();
	for (; it != mvItems.end(); ++it, ++lCount) {
		if (lCount == alX) {
			mvItems.erase(it);
			break;
		}
	}

	UpdateProperties();
}

void iWidgetItemContainer::RemoveItem(const tWString &asItem) {
	tWStringVecIt it = mvItems.begin();
	for (; it != mvItems.end(); ++it) {
		if (*it == asItem) {
			mvItems.erase(it);
			break;
		}
	}
}

//-----------------------------------------------------------------------

const tWString &iWidgetItemContainer::GetItem(int alX) const {
	return mvItems[alX];
}
void iWidgetItemContainer::SetItem(int alX, const tWString &asText) {
	mvItems[alX] = asText;
}

int iWidgetItemContainer::GetItemNum() const {
	return (int)mvItems.size();
}
bool iWidgetItemContainer::HasItem(const tWString &asItem) {
	tWStringVecIt it = mvItems.begin();
	for (; it != mvItems.end(); ++it) {
		if (*it == asItem)
			return true;
	}
	return false;
}

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

} // namespace hpl
