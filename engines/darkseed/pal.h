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

#ifndef DARKSEED_PAL_H
#define DARKSEED_PAL_H

#include "common/path.h"
#include "common/stream.h"

namespace Darkseed {

#define DARKSEED_NUM_PAL_ENTRIES 16
#define DARKSEED_PAL_SIZE (DARKSEED_NUM_PAL_ENTRIES * 3)


class Pal {
public:
	byte _palData[DARKSEED_PAL_SIZE];

	Pal() {};
	Pal(const Pal &pal);
	Pal & operator=(const Pal &pal);
	void loadFromScreen();
	void load(const Pal &pal);
	bool load(const Common::Path &filename, bool shouldInstallPalette = true);
	bool loadFromStream(Common::SeekableReadStream &readStream, bool shouldInstallPalette = true);
	void clear();
	void swapEntries(int idx1, int idx2);
	void updatePalette(int delta, const Pal &targetPal, bool shouldInstallPalette = true);
	void installPalette() const;
};

} // namespace Darkseed

#endif // DARKSEED_PAL_H
