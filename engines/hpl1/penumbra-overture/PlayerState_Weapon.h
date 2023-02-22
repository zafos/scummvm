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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of Penumbra Overture.
 */

#ifndef GAME_PLAYER_STATE_WEAPON_H
#define GAME_PLAYER_STATE_WEAPON_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/PlayerState.h"

using namespace hpl;

//-----------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// THROW STATE
//////////////////////////////////////////////////////////////////////////

class cHudModel_Throw;

class cPlayerState_Throw : public iPlayerState {
public:
	cPlayerState_Throw(cInit *apInit, cPlayer *apPlayer);

	void OnUpdate(float afTimeStep);

	void OnDraw();
	void OnPostSceneDraw();

	bool OnJump();

	void OnStartInteractMode();

	void OnStartInteract();
	void OnStopInteract();
	void OnStartExamine();
	void OnStartHolster();

	bool OnAddYaw(float afVal);
	bool OnAddPitch(float afVal);

	bool OnMoveForwards(float afMul, float afTimeStep);
	bool OnMoveSideways(float afMul, float afTimeStep);

	void EnterState(iPlayerState *apPrevState);
	void LeaveState(iPlayerState *apNextState);

	void OnStartRun();
	void OnStopRun();

	void OnStartCrouch();
	void OnStopCrouch();

	bool OnStartInventory();
	bool OnStartInventoryShortCut(int alNum);

	void SetHudObject(cHudModel_Throw *apHudObject) { mpHudObject = apHudObject; }

private:
	cHudModel_Throw *mpHudObject;
	float mfLastForward;
	float mfLastSideways;
};

//-----------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// WEAPON MELEE STATE
//////////////////////////////////////////////////////////////////////////

class cHudModel_WeaponMelee;

class cPlayerState_WeaponMelee : public iPlayerState {
public:
	cPlayerState_WeaponMelee(cInit *apInit, cPlayer *apPlayer);

	void OnUpdate(float afTimeStep);

	void OnDraw();
	void OnPostSceneDraw();

	bool OnJump();

	void OnStartInteractMode();

	void OnStartInteract();
	void OnStopInteract();
	void OnStartExamine();
	void OnStartHolster();

	bool OnAddYaw(float afVal);
	bool OnAddPitch(float afVal);

	bool OnMoveForwards(float afMul, float afTimeStep);
	bool OnMoveSideways(float afMul, float afTimeStep);

	void EnterState(iPlayerState *apPrevState);
	void LeaveState(iPlayerState *apNextState);

	void OnStartRun();
	void OnStopRun();

	void OnStartCrouch();
	void OnStopCrouch();

	bool OnStartInventory();
	bool OnStartInventoryShortCut(int alNum);

	void SetHudWeapon(cHudModel_WeaponMelee *apHudWeapon) { mpHudWeapon = apHudWeapon; }

private:
	cHudModel_WeaponMelee *mpHudWeapon;
	float mfLastForward;
	float mfLastSideways;
};

//-----------------------------------------------------------------

#endif // GAME_PLAYER_STATE_WEAPON_H
