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

#ifndef DARKSEED_KOFONT_H
#define DARKSEED_KOFONT_H

#include "graphics/font.h"
#include "common/file.h"
#include "darkseed/gamefont.h"

namespace Darkseed {

class KoFont : public Graphics::Font {
private:
	Common::Array<Common::Array<uint8>> _fontDataSet1;
	Common::Array<Common::Array<uint8>> _fontDataSet2;
	Common::Array<Common::Array<uint8>> _fontDataSet3;
	Common::Array<Common::Array<uint8>> _fontDataSetOther;

	GameFont *_gameFont;

public:
	KoFont();
	~KoFont();

	int getFontHeight() const override;
	int getMaxCharWidth() const override;
	int getCharWidth(uint32 chr) const override;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

	static void extractKoIndexComponents(uint32 charIdx, uint16 *param_2, uint16 *param_3, uint16 *param_4);
	static bool isConsonant(uint32 charIdx);
	static Common::U32String getObjectString(const Common::U32String &object);
	static Common::U32String getTopicString(const Common::U32String &object);
	static Common::U32String getLinkingString(const Common::U32String &object);
	static Common::U32String getLocationString(const Common::U32String &object);
private:
	void loadFontDataSet(Common::Array<Common::Array<uint8>> &dataSet, int size, int packedGlyphSize, Common::File &file);
	void loadFontGlyph(Common::Array<uint8> &pixels, int packedGlyphSize, Common::File &file);

	void createGlyph(uint8 *pixels, uint32 chr) const;
	void addToGlyph(uint8 *destPixels, int16 index) const;
	void addPixels(uint8 *destPixels, const Common::Array<uint8> &pixels) const;
	int getOtherCharIdx(uint32 chr) const;
	void drawOtherGlyph(Graphics::Surface *dst, uint8 chr, int x, int y) const;
};

} // namespace Darkseed

#endif // DARKSEED_KOFONT_H
