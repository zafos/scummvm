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

#include "twine/scene/scene.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "engines/enhancements.h"
#include "twine/audio/music.h"
#include "twine/audio/sound.h"
#include "twine/debugger/debug_state.h"
#include "twine/holomap.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/scene/extra.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/scene/movements.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

Scene::~Scene() {
	free(_currentScene);
}

void Scene::setActorStaticFlags(ActorStruct *act, uint32 staticFlags) {
	if (staticFlags & 0x1) {
		act->_flags.bComputeCollisionWithObj = 1;
	}
	if (staticFlags & 0x2) {
		act->_flags.bComputeCollisionWithBricks = 1;
	}
	if (staticFlags & 0x4) {
		act->_flags.bCheckZone = 1;
	}
	if (staticFlags & 0x8) {
		act->_flags.bSpriteClip = 1;
	}
	if (staticFlags & 0x10) {
		act->_flags.bCanBePushed = 1;
	}
	if (staticFlags & 0x20) {
		act->_flags.bComputeLowCollision = 1;
	}
	if (staticFlags & 0x40) {
		act->_flags.bCanDrown = 1;
	}
	if (staticFlags & 0x80) {
		act->_flags.bComputeCollisionWithFloor = 1;
	}

	if (staticFlags & 0x100) {
		act->_flags.bUnk0100 = 1;
	}
	if (staticFlags & 0x200) {
		act->_flags.bIsInvisible = 1;
	}
	if (staticFlags & 0x400) {
		act->_flags.bSprite3D = 1;
	}
	if (staticFlags & 0x800) {
		act->_flags.bObjFallable = 1;
	}
	if (staticFlags & 0x1000) {
		act->_flags.bNoShadow = 1;
	}
	if (staticFlags & 0x2000) {
		act->_flags.bIsBackgrounded = 1;
	}
	if (staticFlags & 0x4000) {
		act->_flags.bIsCarrierActor = 1;
	}
	if (staticFlags & 0x8000) {
		act->_flags.bUseMiniZv = 1;
	}
	if (staticFlags & 0x10000) {
		act->_flags.bHasInvalidPosition = 1;
	}
	if (staticFlags & 0x20000) {
		act->_flags.bNoElectricShock = 1;
	}
	if (staticFlags & 0x40000) {
		act->_flags.bHasSpriteAnim3D = 1;
	}
	if (staticFlags & 0x80000) {
		act->_flags.bNoPreClipping = 1;
	}
	if (staticFlags & 0x100000) {
		act->_flags.bHasZBuffer = 1;
	}
	if (staticFlags & 0x200000) {
		act->_flags.bHasZBufferInWater = 1;
	}
}

void Scene::setBonusParameterFlags(ActorStruct *act, uint16 bonusFlags) {
	if (bonusFlags & 0x1) {
		act->_bonusParameter.givenNothing = 1;
	}
	if (bonusFlags & 0x2) {
		act->_bonusParameter.unk2 = 1;
	}
	if (bonusFlags & 0x4) {
		act->_bonusParameter.unk3 = 1;
	}
	if (bonusFlags & 0x8) {
		act->_bonusParameter.unk4 = 1;
	}
	if (bonusFlags & 0x10) {
		act->_bonusParameter.kashes = 1;
	}
	if (bonusFlags & 0x20) {
		act->_bonusParameter.lifepoints = 1;
	}
	if (bonusFlags & 0x40) {
		act->_bonusParameter.magicpoints = 1;
	}
	if (bonusFlags & 0x80) {
		act->_bonusParameter.key = 1;
	}
	if (bonusFlags & 0x100) {
		act->_bonusParameter.cloverleaf = 1;
	}
}

bool Scene::loadSceneCubeXY(int numcube, int32 *cubex, int32 *cubey) {
	uint8 *scene = nullptr;
	// numcube+1 because at 0 is SizeCube.MAX (size of the largest .SCC)
	const int32 sceneSize = HQR::getAllocEntry(&scene, Resources::HQR_SCENE_FILE, numcube + 1);
	if (sceneSize <= 0) {
		return false;
	}
	Common::MemoryReadStream stream(scene, sceneSize, DisposeAfterUse::YES);

	*cubex = *cubey = 0;

	// World info: INFO_WORLD
	const uint8 island = stream.readByte();

	// Used only for 3DExt
	const int32 x = stream.readByte();
	const int32 y = stream.readByte();

	/*uint8 shadowlvl =*/stream.readByte();
	/*uint8 modelaby =*/stream.readByte();
	const uint8 cubemode = stream.readByte();

	if (cubemode == CUBE_EXTERIEUR && island == _island && ABS(x - _currentCubeX) <= 1 && ABS(y - _currentCubeY) <= 1) {
		*cubex = x;
		*cubey = y;

		return true;
	}
	return false;
}

void Scene::loadModel(ActorStruct &actor, int32 modelIndex, bool lba1) {
	actor._body = modelIndex;
	if (!actor._flags.bSprite3D) {
		debug(1, "Init actor with model %i", modelIndex);
		_engine->_resources->loadEntityData(actor._entityData, modelIndex);
		actor._entityDataPtr = &actor._entityData;
	} else {
		actor._entityDataPtr = nullptr;
	}
}

bool Scene::loadSceneLBA2() {
	Common::MemoryReadStream stream(_currentScene, _currentSceneSize);
	_island = stream.readByte();
	_sceneTextBank = (TextBankId)_island;
	_currentCubeX = stream.readByte();
	_currentCubeY = stream.readByte();
	_shadowLevel = stream.readByte();
	_modeLabyrinthe = stream.readByte();
	_isOutsideScene = stream.readByte();

	/*uint8 n =*/ stream.readByte();

	_alphaLight = ClampAngle(stream.readSint16LE());
	_betaLight = ClampAngle(stream.readSint16LE());
	debug(2, "Using %i and %i as light vectors", _alphaLight, _betaLight);

	for (int i = 0; i < 4; ++i) {
		_sampleAmbiance[i] = stream.readUint16LE();
		_sampleRepeat[i] = stream.readUint16LE();
		_sampleRound[i] = stream.readUint16LE();
		_sampleFrequency[i] = stream.readUint16LE();
		_sampleVolume[i] = stream.readUint16LE();
	}

	_sampleMinDelay = stream.readUint16LE();
	_sampleMinDelayRnd = stream.readUint16LE();

	_cubeJingle = stream.readByte();

	// load hero properties
	_sceneHeroPos.x = stream.readSint16LE();
	_sceneHeroPos.y = stream.readSint16LE();
	_sceneHeroPos.z = stream.readSint16LE();

	_sceneHero->_moveScriptSize = (int16)stream.readUint16LE();
	_sceneHero->_ptrTrack = _currentScene + stream.pos();
	stream.skip(_sceneHero->_moveScriptSize);

	_sceneHero->_lifeScriptSize = (int16)stream.readUint16LE();
	_sceneHero->_lifeScript = _currentScene + stream.pos();
	stream.skip(_sceneHero->_lifeScriptSize);

	_nbObjets = (int16)stream.readUint16LE();
	int cnt = 1;
	for (int32 a = 1; a < _nbObjets; a++, cnt++) {
		_engine->_actor->initObject(a);
		ActorStruct *act = &_sceneActors[a];
		setActorStaticFlags(act, stream.readUint32LE());

		loadModel(*act, (int16)stream.readUint16LE(), false);

		act->_genBody = (BodyType)stream.readByte();
		act->_genAnim = (AnimationTypes)stream.readSint16LE();
		act->_sprite = (int16)stream.readUint16LE();
		act->_posObj.x = (int16)stream.readUint16LE();
		act->_posObj.y = (int16)stream.readUint16LE();
		act->_posObj.z = (int16)stream.readUint16LE();
		act->_oldPos = act->posObj();
		act->_hitForce = stream.readByte();
		setBonusParameterFlags(act, stream.readUint16LE());
		act->_beta = (int16)stream.readUint16LE();
		act->_srot = (int16)stream.readUint16LE();
		act->_move = (ControlMode)stream.readByte(); // move
		act->_cropLeft = stream.readSint16LE();
		act->_delayInMillis = act->_cropLeft; // TODO: this might not be needed
		act->_cropTop = stream.readSint16LE();
		act->_cropRight = stream.readSint16LE();
		act->_cropBottom = stream.readSint16LE();
		act->_followedActor = act->_cropBottom; // TODO: is this needed? and valid?
		act->_bonusAmount = stream.readSint16LE();
		act->_talkColor = stream.readByte();
		if (act->_flags.bHasSpriteAnim3D) {
			/*act->spriteAnim3DNumber = */stream.readSint32LE();
			/*act->spriteSizeHit = */stream.readSint16LE();
			/*act->cropBottom = act->spriteSizeHit;*/
		}
		act->_armor = stream.readByte();
		act->setLife(stream.readByte());

		act->_moveScriptSize = (int16)stream.readUint16LE();
		act->_ptrTrack = _currentScene + stream.pos();
		stream.skip(act->_moveScriptSize);

		act->_lifeScriptSize = (int16)stream.readUint16LE();
		act->_lifeScript = _currentScene + stream.pos();
		stream.skip(act->_lifeScriptSize);

		if (_engine->_debugState->_onlyLoadActor != -1 && _engine->_debugState->_onlyLoadActor != cnt) {
			_nbObjets--;
			a--;
		}
	}

	/* uint32 checksum = */stream.readUint32LE();

	_sceneNumZones = (int16)stream.readUint16LE();
	for (int32 i = 0; i < _sceneNumZones; i++) {
		ZoneStruct *zone = &_sceneZones[i];
		zone->mins.x = stream.readSint32LE();
		zone->mins.y = stream.readSint32LE();
		zone->mins.z = stream.readSint32LE();

		zone->maxs.x = stream.readSint32LE();
		zone->maxs.y = stream.readSint32LE();
		zone->maxs.z = stream.readSint32LE();

		zone->infoData.generic.info0 = stream.readSint32LE();
		zone->infoData.generic.info1 = stream.readSint32LE();
		zone->infoData.generic.info2 = stream.readSint32LE();
		zone->infoData.generic.info3 = stream.readSint32LE();
		zone->infoData.generic.info4 = stream.readSint32LE();
		zone->infoData.generic.info5 = stream.readSint32LE();
		zone->infoData.generic.info6 = stream.readSint32LE();
		zone->infoData.generic.info7 = stream.readSint32LE();

		zone->type = (ZoneType)stream.readUint16LE();
		zone->num = stream.readSint16LE();
	}

	_sceneNumTracks = (int16)stream.readUint16LE();
	for (int32 i = 0; i < _sceneNumTracks; i++) {
		IVec3 *point = &_sceneTracks[i];
		point->x = stream.readSint32LE();
		point->y = stream.readSint32LE();
		point->z = stream.readSint32LE();
	}

	uint16 sceneNumPatches = stream.readUint32LE();
	for (uint16 i = 0; i < sceneNumPatches; i++) {
		/*size = */stream.readUint16LE();
		/*offset = */stream.readUint16LE();
	}

	return true;
}

// LoadScene
bool Scene::loadSceneLBA1() {
	Common::MemoryReadStream stream(_currentScene, _currentSceneSize);

	// load scene ambience properties
	_sceneTextBank = (TextBankId)stream.readByte();
	_currentGameOverScene = stream.readByte();
	stream.skip(4);

	// FIXME: Workaround to fix lighting issue - not using proper dark light
	// Using 1215 and 1087 as light vectors - scene 8
	_alphaLight = ClampAngle((int16)stream.readUint16LE());
	_betaLight = ClampAngle((int16)stream.readUint16LE());
	debug(2, "Using %i and %i as light vectors", _alphaLight, _betaLight);

	for (int i = 0; i < 4; ++i) {
		_sampleAmbiance[i] = stream.readUint16LE();
		_sampleRepeat[i] = stream.readUint16LE();
		_sampleRound[i] = stream.readUint16LE();
	}

	_sampleMinDelay = stream.readUint16LE();
	_sampleMinDelayRnd = stream.readUint16LE();

	_cubeJingle = stream.readByte();

	// load hero properties
	_sceneHeroPos.x = (int16)stream.readUint16LE();
	_sceneHeroPos.y = (int16)stream.readUint16LE();
	_sceneHeroPos.z = (int16)stream.readUint16LE();

	_sceneHero->_moveScriptSize = (int16)stream.readUint16LE();
	_sceneHero->_ptrTrack = _currentScene + stream.pos();
	stream.skip(_sceneHero->_moveScriptSize);

	_sceneHero->_lifeScriptSize = (int16)stream.readUint16LE();
	_sceneHero->_lifeScript = _currentScene + stream.pos();
	stream.skip(_sceneHero->_lifeScriptSize);

	_nbObjets = (int16)stream.readUint16LE();
	int cnt = 1;
	for (int32 a = 1; a < _nbObjets; a++, cnt++) {
		_engine->_actor->initObject(a);

		ActorStruct *act = &_sceneActors[a];
		setActorStaticFlags(act, stream.readUint16LE());

		loadModel(*act, stream.readUint16LE(), true);

		act->_genBody = (BodyType)stream.readByte();
		act->_genAnim = (AnimationTypes)stream.readByte();
		act->_sprite = (int16)stream.readUint16LE();
		act->_posObj.x = (int16)stream.readUint16LE();
		act->_posObj.y = (int16)stream.readUint16LE();
		act->_posObj.z = (int16)stream.readUint16LE();
		act->_oldPos = act->posObj();
		act->_hitForce = stream.readByte();
		setBonusParameterFlags(act, stream.readUint16LE());
		act->_bonusParameter.givenNothing = 0;
		act->_beta = (int16)stream.readUint16LE();
		act->_srot = (int16)stream.readUint16LE();
		act->_move = (ControlMode)stream.readUint16LE();
		act->_cropLeft = stream.readSint16LE();
		act->_delayInMillis = act->_cropLeft; // TODO: this might not be needed
		act->_cropTop = stream.readSint16LE();
		act->_cropRight = stream.readSint16LE();
		act->_cropBottom = stream.readSint16LE();
		act->_followedActor = act->_cropBottom; // TODO: is this needed? and valid?
		act->_bonusAmount = stream.readByte();
		act->_talkColor = stream.readByte();
		act->_armor = stream.readByte();
		act->setLife(stream.readByte());

		act->_moveScriptSize = (int16)stream.readUint16LE();
		act->_ptrTrack = _currentScene + stream.pos();
		stream.skip(act->_moveScriptSize);

		act->_lifeScriptSize = (int16)stream.readUint16LE();
		act->_lifeScript = _currentScene + stream.pos();
		stream.skip(act->_lifeScriptSize);

		if (_engine->_debugState->_onlyLoadActor != -1 && _engine->_debugState->_onlyLoadActor != cnt) {
			_nbObjets--;
			a--;
		}
	}

	_sceneNumZones = (int16)stream.readUint16LE();
	for (int32 i = 0; i < _sceneNumZones; i++) {
		ZoneStruct *zone = &_sceneZones[i];
		zone->mins.x = stream.readSint16LE();
		zone->mins.y = stream.readSint16LE();
		zone->mins.z = stream.readSint16LE();

		zone->maxs.x = stream.readSint16LE();
		zone->maxs.y = stream.readSint16LE();
		zone->maxs.z = stream.readSint16LE();

		zone->type = (ZoneType)stream.readUint16LE();
		zone->num = stream.readSint16LE();

		zone->infoData.generic.info0 = stream.readSint16LE();
		zone->infoData.generic.info1 = stream.readSint16LE();
		zone->infoData.generic.info2 = stream.readSint16LE();
		zone->infoData.generic.info3 = stream.readSint16LE();
	}

	_sceneNumTracks = stream.readUint16LE();
	for (int32 i = 0; i < _sceneNumTracks; i++) {
		IVec3 *point = &_sceneTracks[i];
		point->x = stream.readSint16LE();
		point->y = stream.readSint16LE();
		point->z = stream.readSint16LE();
	}

	if (_engine->isLBA1()) {
		if (_engine->enhancementEnabled(kEnhMinorBugFixes)) {
			if (_numCube == LBA1SceneId::Hamalayi_Mountains_landing_place) {
				// move the mine a little bit, as it's too close to the change cube zone
				_sceneActors[21]._posObj.x = _sceneActors[21]._oldPos.x = 6656 + 256;
				_sceneActors[21]._posObj.z = _sceneActors[21]._oldPos.z = 768;
			}
#if 0
			else if (_numCube == LBA1SceneId::Tippet_Island_Secret_passage_scene_1) {
				_sceneZones[6].maxs.z = 3616;
			}
#endif
		}
		if (_engine->enhancementEnabled(kEnhGameBreakingBugFixes)) {
			if (_numCube == LBA1SceneId::Principal_Island_outside_the_fortress) {
				// https://bugs.scummvm.org/ticket/13818
				_sceneActors[29]._posObj.z = _sceneActors[29]._oldPos.z = 1795;
			} else if (_numCube == LBA1SceneId::Principal_Island_inside_the_fortress) {
				// https://bugs.scummvm.org/ticket/13819
				// Set this zone to something invalid to fix a getting-stuck-bug
				// the original value was ZoneType::kGrid (3)
				_sceneZones[11].type = ZoneType::kFunFrockFix;
			}
		}
	}

	return true;
}

bool Scene::loadScene(int32 index) {
	// load scene from file
	if (_engine->isLBA2()) {
		index++;
	}
	_currentSceneSize = HQR::getAllocEntry(&_currentScene, Resources::HQR_SCENE_FILE, index);
	if (_currentSceneSize == 0) {
		return false;
	}

	if (_engine->isLBA1()) {
		return loadSceneLBA1();
	} else if (_engine->isLBA2()) {
		return loadSceneLBA2();
	}

	return false;
}

void Scene::clearScene() {
	_engine->_extra->clearExtra();

	// ClearFlagsCube
	for (int32 i = 0; i < ARRAYSIZE(_listFlagCube); i++) {
		_listFlagCube[i] = 0;
	}

	for (int32 i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		_engine->_redraw->overlayList[i].num = -1;
	}

	_engine->_screens->_flagPalettePcx = false;
}

void Scene::reloadCurrentScene() {
	_newCube = _numCube;
}

void Scene::dumpSceneScript(const char *type, int actorIdx, const uint8* script, int size) const {
	Common::String fname = Common::String::format("./dumps/%i-%i.%s", _numCube, actorIdx, type);
	Common::DumpFile out;
	if (!out.open(fname.c_str(), true)) {
		warning("Scene::dumpSceneScript(): Can not open dump file %s", fname.c_str());
	} else {
		out.write(script, size);
		out.flush();
		out.close();
	}
}

void Scene::dumpSceneScripts() const {
	for (int32 a = 0; a < _nbObjets; ++a) {
		const ActorStruct &actor = _sceneActors[a];
		dumpSceneScript("life", a, actor._lifeScript, actor._lifeScriptSize);
		dumpSceneScript("move", a, actor._ptrTrack, actor._moveScriptSize);
	}
}

void Scene::changeCube() {
	if (_engine->isLBA1()) {
		if (_engine->enhancementEnabled(kEnhMinorBugFixes)) {
			if (_numCube == LBA1SceneId::Citadel_Island_Harbor && _newCube == LBA1SceneId::Principal_Island_Harbor) {
				if (_sceneNumZones >= 15 && _sceneNumTracks >= 8) {
					const ZoneStruct *zone = &_sceneZones[15];
					const IVec3 &track = _sceneTracks[8];
					IVec3 &pos = _zoneHeroPos;
					pos.x = zone->infoData.ChangeScene.x - zone->mins.x + track.x;
					pos.y = zone->infoData.ChangeScene.y - zone->mins.y + track.y;
					pos.z = zone->infoData.ChangeScene.z - zone->mins.z + track.z;
					_engine->_scene->_flagChgCube = ScenePositionType::kZone;
					debug(2, "Using zone position %i:%i:%i", pos.x, pos.y, pos.z);
				}
			}
		}

		// change twinsen house destroyed hard-coded
		if (_newCube == LBA1SceneId::Citadel_Island_near_twinsens_house && _engine->_gameState->hasOpenedFunfrocksSafe()) {
			_newCube = LBA1SceneId::Citadel_Island_Twinsens_house_destroyed;
		}
	}

	// local backup previous scene
	_oldcube = _numCube;
	_numCube = _newCube;

	snprintf(_engine->_gameState->_sceneName, sizeof(_engine->_gameState->_sceneName), "%i %s", _numCube, _engine->_holomap->getLocationName(_numCube));
	debug(2, "Entering scene %s (came from %i)", _engine->_gameState->_sceneName, _oldcube);

	if (_engine->isLBA1()) {
		if (_newCube == LBA1SceneId::Polar_Island_end_scene) {
			_engine->unlockAchievement("LBA_ACH_001");
			// if you finish the game in less than 4 hours
			if (_engine->getTotalPlayTime() <= 1000 * 60 * 60 * 4) {
				_engine->unlockAchievement("LBA_ACH_005");
			}
		} else if (_newCube == LBA1SceneId::Brundle_Island_Secret_room) {
			_engine->unlockAchievement("LBA_ACH_006");
		}
	}

	_engine->_sound->stopSamples();

	clearScene();
	_engine->_actor->loadHeroEntities();

	_sceneHero->_move = ControlMode::kManual;
	_sceneHero->_zoneSce = -1;
	_sceneHero->_offsetLife = 0;
	_sceneHero->_offsetTrack = -1;
	_sceneHero->_labelTrack = -1;

	loadScene(_newCube);
	if (ConfMan.getBool("dump_scripts")) {
		dumpSceneScripts();
	}

	if (_numHolomapTraj != -1) {
		_engine->testRestoreModeSVGA(false);
		_engine->_screens->setBlackPal();
		_engine->_holomap->holoTraj(_numHolomapTraj);
		_numHolomapTraj = -1;
		_engine->_screens->_flagFade = true;
	} else {
		// TODO lbawin can do a fade here (if activated)
		// _engine->_screens->_flagFade = true;

	}

	if (_newCube == LBA1SceneId::Citadel_Island_end_sequence_1 || _newCube == LBA1SceneId::Citadel_Island_end_sequence_2) {
		_sceneTextBank = TextBankId::Tippet_Island;
	}

	_engine->_text->initSceneTextBank();

	if (_cubeJingle != 255) {
		// _engine->_music->fadeMusicMidi(1);
	}

	_engine->_grid->initGrid(_newCube);

	if (_flagChgCube == ScenePositionType::kZone) {
		_sceneStart = _zoneHeroPos;
	} else if (_flagChgCube == ScenePositionType::kScene || _flagChgCube == ScenePositionType::kNoPosition) {
		_sceneStart = _sceneHeroPos;
	}

	_sceneHero->_posObj = _sceneStart;
	_startYFalling = _sceneStart.y;

	_engine->_renderer->setLightVector(_alphaLight, _betaLight, LBAAngles::ANGLE_0);

	if (_oldcube != SCENE_CEILING_GRID_FADE_1 && _oldcube != _newCube) {
		_engine->_actor->_previousHeroBehaviour = _engine->_actor->_heroBehaviour;
		_engine->_actor->_previousHeroAngle = _sceneHero->_beta;
		_engine->autoSave();
	}

	_engine->_actor->restartPerso();

	// StartInitAllObjs
	for (int32 a = 1; a < _nbObjets; a++) {
		_engine->_actor->startInitObj(a);
	}

	_engine->_gameState->_nbLittleKeys = 0;
	_engine->_gameState->_magicBall = -1;
	_engine->_movements->_lastJoyFlag = true;
	_engine->_grid->_zoneGrm = -1;
	_engine->_grid->_indexGrm = -1;
	_engine->_redraw->_firstTime = true;
	_engine->_cameraZone = false;
	_newCube = SCENE_CEILING_GRID_FADE_1;
	_flagChgCube = ScenePositionType::kNoPosition;
	_flagRenderGrid = true;

	_samplePlayed = 2 * 4 * 8;
	_timerNextAmbiance = 0;

	ActorStruct *followedActor = getActor(_numObjFollow);
	_engine->_grid->centerOnActor(followedActor);

	_engine->_screens->_flagFade = true;
	_engine->_renderer->setLightVector(_alphaLight, _betaLight, LBAAngles::ANGLE_0);

	_zoneHeroPos = IVec3();

	debug(2, "Scene %i music track id: %i", _numCube, _cubeJingle);
	if (_cubeJingle != 255) {
		_engine->_music->playMusic(_cubeJingle);
	}

	_engine->_gameState->handleLateGameItems();
}

ActorStruct *Scene::getActor(int32 actorIdx) {
	if (actorIdx < 0 || actorIdx >= NUM_MAX_ACTORS) {
		error("Invalid actor id given: %i", actorIdx);
	}
	return &_sceneActors[actorIdx];
}

void Scene::initSceneVars() {
	_sampleAmbiance[0] = -1;
	_sampleAmbiance[1] = -1;
	_sampleAmbiance[2] = -1;
	_sampleAmbiance[3] = -1;

	_sampleRepeat[0] = 0;
	_sampleRepeat[1] = 0;
	_sampleRepeat[2] = 0;
	_sampleRepeat[3] = 0;

	_sampleRound[0] = 0;
	_sampleRound[1] = 0;
	_sampleRound[2] = 0;
	_sampleRound[3] = 0;

	_nbObjets = 0;
	_sceneNumZones = 0;
	_sceneNumTracks = 0;
}

void Scene::playSceneMusic() {
	if (_engine->isLBA1()) {
		if (_numCube == LBA1SceneId::Tippet_Island_Twinsun_Cafe && _engine->_gameState->hasArrivedHamalayi()) {
			if (_engine->isCDROM()) {
				_engine->_music->playCdTrack(8);
			} else {
				_engine->_music->playMusic(_cubeJingle);
			}
			return;
		}
	}
	_engine->_music->playMidiFile(_cubeJingle);
}

void Scene::processEnvironmentSound() {
	if (_engine->timerRef < _timerNextAmbiance) {
		return;
	}
	int16 currentAmb = _engine->getRandomNumber(4); // random ambiance

	for (int32 s = 0; s < 4; s++) {
		if (!(_samplePlayed & (1 << currentAmb))) { // if not already played
			_samplePlayed |= (1 << currentAmb);     // make sample played

			if (_samplePlayed == 15) { // reset if all samples played
				_samplePlayed = 0;
			}

			const int16 sampleIdx = _sampleAmbiance[currentAmb];
			if (sampleIdx != -1) {
				int16 decal = _sampleRound[currentAmb];
				int16 repeat = _sampleRepeat[currentAmb];

				const uint16 pitchbend = 0x1000 + _engine->getRandomNumber(decal) - (decal / 2);
				_engine->_sound->mixSample(sampleIdx, pitchbend, repeat, 110, 110);
				break;
			}
		}

		currentAmb++;    // try next ambiance
		currentAmb &= 3; // loop in all 4 ambiances
	}

	// compute next ambiance timer
	_timerNextAmbiance = _engine->timerRef + _engine->toSeconds(_engine->getRandomNumber(_sampleMinDelayRnd) + _sampleMinDelay);
}

void Scene::processZoneExtraBonus(ZoneStruct *zone) {
	if (zone->infoData.Bonus.used) {
		return;
	}

	const int bonusSprite = _engine->_extra->getBonusSprite(zone->infoData.Bonus.typesFlag);
	if (bonusSprite == -1) {
		return;
	}

	const int32 amount = zone->infoData.Bonus.amount;
	const int32 x = (zone->maxs.x + zone->mins.x) / 2;
	const int32 z = (zone->maxs.z + zone->mins.z) / 2;
	const int32 angle = _engine->_movements->getAngle(x, z, _sceneHero->_posObj.x, _sceneHero->_posObj.z);
	const int32 index = _engine->_extra->addExtraBonus(x, zone->maxs.y, z, LBAAngles::ANGLE_63, angle, bonusSprite, amount);

	if (index != -1) {
		_engine->_extra->_extraList[index].type |= ExtraType::TIME_IN;
		zone->infoData.Bonus.used = 1; // set as used
	}
}

void Scene::checkZoneSce(int32 actorIdx) {
	ActorStruct *actor = &_sceneActors[actorIdx];

	int32 currentX = actor->_posObj.x;
	int32 currentY = actor->_posObj.y;
	int32 currentZ = actor->_posObj.z;

	actor->_zoneSce = -1;
	bool flaggrm = false;

	if (IS_HERO(actorIdx)) {
		_flagClimbing = false;
	}

	for (int32 z = 0; z < _sceneNumZones; z++) {
		ZoneStruct *zone = &_sceneZones[z];

		// check if actor is in zone
		if ((currentX >= zone->mins.x && currentX <= zone->maxs.x) &&
		    (currentY >= zone->mins.y && currentY <= zone->maxs.y) &&
		    (currentZ >= zone->mins.z && currentZ <= zone->maxs.z)) {
			switch (zone->type) {
			default:
				warning("lba2 zone types not yet implemented");
				break;
			case ZoneType::kFunFrockFix:
				break;
			case ZoneType::kCube:
				if (IS_HERO(actorIdx) && actor->_lifePoint > 0) {
					_newCube = zone->num;
					_zoneHeroPos.x = actor->_posObj.x - zone->mins.x + zone->infoData.ChangeScene.x;
					_zoneHeroPos.y = actor->_posObj.y - zone->mins.y + zone->infoData.ChangeScene.y;
					_zoneHeroPos.z = actor->_posObj.z - zone->mins.z + zone->infoData.ChangeScene.z;
					_flagChgCube = ScenePositionType::kZone;
				}
				break;
			case ZoneType::kCamera:
				if (_numObjFollow == actorIdx && !_engine->_debugState->_useFreeCamera) {
					_engine->_cameraZone = true;
					if (_engine->_grid->_startCube.x != zone->infoData.CameraView.x || _engine->_grid->_startCube.y != zone->infoData.CameraView.y || _engine->_grid->_startCube.z != zone->infoData.CameraView.z) {
						_engine->_grid->_startCube.x = zone->infoData.CameraView.x;
						_engine->_grid->_startCube.y = zone->infoData.CameraView.y;
						_engine->_grid->_startCube.z = zone->infoData.CameraView.z;
						_engine->_redraw->_firstTime = true;
					}
				}
				break;
			case ZoneType::kSceneric:
				actor->_zoneSce = zone->num;
				break;
			case ZoneType::kGrid:
				if (_numObjFollow == actorIdx) {
					flaggrm = true;
					if (_engine->_grid->_zoneGrm != zone->num) {
						if (_engine->_grid->_zoneGrm != -1) {
							_engine->_grid->copyMapToCube();
						}

						_engine->_grid->_zoneGrm = zone->num;
						_engine->_grid->_indexGrm = z;
						_engine->saveTimer(false);
						_engine->_grid->initCellingGrid(_engine->_grid->_zoneGrm);
						_engine->restoreTimer();
					}
				}
				break;
			case ZoneType::kObject:
				if (IS_HERO(actorIdx) && _engine->_movements->actionNormal()) {
					_engine->_animations->initAnim(AnimationTypes::kAction, AnimType::kAnimationThen, AnimationTypes::kStanding, OWN_ACTOR_SCENE_INDEX);
					processZoneExtraBonus(zone);
				}
				break;
			case ZoneType::kText:
				if (IS_HERO(actorIdx) && _engine->_movements->actionNormal()) {
					_engine->saveTimer(false);
					_engine->testRestoreModeSVGA(true);
					_engine->_text->setFontCrossColor(zone->infoData.DisplayText.textColor);
					_talkingActor = actorIdx;
					_engine->_text->drawTextProgressive((TextId)zone->num);
					_engine->restoreTimer();
					_engine->_redraw->drawScene(true);
				}
				break;
			case ZoneType::kLadder:
				if (IS_HERO(actorIdx) && _engine->_actor->_heroBehaviour != HeroBehaviourType::kProtoPack && (actor->_genAnim == AnimationTypes::kForward || actor->_genAnim == AnimationTypes::kTopLadder || actor->_genAnim == AnimationTypes::kClimbLadder)) {
					IVec2 destPos = _engine->_renderer->rotate(actor->_boundingBox.mins.x, actor->_boundingBox.mins.z, actor->_beta + LBAAngles::ANGLE_360 + LBAAngles::ANGLE_135);
					destPos.x += actor->_processActor.x;
					destPos.y += actor->_processActor.z;

					if (destPos.x >= 0 && destPos.y >= 0 && destPos.x <= SCENE_SIZE_MAX && destPos.y <= SCENE_SIZE_MAX) {
						if (_engine->_grid->worldColBrick(destPos.x, actor->_posObj.y + SIZE_BRICK_Y, destPos.y) != ShapeType::kNone) {
							_flagClimbing = true;
							if (actor->_posObj.y >= (zone->mins.y + zone->maxs.y) / 2) {
								_engine->_animations->initAnim(AnimationTypes::kTopLadder, AnimType::kAnimationAllThen, AnimationTypes::kStanding, actorIdx); // reached end of ladder
							} else {
								_engine->_animations->initAnim(AnimationTypes::kClimbLadder, AnimType::kAnimationTypeRepeat, AnimationTypes::kNoAnim, actorIdx); // go up in ladder
							}
						}
					}
				}
				break;
			}
		}
	}

	if (!flaggrm && actorIdx == _numObjFollow && _engine->_grid->_zoneGrm != -1) {
		_engine->_grid->_zoneGrm = -1;
		_engine->_grid->_indexGrm = -1;
		_engine->_grid->copyMapToCube();
		_engine->_redraw->_firstTime = true;
	}
}

void Scene::stopRunningGame() {
	free(_currentScene);
	_currentScene = nullptr;
}

} // namespace TwinE
