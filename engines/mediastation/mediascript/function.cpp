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

#include "mediastation/mediascript/function.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

Function::Function(Chunk &chunk) {
	_fileId = chunk.readTypedUint16();
	// In PROFILE._ST (only present in some titles), the function ID is reported
	// with 19900 added, so function 100 would be reported as 20000. But in
	// bytecode, the zero-based ID is used, so that's what we'll store here.
	_id = chunk.readTypedUint16();
	_code = new CodeChunk(chunk);
}

Function::~Function() {
	delete _code;
	_code = nullptr;
}

ScriptValue Function::execute(Common::Array<ScriptValue> &args) {
	debugC(5, kDebugScript, "\n********** FUNCTION %d **********", _id);
	ScriptValue returnValue = _code->execute(&args);
	debugC(5, kDebugScript, "********** END FUNCTION **********");
	return returnValue;
}

} // End of namespace MediaStation
