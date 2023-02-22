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

#include "twine/script/script_life_v2.h"
#include "twine/resources/resources.h"
#include "twine/script/script_move_v2.h"
#include "twine/renderer/screens.h"
#include "twine/twine.h"

namespace TwinE {

static const ScriptLifeFunction function_map[] = {
	{"END", ScriptLifeV2::lEND},
	{"NOP", ScriptLifeV2::lNOP},
	{"SNIF", ScriptLifeV2::lSNIF},
	{"OFFSET", ScriptLifeV2::lOFFSET},
	{"NEVERIF", ScriptLifeV2::lNEVERIF},
	{"", ScriptLifeV2::lEMPTY}, // unused
	{"NO_IF", ScriptLifeV2::lNO_IF},
	{"", ScriptLifeV2::lEMPTY}, // unused
	{"", ScriptLifeV2::lEMPTY}, // unused
	{"", ScriptLifeV2::lEMPTY}, // unused
	{"PALETTE", ScriptLifeV2::lPALETTE},
	{"RETURN", ScriptLifeV2::lRETURN},
	{"IF", ScriptLifeV2::lIF},
	{"SWIF", ScriptLifeV2::lSWIF},
	{"ONEIF", ScriptLifeV2::lONEIF},
	{"ELSE", ScriptLifeV2::lELSE},
	{"ENDIF", ScriptLifeV2::lEMPTY}, // End of a conditional statement (e.g. IF)
	{"BODY", ScriptLifeV2::lBODY},
	{"BODY_OBJ", ScriptLifeV2::lBODY_OBJ},
	{"ANIM", ScriptLifeV2::lANIM},
	{"ANIM_OBJ", ScriptLifeV2::lANIM_OBJ},
	{"SET_CAMERA", ScriptLifeV2::lSET_CAMERA},
	{"CAMERA_CENTER", ScriptLifeV2::lCAMERA_CENTER},
	{"SET_TRACK", ScriptLifeV2::lSET_TRACK},
	{"SET_TRACK_OBJ", ScriptLifeV2::lSET_TRACK_OBJ},
	{"MESSAGE", ScriptLifeV2::lMESSAGE},
	{"FALLABLE", ScriptLifeV2::lFALLABLE},
	{"SET_DIRMODE", ScriptLifeV2::lSET_DIRMODE},
	{"SET_DIRMODE_OBJ", ScriptLifeV2::lSET_DIRMODE_OBJ},
	{"CAM_FOLLOW", ScriptLifeV2::lCAM_FOLLOW},
	{"SET_BEHAVIOUR", ScriptLifeV2::lSET_BEHAVIOUR},
	{"SET_FLAG_CUBE", ScriptLifeV2::lSET_FLAG_CUBE},
	{"COMPORTEMENT", ScriptLifeV2::lCOMPORTEMENT},
	{"SET_COMPORTEMENT", ScriptLifeV2::lSET_COMPORTEMENT},
	{"SET_COMPORTEMENT_OBJ", ScriptLifeV2::lSET_COMPORTEMENT_OBJ},
	{"END_COMPORTEMENT", ScriptLifeV2::lEND_COMPORTEMENT},
	{"SET_FLAG_GAME", ScriptLifeV2::lSET_FLAG_GAME},
	{"KILL_OBJ", ScriptLifeV2::lKILL_OBJ},
	{"SUICIDE", ScriptLifeV2::lSUICIDE},
	{"USE_ONE_LITTLE_KEY", ScriptLifeV2::lUSE_ONE_LITTLE_KEY},
	{"GIVE_GOLD_PIECES", ScriptLifeV2::lGIVE_GOLD_PIECES},
	{"END_LIFE", ScriptLifeV2::lEND_LIFE},
	{"STOP_L_TRACK", ScriptLifeV2::lSTOP_L_TRACK},
	{"RESTORE_L_TRACK", ScriptLifeV2::lRESTORE_L_TRACK},
	{"MESSAGE_OBJ", ScriptLifeV2::lMESSAGE_OBJ},
	{"INC_CHAPTER", ScriptLifeV2::lINC_CHAPTER},
	{"FOUND_OBJECT", ScriptLifeV2::lFOUND_OBJECT},
	{"SET_DOOR_LEFT", ScriptLifeV2::lSET_DOOR_LEFT},
	{"SET_DOOR_RIGHT", ScriptLifeV2::lSET_DOOR_RIGHT},
	{"SET_DOOR_UP", ScriptLifeV2::lSET_DOOR_UP},
	{"SET_DOOR_DOWN", ScriptLifeV2::lSET_DOOR_DOWN},
	{"GIVE_BONUS", ScriptLifeV2::lGIVE_BONUS},
	{"CHANGE_CUBE", ScriptLifeV2::lCHANGE_CUBE},
	{"OBJ_COL", ScriptLifeV2::lOBJ_COL},
	{"BRICK_COL", ScriptLifeV2::lBRICK_COL},
	{"OR_IF", ScriptLifeV2::lOR_IF},
	{"INVISIBLE", ScriptLifeV2::lINVISIBLE},
	{"SHADOW_OBJ", ScriptLifeV2::lSHADOW_OBJ},
	{"POS_POINT", ScriptLifeV2::lPOS_POINT},
	{"SET_MAGIC_LEVEL", ScriptLifeV2::lSET_MAGIC_LEVEL},
	{"SUB_MAGIC_POINT", ScriptLifeV2::lSUB_MAGIC_POINT},
	{"SET_LIFE_POINT_OBJ", ScriptLifeV2::lSET_LIFE_POINT_OBJ},
	{"SUB_LIFE_POINT_OBJ", ScriptLifeV2::lSUB_LIFE_POINT_OBJ},
	{"HIT_OBJ", ScriptLifeV2::lHIT_OBJ},
	{"PLAY_ACF", ScriptLifeV2::lPLAY_ACF},
	{"PLAY_MIDI", ScriptLifeV2::lPLAY_MIDI},
	{"ECLAIR", ScriptLifeV2::lECLAIR},
	{"INC_CLOVER_BOX", ScriptLifeV2::lINC_CLOVER_BOX},
	{"SET_USED_INVENTORY", ScriptLifeV2::lSET_USED_INVENTORY},
	{"ADD_CHOICE", ScriptLifeV2::lADD_CHOICE},
	{"ASK_CHOICE", ScriptLifeV2::lASK_CHOICE},
	{"INIT_BUGGY", ScriptLifeV2::lINIT_BUGGY},
	{"MEMO_ARDOISE", ScriptLifeV2::lMEMO_ARDOISE},
	{"SET_HOLO_POS", ScriptLifeV2::lSET_HOLO_POS},
	{"CLR_HOLO_POS", ScriptLifeV2::lCLR_HOLO_POS},
	{"ADD_FUEL", ScriptLifeV2::lADD_FUEL},
	{"SUB_FUEL", ScriptLifeV2::lSUB_FUEL},
	{"SET_GRM", ScriptLifeV2::lSET_GRM},
	{"SET_CHANGE_CUBE", ScriptLifeV2::lSET_CHANGE_CUBE},
	{"MESSAGE_ZOE", ScriptLifeV2::lMESSAGE_ZOE}, // lSAY_MESSAGE
	{"FULL_POINT", ScriptLifeV2::lFULL_POINT},
	{"BETA", ScriptLifeV2::lBETA},
	{"FADE_TO_PAL", ScriptLifeV2::lFADE_TO_PAL},
	{"ACTION", ScriptLifeV2::lACTION},
	{"SET_FRAME", ScriptLifeV2::lSET_FRAME},
	{"SET_SPRITE", ScriptLifeV2::lSET_SPRITE},
	{"SET_FRAME_3DS", ScriptLifeV2::lSET_FRAME_3DS},
	{"IMPACT_OBJ", ScriptLifeV2::lIMPACT_OBJ},
	{"IMPACT_POINT", ScriptLifeV2::lIMPACT_POINT},
	{"ADD_MESSAGE", ScriptLifeV2::lADD_MESSAGE},
	{"BUBBLE", ScriptLifeV2::lBUBBLE},
	{"NO_CHOC", ScriptLifeV2::lNO_CHOC},
	{"ASK_CHOICE_OBJ", ScriptLifeV2::lASK_CHOICE_OBJ},
	{"CINEMA_MODE", ScriptLifeV2::lCINEMA_MODE},
	{"SAVE_HERO", ScriptLifeV2::lSAVE_HERO},
	{"RESTORE_HERO", ScriptLifeV2::lRESTORE_HERO},
	{"ANIM_SET", ScriptLifeV2::lANIM_SET},
	{"RAIN", ScriptLifeV2::lRAIN}, // LM_PLUIE
	{"GAME_OVER", ScriptLifeV2::lGAME_OVER},
	{"THE_END", ScriptLifeV2::lTHE_END},
	{"ESCALATOR", ScriptLifeV2::lESCALATOR},
	{"PLAY_MUSIC", ScriptLifeV2::lPLAY_MUSIC},
	{"TRACK_TO_VAR_GAME", ScriptLifeV2::lTRACK_TO_VAR_GAME},
	{"VAR_GAME_TO_TRACK", ScriptLifeV2::lVAR_GAME_TO_TRACK},
	{"ANIM_TEXTURE", ScriptLifeV2::lANIM_TEXTURE},
	{"ADD_MESSAGE_OBJ", ScriptLifeV2::lADD_MESSAGE_OBJ},
	{"BRUTAL_EXIT", ScriptLifeV2::lBRUTAL_EXIT},
	{"REM", ScriptLifeV2::lEMPTY}, // unused
	{"LADDER", ScriptLifeV2::lLADDER},
	{"SET_ARMOR", ScriptLifeV2::lSET_ARMOR},
	{"SET_ARMOR_OBJ", ScriptLifeV2::lSET_ARMOR_OBJ},
	{"ADD_LIFE_POINT_OBJ", ScriptLifeV2::lADD_LIFE_POINT_OBJ},
	{"STATE_INVENTORY", ScriptLifeV2::lSTATE_INVENTORY},
	{"AND_IF", ScriptLifeV2::lAND_IF},
	{"SWITCH", ScriptLifeV2::lSWITCH},
	{"OR_CASE", ScriptLifeV2::lOR_CASE},
	{"CASE", ScriptLifeV2::lCASE},
	{"DEFAULT", ScriptLifeV2::lEMPTY}, // unused
	{"BREAK", ScriptLifeV2::lBREAK},
	{"END_SWITCH", ScriptLifeV2::lEMPTY}, // unused
	{"SET_HIT_ZONE", ScriptLifeV2::lSET_HIT_ZONE},
	{"SAVE_COMPORTEMENT", ScriptLifeV2::lSAVE_COMPORTEMENT},
	{"RESTORE_COMPORTEMENT", ScriptLifeV2::lRESTORE_COMPORTEMENT},
	{"SAMPLE", ScriptLifeV2::lSAMPLE},
	{"SAMPLE_RND", ScriptLifeV2::lSAMPLE_RND},
	{"SAMPLE_ALWAYS", ScriptLifeV2::lSAMPLE_ALWAYS},
	{"SAMPLE_STOP", ScriptLifeV2::lSAMPLE_STOP},
	{"REPEAT_SAMPLE", ScriptLifeV2::lREPEAT_SAMPLE},
	{"BACKGROUND", ScriptLifeV2::lBACKGROUND},
	{"ADD_VAR_GAME", ScriptLifeV2::lADD_VAR_GAME},
	{"SUB_VAR_GAME", ScriptLifeV2::lSUB_VAR_GAME},
	{"ADD_VAR_CUBE", ScriptLifeV2::lADD_VAR_CUBE},
	{"SUB_VAR_CUBE", ScriptLifeV2::lSUB_VAR_CUBE},
	{"NOP", ScriptLifeV2::lEMPTY}, // unused
	{"SET_RAIL", ScriptLifeV2::lSET_RAIL},
	{"INVERSE_BETA", ScriptLifeV2::lINVERSE_BETA},
	{"NO_BODY", ScriptLifeV2::lNO_BODY},
	{"GIVE_GOLD_PIECES", ScriptLifeV2::lGIVE_GOLD_PIECES},
	{"STOP_L_TRACK_OBJ", ScriptLifeV2::lSTOP_L_TRACK_OBJ},
	{"RESTORE_L_TRACK_OBJ", ScriptLifeV2::lRESTORE_L_TRACK_OBJ},
	{"SAVE_COMPORTEMENT_OBJ", ScriptLifeV2::lSAVE_COMPORTEMENT_OBJ},
	{"RESTORE_COMPORTEMENT_OBJ", ScriptLifeV2::lRESTORE_COMPORTEMENT_OBJ},
	{"SPY", ScriptLifeV2::lEMPTY}, // unused
	{"DEBUG", ScriptLifeV2::lEMPTY}, // unused
	{"DEBUG_OBJ", ScriptLifeV2::lEMPTY}, // unused
	{"POPCORN", ScriptLifeV2::lEMPTY}, // unused
	{"FLOW_POINT", ScriptLifeV2::lFLOW_POINT},
	{"FLOW_OBJ", ScriptLifeV2::lFLOW_OBJ},
	{"SET_ANIM_DIAL", ScriptLifeV2::lSET_ANIM_DIAL},
	{"PCX", ScriptLifeV2::lPCX},
	{"END_MESSAGE", ScriptLifeV2::lEND_MESSAGE},
	{"END_MESSAGE_OBJ", ScriptLifeV2::lEND_MESSAGE_OBJ},
	{"PARM_SAMPLE", ScriptLifeV2::lPARM_SAMPLE},
	{"NEW_SAMPLE", ScriptLifeV2::lNEW_SAMPLE},
	{"POS_OBJ_AROUND", ScriptLifeV2::lPOS_OBJ_AROUND},
	{"PCX_MESS_OBJ", ScriptLifeV2::lPCX_MESS_OBJ}
};

int32 ScriptLifeV2::lPALETTE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 palIndex = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::PALETTE(%i)", palIndex);
	ScopedEngineFreeze scoped(engine);
	HQR::getEntry(engine->_screens->_palette, Resources::HQR_RESS_FILE, palIndex);
	engine->_screens->convertPalToRGBA(engine->_screens->_palette, engine->_screens->_paletteRGBA);
	engine->setPalette(engine->_screens->_paletteRGBA);
	engine->_screens->_useAlternatePalette = true;
	return 0;
}

int32 ScriptLifeV2::lPLAY_MUSIC(TwinEEngine *engine, LifeScriptContext &ctx) {
	// TODO: game var 157 is checked here
	return lPLAY_CD_TRACK(engine, ctx);
}

int32 ScriptLifeV2::lTRACK_TO_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lVAR_GAME_TO_TRACK(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lANIM_TEXTURE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lADD_MESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lADD_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lCAMERA_CENTER(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lBUBBLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lNO_CHOC(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lCINEMA_MODE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAVE_HERO(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lRESTORE_HERO(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lRAIN(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lESCALATOR(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_CAMERA(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPLAY_ACF(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSHADOW_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lECLAIR(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lINIT_BUGGY(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lMEMO_ARDOISE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_CHANGE_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lMESSAGE_ZOE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lFADE_TO_PAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lACTION(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_FRAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_SPRITE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_FRAME_3DS(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lIMPACT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lIMPACT_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lLADDER(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_ARMOR(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_ARMOR_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lADD_LIFE_POINT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSTATE_INVENTORY(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lAND_IF(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSWITCH(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lOR_CASE (TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lCASE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lDEFAULT(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lBREAK(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lEND_SWITCH(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_HIT_ZONE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAVE_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lRESTORE_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAMPLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAMPLE_RND(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAMPLE_ALWAYS(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAMPLE_STOP(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lREPEAT_SAMPLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lBACKGROUND(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lADD_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSUB_VAR_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lADD_VAR_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSUB_VAR_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_RAIL(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lINVERSE_BETA(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lNO_BODY(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSTOP_L_TRACK_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lRESTORE_L_TRACK_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSAVE_COMPORTEMENT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lRESTORE_COMPORTEMENT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSPY(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lDEBUG(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lDEBUG_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPOPCORN(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lFLOW_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lFLOW_OBJ (TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lSET_ANIM_DIAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPCX(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lEND_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lEND_MESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPARM_SAMPLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lNEW_SAMPLE (TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPOS_OBJ_AROUND(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

int32 ScriptLifeV2::lPCX_MESS_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	return -1;
}

ScriptLifeV2::ScriptLifeV2(TwinEEngine *engine) : ScriptLife(engine, function_map, ARRAYSIZE(function_map)) {
}

} // namespace TwinE
