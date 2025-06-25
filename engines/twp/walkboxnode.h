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

#ifndef TWP_WALKBOXNODE_H
#define TWP_WALKBOXNODE_H

#include "twp/scenegraph.h"

namespace Twp {

enum class WalkboxMode {
	None,
	Merged,
	All
};

class WalkboxNode : public Node {
public:
	WalkboxNode();

	void setMode(WalkboxMode mode) { _mode = mode; }
	WalkboxMode getMode() const { return _mode; }

private:
	virtual void drawCore(const Math::Matrix4 &trsf) override;

private:
	WalkboxMode _mode;
};

enum class PathMode {
	None,
	GraphMode,
	All
};

class PathNode : public Node {
public:
	PathNode();

	void setMode(PathMode mode) { _mode = mode; }
	PathMode getMode() const { return _mode; }

private:
	Math::Vector2d fixPos(const Math::Vector2d &pos);
	virtual void drawCore(const Math::Matrix4 &trsf) override;

private:
	PathMode _mode = PathMode::None;
};

class LightingNode : public Node {
public:
	LightingNode();

private:
	virtual void drawCore(const Math::Matrix4 &trsf) override;
};

} // namespace Twp

#endif
