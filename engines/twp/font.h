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

#ifndef TWP_FONT_H
#define TWP_FONT_H

#include "common/rect.h"
#include "common/hashmap.h"
#include "common/str.h"
#include "twp/spritesheet.h"
#include "twp/gfx.h"

namespace Common {
template<>
struct Hash<Common::u32char_type_t> : public UnaryFunction<Common::u32char_type_t, uint> {
	uint operator()(Common::u32char_type_t val) const { return (uint)val; }
};
} // namespace Common

namespace Twp {

struct KerningKey {
	int first;
	int second;
};
bool operator==(const KerningKey &l, const KerningKey &r);

} // namespace Twp

namespace Common {

template<>
struct Hash<Twp::KerningKey> : public Common::UnaryFunction<Twp::KerningKey, uint> {
	uint operator()(Twp::KerningKey val) const { return (uint)(val.first ^ val.second); }
};

} // namespace Common

namespace Twp {
typedef Common::u32char_type_t CodePoint;

// represents a glyph: a part of an image for a specific font character
struct Glyph {
	int advance;              // Offset to move horizontally to the next character.
	Common::Rect bounds;      // Bounding rectangle of the glyph, in coordinates relative to the baseline.
	Common::Rect textureRect; // Texture coordinates of the glyph inside the font's texture.
};

class Font {
public:
	virtual ~Font() = default;

	virtual int getLineHeight() = 0;
	virtual float getKerning(CodePoint prev, CodePoint next) = 0;
	virtual Glyph getGlyph(CodePoint chr) = 0;
	virtual Common::String getName() = 0;
};

// Represents a bitmap font from a spritesheet.
class GGFont : public Font {
public:
	~GGFont() override;
	void load(const Common::String &path);

	int getLineHeight() final { return _lineHeight; }
	float getKerning(CodePoint prev, CodePoint next) final { return 0.f; }
	Glyph getGlyph(CodePoint chr) final;
	Common::String getName() final { return _name; }

private:
	Common::HashMap<CodePoint, Glyph> _glyphs;
	int _lineHeight = 0;
	Common::String _name;
};

struct Char {
	int id = 0;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	int xoff = 0;
	int yoff = 0;
	int xadv = 0;
	int page = 0;
	int chnl = 0;
	Common::String _letter;
};

class BmFont : public Font {
public:
	~BmFont() override;
	void load(const Common::String &path);

	int getLineHeight() final { return _lnHeight; }
	float getKerning(CodePoint prev, CodePoint next) final;
	Glyph getGlyph(CodePoint chr) final;
	Common::String getName() final { return _name; }

private:
	Common::HashMap<CodePoint, Glyph> _glyphs;
	Common::HashMap<KerningKey, float> _kernings;
	int _lnHeight = 0;
	int _base = 0;
	int _scaleW = 0;
	int _scaleH = 0;
	int _pages = 0;
	int _packed = 0;
	Common::String _name;
};

enum TextHAlignment {
	thLeft,
	thCenter,
	thRight
};
enum TextVAlignment {
	tvTop,
	tvCenter,
	tvBottom
};

// This class allows to render a text.
//
// A text can contains color in hexadecimal with this format: #RRGGBB
class Text {
public:
	Text();
	Text(const Common::String &fontName, const Common::String &text, TextHAlignment hAlign = thCenter, TextVAlignment vAlign = tvCenter, float maxWidth = 0.0f, const Color &color = Color());

	void setText(const Common::String &text) {
		_txt = text;
		_dirty = true;
	}
	Common::String getText() const { return _txt; }

	void setColor(const Color &c) {
		_col = c;
		_dirty = true;
	}
	Color getColor() const { return _col; }

	void setMaxWidth(float maxW) {
		_maxW = maxW;
		_dirty = true;
	}
	float getMaxWidth() const { return _maxW; }

	void setHAlign(TextHAlignment align) {
		_hAlign = align;
		_dirty = true;
	}
	TextHAlignment getHAlign() const { return _hAlign; }

	void setVAlign(TextVAlignment align) {
		_vAlign = align;
		_dirty = true;
	}
	TextVAlignment getVAlign() const { return _vAlign; }

	void setFont(const Common::String &fontName);
	Common::SharedPtr<Font> getFont() { return _font; }
	Math::Vector2d getBounds();

	void draw(Gfx &gfx, const Math::Matrix4 &trsf = Math::Matrix4());

private:
	void update();

private:
	Common::SharedPtr<Font> _font;
	Common::String _fontName;
	Texture *_texture = nullptr;
	Common::String _txt;
	Color _col;
	TextHAlignment _hAlign = TextHAlignment::thLeft;
	TextVAlignment _vAlign = TextVAlignment::tvCenter;
	Common::Array<Vertex> _vertices;
	Math::Vector2d _bnds;
	float _maxW = 0.f;
	Common::Array<Common::Rect> _quads;
	bool _dirty = false;
};

} // namespace Twp

#endif
