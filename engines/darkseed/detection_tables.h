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

namespace Darkseed {

const PlainGameDescriptor darkseedGames[] = {
	{ "darkseed", "Darkseed" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"darkseed",
		nullptr,
		AD_ENTRY1s("TOS.EXE", "e20ca609f2acb623e0076ef2288673b2", 147016),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{ // version 1.2
		"darkseed",
		nullptr,
		AD_ENTRY1s("TOS.EXE", "9fad9d0a4b5e7d0657172593750c7b81", 214784),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"darkseed",
		nullptr,
		AD_ENTRY1s("TOS.EXE", "73854f950819beb0eb0d73bd52cc9030", 149412),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"darkseed",
		nullptr,
		AD_ENTRY1s("TOS.EXE", "ba87f00c3a51ca3e3bb218fc58f128eb", 121662),
		Common::ZH_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
    { // unpacked exe
		"darkseed",
		"unpacked",
		AD_ENTRY1s("TOS.EXE", "0fc2751aa16cac26ad3aa9d1cbbb5c7b", 209208),
		Common::ZH_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"darkseed",
		nullptr,
		AD_ENTRY1s("TOS.EXE", "62a636d3d6b19336d059cd2f8b1a365f", 154450),
		Common::KO_KOR,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"darkseed",
		"CD",
		AD_ENTRY1s("TOS.EXE", "679abf5829b2453d30b17caabafea168", 168432),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_CD,
		GUIO1(GAMEOPTION_FLOPPY_MUSIC)
	},
	{	// 1.51 according to DS.BAT, 1.5P according to intro
		"darkseed",
		"CD",
		AD_ENTRY1s("TOS.EXE", "afaeb490ef8e7625008867aa8f20c703", 168480),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_CD,
		GUIO1(GAMEOPTION_FLOPPY_MUSIC)
	},
	{
		"darkseed",
		"CD",
		AD_ENTRY1s("TOS.EXE", "57581682c29fc7d242b463210b6e54b4", 144422),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_CD,
		GUIO1(GAMEOPTION_FLOPPY_MUSIC)
	},
	{
		"darkseed",
		"CD",
		AD_ENTRY1s("TOS.EXE", "9b8cdd3b4268d18babf7629fca6a271e", 143534),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_CD,
		GUIO1(GAMEOPTION_FLOPPY_MUSIC)
	},
	{
		"darkseed",
		"CD",
		AD_ENTRY1s("TOS.EXE", "3c00f3e80fa2c40641278243f96b8398", 170944),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_CD,
		GUIO1(GAMEOPTION_FLOPPY_MUSIC)
	},
	{
		"darkseed",
		"CD",
		AD_ENTRY1s("DARKSEED.EXE", "27321d178a553c4dc17d1a2a601a9a6f", 1432140),
		Common::JA_JPN,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_CD,
		GUIO1(GUIO_NONE)
	},
	{
		"darkseed",
		"DEMO",
		AD_ENTRY1s("TOS.EXE", "43014b73a2cc549bd13d65b18a6aefe4", 161184),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING | ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},
	AD_TABLE_END_MARKER
};

} // End of namespace Darkseed
