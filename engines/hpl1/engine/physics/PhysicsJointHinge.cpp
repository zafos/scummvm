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
 * This file is part of HPL1 Engine.
 */

#include "hpl1/engine/physics/PhysicsJointHinge.h"

#include "hpl1/engine/game/Game.h"
#include "hpl1/engine/scene/Scene.h"
#include "hpl1/engine/scene/World3D.h"

#include "hpl1/engine/physics/PhysicsBody.h"
#include "hpl1/engine/physics/PhysicsWorld.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cSaveData_iPhysicsJointHinge, cSaveData_iPhysicsJoint)
	kSerializeVar(mfMaxAngle, eSerializeType_Float32)
		kSerializeVar(mfMinAngle, eSerializeType_Float32)
			kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_iPhysicsJointHinge::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	iPhysicsWorld *apWorld = apGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	cMatrixf mtxChildTemp, mtxParentTemp;

	iPhysicsBody *pChildBody = static_cast<iPhysicsBody *>(apSaveObjectHandler->Get(mlChildBodyId));
	if (pChildBody == NULL)
		return NULL;

	iPhysicsBody *pParentBody = NULL;
	if (mlParentBodyId > 0)
		pParentBody = static_cast<iPhysicsBody *>(apSaveObjectHandler->Get(mlParentBodyId));

	mtxChildTemp = pChildBody->GetLocalMatrix();
	if (pParentBody)
		mtxParentTemp = pParentBody->GetLocalMatrix();

	pChildBody->SetMatrix(m_mtxChildBodySetup);
	if (pParentBody)
		pParentBody->SetMatrix(m_mtxParentBodySetup);

	iPhysicsJointHinge *pJoint = apWorld->CreateJointHinge(msName, mvStartPivotPoint, mvPinDir, pParentBody, pChildBody);

	pChildBody->SetMatrix(mtxChildTemp);
	if (pParentBody)
		pParentBody->SetMatrix(mtxParentTemp);

	return pJoint;
}

//-----------------------------------------------------------------------

int cSaveData_iPhysicsJointHinge::GetSaveCreatePrio() {
	return 1;
}

//-----------------------------------------------------------------------

iSaveData *iPhysicsJointHinge::CreateSaveData() {
	return hplNew(cSaveData_iPhysicsJointHinge, ());
}

//-----------------------------------------------------------------------

void iPhysicsJointHinge::SaveToSaveData(iSaveData *apSaveData) {
	kSaveData_SaveToBegin(iPhysicsJointHinge);

	kSaveData_SaveTo(mfMaxAngle);
	kSaveData_SaveTo(mfMinAngle);
}

//-----------------------------------------------------------------------

void iPhysicsJointHinge::LoadFromSaveData(iSaveData *apSaveData) {
	kSaveData_LoadFromBegin(iPhysicsJointHinge);

	kSaveData_LoadFrom(mfMaxAngle);
	kSaveData_LoadFrom(mfMinAngle);
}

//-----------------------------------------------------------------------

void iPhysicsJointHinge::SaveDataSetup(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	kSaveData_SetupBegin(iPhysicsJointHinge);
}

//-----------------------------------------------------------------------
} // namespace hpl
