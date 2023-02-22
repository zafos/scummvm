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

#ifndef HPL_MATERIAL_BASE_LIGHT_H
#define HPL_MATERIAL_BASE_LIGHT_H

#include "hpl1/engine/graphics/GPUProgram.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/scene/Light3D.h"

enum eBaseLightProgram {
	eBaseLightProgram_Point1 = 0,
	eBaseLightProgram_Point2 = 1,
	eBaseLightProgram_Point3 = 2,
	eBaseLightProgram_Spot1 = 3,
	eBaseLightProgram_Spot2 = 4,
	eBaseLightProgram_Spot3 = 5,
	eBaseLightProgram_LastEnum = 6
};

namespace hpl {

class cAmbProgramSetup : public iMaterialProgramSetup {
public:
	void Setup(iGpuProgram *apProgram, cRenderSettings *apRenderSettings);
};

class iMaterial_BaseLight : public iMaterial {
public:
	iMaterial_BaseLight(const tString &asLightVertexProgram,
						const tString &asLightFragmentProgram,

						const tString &asName, iLowLevelGraphics *apLowLevelGraphics,
						cImageManager *apImageManager, cTextureManager *apTextureManager,
						cRenderer2D *apRenderer, cGpuProgramManager *apProgramManager,
						eMaterialPicture aPicture, cRenderer3D *apRenderer3D);

	virtual ~iMaterial_BaseLight();

	tTextureTypeList GetTextureTypes();

	bool UsesType(eMaterialRenderType aType);

	iGpuProgram *getGpuProgram(const eMaterialRenderType aType, const int alPass, iLight3D *apLight);
	iMaterialProgramSetup *getGpuProgramSetup(const eMaterialRenderType aType, const int alPass, iLight3D *apLight);

	bool VertexProgramUsesLight(eMaterialRenderType aType, int alPass, iLight3D *apLight);
	bool VertexProgramUsesEye(eMaterialRenderType aType, int alPass, iLight3D *apLight);

	eMaterialAlphaMode GetAlphaMode(eMaterialRenderType aType, int alPass, iLight3D *apLight);
	eMaterialBlendMode GetBlendMode(eMaterialRenderType aType, int alPass, iLight3D *apLight);
	eMaterialChannelMode GetChannelMode(eMaterialRenderType aType, int alPass, iLight3D *apLight);

	iTexture *GetTexture(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight);
	eMaterialBlendMode GetTextureBlend(int alUnit, eMaterialRenderType aType, int alPass, iLight3D *apLight);

	int GetNumOfPasses(eMaterialRenderType aType, iLight3D *apLight);

	//////////////////////////////////////////////////////////////////
	// Old and worthless stuff, only used by 2D renderer
	void Compile() {}
	bool StartRendering(eMaterialRenderType aType, iCamera *apCam, iLight *pLight) { return false; }
	void EndRendering(eMaterialRenderType aType) {}
	tVtxBatchFlag GetBatchFlags(eMaterialRenderType aType) { return 0; }
	bool NextPass(eMaterialRenderType aType) { return false; }
	bool HasMultiplePasses(eMaterialRenderType aType) { return false; }
	eMaterialType GetType(eMaterialRenderType aType) { return eMaterialType_Diffuse; }
	void EditVertexes(eMaterialRenderType aType, iCamera *apCam, iLight *pLight,
					  tVertexVec *apVtxVec, cVector3f *apTransform, unsigned int alIndexAdd) {}

protected:
	iTexture *mpNormalizationMap;
	iTexture *mpSpotNegativeRejectMap;

	bool mbUsesTwoPassSpot;

	// properties to set
	bool mbUseSpecular;
	bool mbUseNormalMap;
	bool mbUseColorSpecular;

	iGpuProgram *_diffuseShader;
	iGpuProgram *_ambientShader;

	iGpuProgram *_shaders[eBaseLightProgram_LastEnum];
};

}     // namespace hpl

#endif // HPL_MATERIAL_BASE_LIGHT_H
