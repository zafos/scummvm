
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

#include "common/formats/json.h"
#include "common/config-manager.h"
#include "twp/twp.h"
#include "twp/resmanager.h"
#include "twp/spritesheet.h"

namespace Twp {

static void parseSize(const Common::JSONObject &value, Math::Vector2d &v) {
	v.setX(value["w"]->asIntegerNumber());
	v.setY(value["h"]->asIntegerNumber());
}

static void parseRect(const Common::JSONObject &value, Common::Rect &rect) {
	rect.left = value["x"]->asIntegerNumber();
	rect.top = value["y"]->asIntegerNumber();
	rect.setWidth(value["w"]->asIntegerNumber());
	rect.setHeight(value["h"]->asIntegerNumber());
}

static void parseFrame(const Common::String &key, const Common::JSONObject &value, SpriteSheetFrame &frame) {
	frame.name = key;
	parseRect(value["frame"]->asObject(), frame.frame);
	parseRect(value["spriteSourceSize"]->asObject(), frame.spriteSourceSize);
	parseSize(value["sourceSize"]->asObject(), frame.sourceSize);
}

void SpriteSheet::parseSpriteSheet(const Common::String &contents) {
	Common::ScopedPtr<Common::JSONValue> json(Common::JSON::parse(contents.c_str()));
	const Common::JSONObject &obj = json->asObject()["frames"]->asObject();
	for (auto it = obj.begin(); it != obj.end(); it++) {
		parseFrame(it->_key, it->_value->asObject(), _frameTable[it->_key]);
	}

	const Common::JSONObject &jMeta = json->asObject()["meta"]->asObject();
	meta.image = jMeta["image"]->asString();
}

const SpriteSheetFrame &SpriteSheet::getFrame(const Common::String &key) const {
	if (key.hasSuffixIgnoreCase("_en")) {
		Common::String newKey(ResManager::getKey(key));
		if (_frameTable.contains(newKey))
			return _frameTable[newKey];
	}
	return _frameTable[key];
}

const SpriteSheetFrame *SpriteSheet::frame(const Common::String &key) const {
	if (key.hasSuffixIgnoreCase("_en")) {
		Common::String newKey(ResManager::getKey(key));
		if (_frameTable.contains(newKey))
			return &_frameTable[newKey];
	}
	if (_frameTable.contains(key))
		return &_frameTable[key];
	return nullptr;
}

} // namespace Twp
