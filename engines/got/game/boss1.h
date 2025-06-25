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

#ifndef GOT_GAME_BOSS1_H
#define GOT_GAME_BOSS1_H

#include "got/data/defines.h"

namespace Got {

// Boss 1 - Snake (Jormangund)
extern int boss1Movement(Actor *actor);
extern void boss1CheckHit(const Actor *actor, int x1, int y1, int x2, int y2, int actorNum);
extern void boss1SetupLevel();
extern void boss1ClosingSequence1();
extern void boss1ClosingSequence2();
extern void boss1ClosingSequence3();
extern void boss1ClosingSequence4();

} // namespace Got

#endif
