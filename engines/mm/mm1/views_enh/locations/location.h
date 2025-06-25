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

#ifndef MM1_VIEWS_ENH_LOCATIONS_LOCATION_H
#define MM1_VIEWS_ENH_LOCATIONS_LOCATION_H

#include "mm/mm1/views_enh/party_view.h"
#include "mm/shared/xeen/sprites.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

class Location : public PartyView {
protected:
	Shared::Xeen::SpriteResource _escSprite;
	int _locationId = -1;

protected:
	/**
	 * Called when an active timeout countdown expired
	 */
	void timeout() override {
		leave();
	}

	/**
	 * Subtract gold from current character
	 */
	bool subtractGold(uint amount);

	/**
	 * Display a message
	 */
	void displayMessage(const Common::String &msg);

	/**
	 * Displays not enough gold
	 */
	void notEnoughGold();

	/**
	 * Displays backpack is full
	 */
	void backpackFull();

public:
	Location(const Common::String &name, int locationId);

	/**
	 * Game message handler
	 */
	bool msgGame(const GameMessage &msg) override;

	/**
	 * Called when a message is finished being shown
	 */
	virtual void messageShown() {
		leave();
	}

	/**
	 * Leave the location, turning around
	 */
	void leave();

	bool msgUnfocus(const UnfocusMessage &msg) override;

	/**
	 * Draw the location
	 */
	void draw() override;

	/**
	 * Tick handler
	 */
	bool tick() override;
};

} // namespace Locations
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
