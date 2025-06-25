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


#include "common/config-manager.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/ptr.h"

#include "dgds/sound/resource/sci_resource.h"
#include "dgds/dgds.h"
#include "dgds/includes.h"

namespace Dgds {

//
// Unlike the other files in this directory, this is not part of the SCI
// engine.  This is a single function to load patch data from the DGDS
// resource system.
//

static const char *PATCH_RESOURCES[] = {
	"SXTITLE.OVL", // dragon
	"SXCODE1.OVL", // hoc (TODO: when do we load SXCODE2 - maybe when global 0x37 changes)
	"SX.OVL",      // newer games (beamish, sq5 demo)
};

SciResource *getMidiPatchData(int num) {
	assert(num < 999);

	DgdsEngine *engine = DgdsEngine::getInstance();
	ResourceManager *resource = engine->getResourceManager();
	Decompressor *decomp = engine->getDecompressor();
	Common::ScopedPtr<ResourceManager> fddMgr;
	Common::ScopedPtr<Common::SeekableReadStream> ovlStream;

	int resNum = 0;
	for (; resNum < ARRAYSIZE(PATCH_RESOURCES); resNum++) {
		ovlStream.reset(resource->getResource(PATCH_RESOURCES[resNum]));
		if (ovlStream)
			break;
	}

	//
	// WORKAROUND: The MT-32 patch data in Willy Beamish CD version is corrupted.
	// If the FDD version is avaialble in the "FDD" directory, use that instead.
	// This is how the data comes arranged in the GOG version.
	//
	if (num == 1 && engine->getGameId() == GID_WILLY) {
		fddMgr.reset(new ResourceManager("FDD"));
		if (fddMgr.get()->hasResource("SX.OVL")) {
			debug("Overriding MT32 patch data with patches from FDD version.");
			resNum = 2;
			ovlStream.reset(fddMgr.get()->getResource("SX.OVL"));
		}
	}

	if (!ovlStream) {
		warning("Couldn't load DGDS midi patch data from any known OVL file");
		return nullptr;
	}

	DgdsChunkReader chunk(ovlStream.get());

	const Common::String targetSection = Common::String::format("%03d:", num);

	while (chunk.readNextHeader(EX_OVL, PATCH_RESOURCES[resNum])) {
		if (chunk.isContainer()) {
			continue;
		}

		if (chunk.isSection(targetSection)) {
			chunk.readContent(decomp);
			Common::SeekableReadStream *stream = chunk.getContent();

			byte magic = stream->readSByte(); // always 137?
			byte strLen = stream->readSByte(); // header string len
			char *buf = new char[strLen + 1];
			stream->read(buf, strLen);
			buf[strLen] = '\0';

			int dataLen = stream->size() - (strLen + 2);
			byte *data = new byte[dataLen];
			debug(1, "midi patch %s loading magic %d str '%s'", targetSection.c_str(), magic, buf);
			delete [] buf;
			stream->read(data, dataLen);
			return new SciResource(data, dataLen, num);
		} else {
			chunk.skipContent();
		}
	}

	warning("Didn't find section %s in midi patch resource %s", targetSection.c_str(), PATCH_RESOURCES[resNum]);

	return nullptr;
}


} // end namespace Dgds
