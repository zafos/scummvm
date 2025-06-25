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

#ifndef MEDIASTATION_HOTSPOT_H
#define MEDIASTATION_HOTSPOT_H

#include "mediastation/asset.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

class Hotspot : public SpatialEntity {
public:
	Hotspot() : SpatialEntity(kAssetTypeHotspot) {};
	virtual ~Hotspot() { _mouseActiveArea.clear(); }

	bool isInside(const Common::Point &pointToCheck);
	virtual bool isVisible() const override { return false; }
	bool isActive() const { return _isActive; }

	virtual void readParameter(Chunk &chunk, AssetHeaderSectionType paramType) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;

	uint _cursorResourceId = 0;
	Common::Array<Common::Point> _mouseActiveArea;

private:
	bool _isActive = false;
};

} // End of namespace MediaStation

#endif