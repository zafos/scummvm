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

/********************************************
   DISCLAIMER:

   This is a wrapper code to mimic the relevant std:: class
   Please use it ONLY when porting an existing code e.g. from the original sources

   For all new development please use classes from Common::
 *********************************************/

#ifndef COMMON_STD_UTILITY_H
#define COMMON_STD_UTILITY_H

#include "common/util.h"

namespace Std {

template<class T1, class T2>
struct pair {
	T1 first;
	T2 second;

	pair() {
	}
	pair(T1 first_, T2 second_) : first(first_), second(second_) {
	}
};

template< class T1, class T2 >
pair<T1, T2> make_pair(T1 first, T2 second) {
	return pair<T1, T2>(first, second);
}

using Common::move;

} // namespace Std

#endif
