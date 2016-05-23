/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "scumm/he/intern_he.h"

#include "scumm/he/moonbase/moonbase.h"
#include "scumm/he/moonbase/ai_main.h"
#include "scumm/he/moonbase/ai_traveller.h"
#include "scumm/he/moonbase/ai_targetacquisition.h"
#include "scumm/he/moonbase/ai_types.h"
#include "scumm/he/moonbase/ai_pattern.h"

namespace Scumm {

ScummEngine_v90he *_vm;

enum {
	F_GET_SCUMM_DATA = 0,
	F_GET_WORLD_DIST = 1,
	F_GET_WORLD_ANGLE = 2,
	F_GET_TERRAIN_TYPE = 3,
	F_GET_CLOSEST_UNIT = 4,
	F_SIMULATE_BUILDING_LAUNCH = 5,
	F_GET_POWER_ANGLE_FROM_POINT = 6,
	F_CHECK_IF_WATER_STATE = 7,
	F_GET_UNITS_WITHIN_RADIUS = 8,
	F_GET_LANDING_POINT = 9,
	F_GET_ENEMY_UNITS_VISIBLE = 10,
	F_CHECK_IF_WATER_SQUARE = 11,
	F_GET_GROUND_ALTITUDE = 12,
	F_CHECK_FOR_CORD_OVERLAP = 13,
	F_CHECK_FOR_ANGLE_OVERLAP = 14,
	F_ESTIMATE_NEXT_ROUND_ENERGY = 15,
	F_CHECK_FOR_UNIT_OVERLAP = 16,
	F_GET_COORDINATE_VISIBILITY = 17,
	F_CHECK_FOR_ENERGY_SQUARE = 18,
	F_AI_CHAT = 19
};

enum {
	D_GET_HUB_X = 1,
	D_GET_HUB_Y = 2,
	D_GET_WORLD_X_SIZE = 3,
	D_GET_WORLD_Y_SIZE = 4,
	D_GET_CURRENT_PLAYER = 5,
	D_GET_MAX_POWER = 6,
	D_GET_MIN_POWER = 7,
	D_GET_TERRAIN_SQUARE_SIZE = 8,
	D_GET_BUILDING_OWNER = 9,
	D_GET_BUILDING_STATE = 10,
	D_GET_BUILDING_TYPE = 11,
	D_DEBUG_BREAK = 12,
	D_GET_ENERGY_POOLS_ARRAY = 13,
	D_GET_COORDINATE_VISIBILITY = 14,
	D_GET_UNIT_VISIBILITY = 15,
	D_GET_ENERGY_POOL_VISIBILITY = 16,
	D_GET_NUMBER_OF_POOLS = 17,
	D_GET_NUMBER_OF_PLAYERS = 18,
	D_GET_BUILDING_ARMOR = 19,
	D_GET_BUILDING_WORTH = 20,
	D_GET_PLAYER_ENERGY = 21,
	D_GET_PLAYER_MAX_TIME = 22,
	D_GET_WIND_X_SPEED = 23,
	D_GET_WIND_Y_SPEED = 24,
	D_GET_TOTAL_WIND_SPEED = 25,
	D_GET_WIND_X_SPEED_MAX = 26,
	D_GET_WIND_Y_SPEED_MAX = 27,
	D_GET_BIG_X_SIZE = 28,
	D_GET_BIG_Y_SIZE = 29,
	D_GET_ENERGY_POOL_WIDTH = 30,
	D_GET_BUILDING_MAX_ARMOR = 31,
	D_GET_TIMER_VALUE = 32,
	D_GET_LAST_ATTACKED_X = 33,
	D_GET_LAST_ATTACKED_Y = 34,
	D_PRINT_DEBUG_TIMER = 35,
	D_GET_PLAYER_TEAM = 36,
	D_GET_BUILDING_TEAM = 37,
	D_GET_FOW = 38,
	D_GET_ANIM_SPEED = 39,
	D_GET_BUILDING_STACK_PTR = 40,
	D_GET_TURN_COUNTER = 41
};

enum {
	AI_TYPE_PLAYER_NUM = 0,
	AI_TYPE_TYPE = 1
};

enum {
	ENERGY_MODE = 0,
	OFFENSE_MODE = 1,
	DEFENSE_MODE = 2
};

enum {
	LAUNCH_SOURCE_HUB = 0,
	LAUNCH_UNIT = 1,
	LAUNCH_ANGLE = 2,
	LAUNCH_POWER = 3,

	MAX_LAUNCH_POWER = 560,
	MAX_FIRING_DISTANCE = 560
};

enum {
	SCALE_X = 50,
	SCALE_Y = 50,
	SCALE_Z = 50,

	GRAVITY_CONSTANT = (MAX_LAUNCH_POWER *MAX_LAUNCH_POWER) / MAX_FIRING_DISTANCE,

	HEIGHT_LOW = 20,

	NODE_RADIUS = 7,
	NODE_DIAMETER = NODE_RADIUS * 2 + 2,
	NODE_DETECT_RADIUS = NODE_RADIUS - 1,

	BUILDING_HUB_RADIUS = 16
};

enum {
	STATE_CHOOSE_BEHAVIOR = 0,
	STATE_CHOOSE_TARGET = 1,
	STATE_ATTEMPT_SEARCH = 2,
	STATE_INIT_APPROACH_TARGET = 3,
	STATE_APPROACH_TARGET = 4,
	STATE_INIT_ACQUIRE_TARGET = 5,
	STATE_ACQUIRE_TARGET = 6,
	STATE_ENERGIZE_TARGET = 7,
	STATE_OFFEND_TARGET = 8,
	STATE_DEFEND_TARGET = 9,
	STATE_LAUNCH = 10,
	STATE_CRAWLER_DECISION = 11,

	TREE_DEPTH = 2
};

AIEntity *AItype[5];

int AIstate = STATE_CHOOSE_BEHAVIOR;
int behavior = 2;

patternList *moveList[5];

int *storedLaunchAction[5] = {NULL};

const int *MCP_params;

Common::Array<int> lastXCoord[5];
Common::Array<int> lastYCoord[5];

void resetAI(ScummEngine_v90he *vm) {
	_vm = vm;

	AIstate = STATE_CHOOSE_BEHAVIOR;
	warning("----------------------> Resetting AI");

	for (int i = 1; i != 5; i++) {
		if (AItype[i]) {
			delete AItype[i];
			AItype[i] = NULL;
		}

		AItype[i] = new AIEntity(BRUTAKAS);
	}

	for (int i = 1; i != 5; i++) {
		if (moveList[i]) {
			delete moveList[i];
			moveList[i] = NULL;
		}

		moveList[i] = new patternList;
	}
}

void cleanUpAI() {
	warning("----------------------> Cleaning Up AI");

	for (int i = 1; i != 5; i++) {
		if (AItype[i]) {
			delete AItype[i];
			AItype[i] = NULL;
		}
	}

	for (int i = 1; i != 5; i++) {
		if (moveList[i]) {
			delete moveList[i];
			moveList[i] = NULL;
		}
	}
}

void setAIType(const int paramCount, const int32 *params) {
	if (AItype[params[AI_TYPE_PLAYER_NUM]]) {
		delete AItype[params[AI_TYPE_PLAYER_NUM]];
		AItype[params[AI_TYPE_PLAYER_NUM]] = NULL;
	}

	AItype[params[AI_TYPE_PLAYER_NUM]] = new AIEntity(params[AI_TYPE_TYPE]);

	if (params[AI_TYPE_TYPE] == ENERGY_HOG) {
		energyHogType = 1;
	} else {
		energyHogType = 0;
	}

	warning("AI for player %d is %s", params[AI_TYPE_PLAYER_NUM], AItype[params[AI_TYPE_PLAYER_NUM]]->getNameString());
}

int masterControlProgram(const int paramCount, const int32 *params) {
	static Tree *myTree;

	static int index;

	MCP_params = params;

	static int lastSource[5];
	static int lastAngle[5];
	static int lastPower[5];


	static int sourceHub;
	static int target;

	static int targetX;
	static int targetY;

	static int _acquireTarget = 0;

	static int *launchAction = NULL;
	static int *currentLaunchAction = NULL;

	static int OLflag = 0;
	static int TAflag = 0;


	Node *retNode;
	static int retNodeFlag;

	// Memory cleanup in case of quit during game
	if (_vm->readVar(_vm->VAR_U32_USER_VAR_F)) {
		if (myTree != NULL) {
			delete myTree;
			myTree = NULL;
		}

		if (launchAction != NULL) {
			delete launchAction;
			launchAction = NULL;
		}

		if (currentLaunchAction != NULL) {
			delete currentLaunchAction;
			currentLaunchAction = NULL;
		}

		return 1;
	}

	int currentPlayer = getCurrentPlayer();

	int maxTime = getPlayerMaxTime();
	int timerValue = getTimerValue(3);

	// If timer has run out
	if ((AIstate > STATE_CHOOSE_BEHAVIOR) && ((maxTime) && (timerValue > maxTime))) {
		if (myTree != NULL) {
			delete myTree;
			myTree = NULL;
		}

		if (launchAction != NULL) {
			delete launchAction;
			launchAction = NULL;
		}

		launchAction = new int[4];

		if (currentLaunchAction != NULL) {
			launchAction[LAUNCH_SOURCE_HUB] = currentLaunchAction[LAUNCH_SOURCE_HUB];
			launchAction[LAUNCH_UNIT] = currentLaunchAction[LAUNCH_UNIT];
			launchAction[LAUNCH_ANGLE] = currentLaunchAction[LAUNCH_ANGLE];
			launchAction[LAUNCH_POWER] = currentLaunchAction[LAUNCH_POWER];
			delete currentLaunchAction;
			currentLaunchAction = NULL;
		} else {
			if (!_vm->_rnd.getRandomNumber(1))
				launchAction[1] = ITEM_TIME_EXPIRED;
			else
				launchAction[1] = SKIP_TURN;
		}

		AIstate = STATE_LAUNCH;
	}

	static int oldAIstate = 0;

	if (oldAIstate != AIstate) {
		warning("<<%d>>", AIstate);
		oldAIstate = AIstate;
	}

	switch (AIstate) {
	case STATE_CHOOSE_BEHAVIOR:
		behavior = chooseBehavior();
		warning("Behavior mode: %d", behavior);

		if (_vm->_rnd.getRandomNumber(99) < AItype[getCurrentPlayer()]->getBehaviorVariation() * AI_VAR_BASE_BEHAVIOR + 1) {
			if (_vm->_rnd.getRandomNumber(1)) {
				behavior--;

				if (behavior < ENERGY_MODE) behavior = DEFENSE_MODE;
			} else {
				behavior++;

				if (behavior > DEFENSE_MODE) behavior = ENERGY_MODE;
			}

			warning("Alternative behavior: %d", behavior);
		}

		if (behavior == ENERGY_MODE)
			if (!getNumberOfPools())
				behavior = OFFENSE_MODE;

		if (AItype[getCurrentPlayer()]->getID() == CRAWLER_CHUCKER)
			behavior = OFFENSE_MODE;

		if (launchAction != NULL) {
			delete launchAction;
			launchAction = NULL;
		}

		index = 0;
		AIstate = STATE_CHOOSE_TARGET;
		break;

	case STATE_CHOOSE_TARGET:
		target = chooseTarget(behavior);

		if (!target)
			target = chooseTarget(OFFENSE_MODE);

		if (behavior == ENERGY_MODE) {
			int energyPoolScummArray = getEnergyPoolsArray();
			targetX = _vm->_moonbase->readFromArray(energyPoolScummArray, target, ENERGY_POOL_X);
			targetY = _vm->_moonbase->readFromArray(energyPoolScummArray, target, ENERGY_POOL_Y);
		} else {
			targetX = getHubX(target);
			targetY = getHubY(target);
		}

		warning("Target (%d, %d) id: %d", targetX, targetY, target);

		if (getFOW())
			AIstate = STATE_ATTEMPT_SEARCH;
		else
			AIstate = STATE_INIT_APPROACH_TARGET;

		break;

	case STATE_ATTEMPT_SEARCH:
		if (!getCoordinateVisibility(targetX, targetY, currentPlayer)) {
			int closestHub = getClosestUnit(targetX, targetY, getMaxX(), currentPlayer, 1, BUILDING_MAIN_BASE, 1, 0);
			int targetAngle = calcAngle(getHubX(closestHub), getHubY(closestHub), targetX, targetY);
			int testX = static_cast<int>(getHubX(closestHub) + (500 * cos(degToRad(targetAngle))) + getMaxX()) % getMaxX();
			int testY = static_cast<int>(getHubY(closestHub) + (500 * sin(degToRad(targetAngle))) + getMaxY()) % getMaxY();

			int balloonFlag = 0;

			int unitsArray = getUnitsWithinRadius(testX, testY, 500);
			int unitsCounter = 0;

			//see if any balloons are already in the area
			int nextBuilding = _vm->_moonbase->readFromArray(unitsArray, 0, unitsCounter);

			while (nextBuilding) {
				if (((getBuildingType(nextBuilding) == BUILDING_BALLOON) || (getBuildingType(nextBuilding) == BUILDING_TOWER)) && (getBuildingTeam(nextBuilding) == getPlayerTeam(currentPlayer))) {
					balloonFlag = 1;
					nextBuilding = 0;
					continue;
				}

				unitsCounter++;
				nextBuilding = _vm->_moonbase->readFromArray(unitsArray, 0, unitsCounter);
			}

			_vm->nukeArray(unitsArray);

			if (!balloonFlag) {
				launchAction = new int[4];
				launchAction[LAUNCH_SOURCE_HUB] = closestHub;

				if (getPlayerEnergy() > 3) {
					launchAction[LAUNCH_UNIT] = ITEM_BALLOON;
					launchAction[LAUNCH_POWER] = getMaxPower();
				} else {
					launchAction[LAUNCH_UNIT] = ITEM_TOWER;
					launchAction[LAUNCH_POWER] = getMinPower();
				}

				launchAction[LAUNCH_ANGLE] = targetAngle + (_vm->_rnd.getRandomNumber(89) - 45);

				int newTargetPos = abs(fakeSimulateWeaponLaunch(getHubX(closestHub), getHubY(closestHub), launchAction[LAUNCH_POWER], launchAction[LAUNCH_ANGLE]));
				targetX = newTargetPos % getMaxX();
				targetY = newTargetPos / getMaxY();

				AIstate = STATE_INIT_ACQUIRE_TARGET;
				break;
			}
		}

		AIstate = STATE_INIT_APPROACH_TARGET;
		break;

	case STATE_INIT_APPROACH_TARGET:
	{
		int closestOL = getClosestUnit(targetX, targetY, 900, currentPlayer, 1, BUILDING_OFFENSIVE_LAUNCHER, 1);

		if (closestOL && (behavior == OFFENSE_MODE)) {
			AIstate = STATE_OFFEND_TARGET;
			break;
		}
	}

	// get closest hub...if attack mode and almost close enough, maybe throw an offense
	if ((behavior == OFFENSE_MODE) && (getPlayerEnergy() > 6)) {
		if (!_vm->_rnd.getRandomNumber(2)) {
			int closestHub = getClosestUnit(targetX, targetY, getMaxX(), currentPlayer, 1, BUILDING_MAIN_BASE, 1);

			int dist = getDistance(targetX, targetY, getHubX(closestHub), getHubY(closestHub));

			if ((dist > 470) && (dist < 900)) {
				int closestOL = getClosestUnit(targetX, targetY, 900, currentPlayer, 1, BUILDING_OFFENSIVE_LAUNCHER, 0);

				if (!closestOL) {
					// Launch an OL
					OLflag = 1;
					targetX = getHubX(closestHub);
					targetY = getHubY(closestHub);

					AIstate = STATE_DEFEND_TARGET;
					break;
				}
			}
		}
	}

	if ((behavior == OFFENSE_MODE) && (AItype[currentPlayer]->getID() == RANGER) && (getPlayerEnergy() > 2)) {
		int closestHub = getClosestUnit(targetX, targetY, getMaxX(), currentPlayer, 1, BUILDING_MAIN_BASE, 1);
		int dist = getDistance(targetX, targetY, getHubX(closestHub), getHubY(closestHub));

		if (dist < 750) {
			AIstate = STATE_OFFEND_TARGET;
			break;
		}
	}

	myTree = initApproachTarget(targetX, targetY, &retNode);

	// If no need to approach, apply appropriate behavior
	if (retNode == myTree->getBaseNode()) {
		switch (behavior) {
		case 0:
			AIstate = STATE_ENERGIZE_TARGET;
			break;

		case 1:
			AIstate = STATE_OFFEND_TARGET;
			break;

		case 2:
			AIstate = STATE_DEFEND_TARGET;
			break;

		case -1:
			AIstate = STATE_LAUNCH;
			break;
		}

		delete myTree;
		myTree = NULL;
		break;
	}

	delete retNode;
	retNode = NULL;

	if (getPlayerEnergy() < 7) {
		if (!_vm->_rnd.getRandomNumber(3)) {
			behavior = DEFENSE_MODE;
			AIstate = STATE_CHOOSE_TARGET;
		} else {
			if (launchAction == NULL) {
				launchAction = new int[4];
			}

			if (!_vm->_rnd.getRandomNumber(2)) {
				launchAction[1] = ITEM_TIME_EXPIRED;
			} else {
				launchAction[1] = SKIP_TURN;
			}

			AIstate = STATE_LAUNCH;
		}

		delete myTree;
		myTree = NULL;
		break;
	}

	AIstate = STATE_CRAWLER_DECISION;
	break;

	// If behavior is offense, possibly just chuck a crawler
	case STATE_CRAWLER_DECISION:
	{
		// Brace just here to scope throwCrawler
		int throwCrawler = 0;

		if (behavior == OFFENSE_MODE) {
			if (getPlayerEnergy() > 6) {
				int crawlerTest = _vm->_rnd.getRandomNumber(9);

				if (!crawlerTest)
					throwCrawler = 1;
			}
		}

		if (AItype[getCurrentPlayer()]->getID() == CRAWLER_CHUCKER) {
			if (getPlayerEnergy() > 6) {
				throwCrawler = 1;
			} else {
				launchAction = new int[4];

				if (!_vm->_rnd.getRandomNumber(1))
					launchAction[1] = ITEM_TIME_EXPIRED;
				else
					launchAction[1] = SKIP_TURN;

				AIstate = STATE_LAUNCH;
				delete myTree;
				myTree = NULL;
			}
		}

		if (throwCrawler) {
			sourceHub = getClosestUnit(targetX, targetY, getMaxX(), getCurrentPlayer(), 1, BUILDING_MAIN_BASE, 1);
			int powAngle = getPowerAngleFromPoint(getHubX(sourceHub), getHubY(sourceHub), targetX, targetY, 15);
			powAngle = abs(powAngle);
			int power = powAngle / 360;
			int angle = powAngle - (power * 360);

			launchAction = new int[4];
			launchAction[0] = sourceHub;
			launchAction[1] = ITEM_CRAWLER;
			warning("CRAWLER DECISION is launching a crawler");
			launchAction[2] = angle;
			launchAction[3] = power;
			retNodeFlag = 0;

			// Need to update target so acquire can work
			int targetCoords = fakeSimulateWeaponLaunch(getHubX(launchAction[LAUNCH_SOURCE_HUB]), getHubY(launchAction[LAUNCH_SOURCE_HUB]), launchAction[LAUNCH_POWER], launchAction[LAUNCH_ANGLE]);
			targetX = targetCoords % getMaxX();
			targetY = targetCoords / getMaxX();
			targetX = (targetX + getMaxX()) % getMaxX();
			targetY = (targetY + getMaxY()) % getMaxY();

			AIstate = STATE_INIT_ACQUIRE_TARGET;
			delete myTree;
			myTree = NULL;
		} else {
			AIstate = STATE_APPROACH_TARGET;
		}
		break;
	}

	// ApproachTarget returns NULL if target is already reachable
	case STATE_APPROACH_TARGET:
		{
			int x, y;
			Node *currentNode = NULL;
			launchAction = approachTarget(myTree, x, y, &currentNode);
		}

		if (launchAction != NULL) {
			if (launchAction[0] == -1) {
				warning("Creating fake target approach hub");
				TAflag = 1;
				int closestHub = getClosestUnit(targetX, targetY, getMaxX(), currentPlayer, 1, BUILDING_MAIN_BASE, 1);
				targetX = getHubX(closestHub);
				targetY = getHubY(closestHub);

				delete launchAction;
				launchAction = NULL;
				AIstate = STATE_DEFEND_TARGET;
				delete myTree;
				myTree = NULL;
				break;
			}

			// Need to update target so acquire can work
			int targetCoords = fakeSimulateWeaponLaunch(getHubX(launchAction[LAUNCH_SOURCE_HUB]), getHubY(launchAction[LAUNCH_SOURCE_HUB]), launchAction[LAUNCH_POWER], launchAction[LAUNCH_ANGLE]);
			targetX = targetCoords % getMaxX();
			targetY = targetCoords / getMaxX();
			targetX = (targetX + getMaxX()) % getMaxX();
			targetY = (targetY + getMaxY()) % getMaxY();

			AIstate = STATE_INIT_ACQUIRE_TARGET;
			behavior = -1;

			delete myTree;
			myTree = NULL;
		}

		break;

	case STATE_ENERGIZE_TARGET:
		launchAction = energizeTarget(targetX, targetY, index);

		if (launchAction != NULL) {
			if (launchAction[0]) {
				assert(launchAction[0] > 0);

				if (launchAction[1] == ITEM_HUB) {
					index = 0;
					retNodeFlag = 0;
					AIstate = STATE_INIT_ACQUIRE_TARGET;
				} else {
					index = 0;
					AIstate = STATE_INIT_ACQUIRE_TARGET;
				}
			} else {
				index++;
				delete launchAction;
				launchAction = NULL;
			}
		} else {
			behavior = DEFENSE_MODE;
			retNodeFlag = 0;
			index = 0;
			AIstate = STATE_CHOOSE_TARGET;
		}
		break;

	case STATE_OFFEND_TARGET:
		launchAction = offendTarget(targetX, targetY, index);

		if (launchAction != NULL) {
			if (launchAction[0]) {
				retNodeFlag = 0;
				AIstate = STATE_INIT_ACQUIRE_TARGET;
			} else {
				index++;
				delete launchAction;
				launchAction = NULL;
			}
		}
		break;

	case STATE_DEFEND_TARGET:
		launchAction = defendTarget(targetX, targetY, index);

		if (launchAction != NULL) {
			if (launchAction[0]) {
				retNodeFlag = 0;
				AIstate = STATE_INIT_ACQUIRE_TARGET;

				if (launchAction[LAUNCH_UNIT] != ITEM_BRIDGE) {
					if (OLflag) {
						OLflag = 0;
						launchAction[LAUNCH_UNIT] = ITEM_OFFENSE;
					}

					if (TAflag) {
						TAflag = 0;
						warning("replacing defense unit %d with a hub", launchAction[LAUNCH_UNIT]);
						launchAction[LAUNCH_UNIT] = ITEM_HUB;
					}
				}
			} else {
				index++;
				delete launchAction;
				launchAction = NULL;
			}
		}
		break;

	case STATE_INIT_ACQUIRE_TARGET:
		myTree = initAcquireTarget(targetX, targetY, &retNode);

		if (myTree == NULL) {
			AIstate = STATE_LAUNCH;
			break;
		}

		// This next line is a questionable fix
		if (retNode == myTree->getBaseNode())
			retNodeFlag = 1;

		_acquireTarget = 0;

		AIstate = STATE_ACQUIRE_TARGET;
		break;

	case STATE_ACQUIRE_TARGET: {
		// Here to scope tempLaunchAction
		int *tempLaunchAction = NULL;

		int errCod = 0;

		_acquireTarget++;

		if (!retNodeFlag) {
			tempLaunchAction = acquireTarget(targetX, targetY, myTree, errCod);
		} else {
			warning("NOT acquiring target!!!!!!!");
			_acquireTarget = 101;
		}

		if (tempLaunchAction != NULL) {
			if (launchAction != NULL) {
				delete launchAction;
				launchAction = NULL;
			}

			launchAction = tempLaunchAction;
		}

		// If no hubs are available for launching...turn must be skipped
		if (launchAction != NULL) {
			if (launchAction[LAUNCH_SOURCE_HUB] == 0) {
				launchAction[LAUNCH_UNIT] = SKIP_TURN;
			}
		}

		if ((tempLaunchAction != NULL) || (errCod == 1) || (_acquireTarget > 100)) {
			if (tempLaunchAction == NULL) {
				warning("\nABORTING acquire target!!!!!");
			}

			assert(launchAction != NULL);
			delete myTree;
			myTree = NULL;
			AIstate = STATE_LAUNCH;
		}
	}
	break;

	default:
		break;
	}

	if (AIstate == STATE_LAUNCH) {
		static float randomAttenuation = 1;

		if (((launchAction[LAUNCH_UNIT] == ITEM_REPAIR) || (launchAction[LAUNCH_UNIT] == ITEM_ANTIAIR) || (launchAction[LAUNCH_UNIT] == ITEM_BRIDGE) || (launchAction[LAUNCH_UNIT] == ITEM_TOWER) || (launchAction[LAUNCH_UNIT] == ITEM_RECLAIMER) || (launchAction[LAUNCH_UNIT] == ITEM_BALLOON) || (launchAction[LAUNCH_UNIT] == ITEM_MINE) || (launchAction[LAUNCH_UNIT] == ITEM_ENERGY) || (launchAction[LAUNCH_UNIT] == ITEM_SHIELD) || (launchAction[LAUNCH_UNIT] == ITEM_OFFENSE) || (launchAction[LAUNCH_UNIT] == ITEM_HUB)) && (getBuildingType(launchAction[LAUNCH_SOURCE_HUB]) == BUILDING_OFFENSIVE_LAUNCHER)) {
			if (getPlayerEnergy() > 2) {
				launchAction[LAUNCH_UNIT] = ITEM_GUIDED;
			} else {
				launchAction[LAUNCH_UNIT] = ITEM_BOMB;
			}
		}

		if ((lastSource[currentPlayer] == launchAction[LAUNCH_SOURCE_HUB]) && (lastAngle[currentPlayer] == launchAction[LAUNCH_ANGLE]) && (lastPower[currentPlayer] == launchAction[LAUNCH_POWER])) {
			randomAttenuation -= .2f;
			randomAttenuation = MAX(randomAttenuation, 0.0f);
			warning("Attenuating...");
		} else {
			randomAttenuation = 1;
		}

		lastSource[currentPlayer] = launchAction[LAUNCH_SOURCE_HUB];
		lastAngle[currentPlayer] = launchAction[LAUNCH_ANGLE];
		lastPower[currentPlayer] = launchAction[LAUNCH_POWER];

		_vm->writeVar(_vm->VAR_U32_USER_VAR_A, launchAction[LAUNCH_SOURCE_HUB]);
		int energy = getPlayerEnergy();
		warning("Computer's energy: %d", energy);

		// Check if there's enough energy to launch this item
		int n = (launchAction[1] / 6);
		int energyRequired = (1 + n * n + n);

		if (((energy - energyRequired) < 0) || (!launchAction[LAUNCH_SOURCE_HUB])) {
			_vm->writeVar(_vm->VAR_U32_USER_VAR_B, SKIP_TURN);
		} else {
			assert((launchAction[LAUNCH_UNIT] >= 0) && (launchAction[LAUNCH_UNIT] <= 18));

			if ((launchAction[LAUNCH_UNIT] < 0) || (launchAction[LAUNCH_UNIT] > 18)) launchAction[LAUNCH_UNIT] = 0;

			_vm->writeVar(_vm->VAR_U32_USER_VAR_B, launchAction[LAUNCH_UNIT]);
		}

		if (launchAction[LAUNCH_UNIT] == ITEM_BOMB) {
			if (energy > 2) {
				if (!_vm->_rnd.getRandomNumber(4)) {
					launchAction[LAUNCH_UNIT] = ITEM_GUIDED;
				}
			}
		}

		{
			// ANGLE setting
			int angleAdjustment = 0;
			angleAdjustment = _vm->_rnd.getRandomNumber(AItype[getCurrentPlayer()]->getAngleVariation() * AI_VAR_BASE_ANGLE) * 3.6;
			//pos or neg choice
			angleAdjustment *= ((_vm->_rnd.getRandomNumber(1) * 2) - 1);
			angleAdjustment *= randomAttenuation;

			int safeAngle = 0;
			int lu = launchAction[LAUNCH_UNIT];

			if ((lu == ITEM_ANTIAIR) || (lu == ITEM_TOWER) || (lu == ITEM_ENERGY) || (lu == ITEM_SHIELD) || (lu == ITEM_OFFENSE) || (lu == ITEM_HUB)) {
				for (int i = 1; i < 90; i++) {
					assert((launchAction[LAUNCH_ANGLE] < 1000) && (angleAdjustment < 360));

					if (checkForAngleOverlap(launchAction[LAUNCH_SOURCE_HUB], launchAction[LAUNCH_ANGLE] + angleAdjustment)) {
						angleAdjustment += (i % 2 ? i : -i);
					} else {
						safeAngle = 1;
						i = 90;
					}
				}
			} else {
				safeAngle = 1;
			}

			if (!safeAngle) angleAdjustment = 0;

			warning("Angle adjustment = %d", angleAdjustment);
			_vm->writeVar(_vm->VAR_U32_USER_VAR_C, launchAction[LAUNCH_ANGLE] + angleAdjustment);
		}

		{
			// POWER setting
			int powerRangeFactor = (getMaxPower() - getMinPower()) / 100;
			int powerAdjustment = static_cast<float>(_vm->_rnd.getRandomNumber(AItype[getCurrentPlayer()]->getPowerVariation() * AI_VAR_BASE_POWER)) * powerRangeFactor;
			//pos or neg choice
			powerAdjustment *= ((_vm->_rnd.getRandomNumber(1) * 2) - 1);
			powerAdjustment *= randomAttenuation;

			warning("Power adjustment = %d", powerAdjustment);
			int newPower = MIN(getMaxPower(), launchAction[LAUNCH_POWER] + powerAdjustment);
			newPower = MAX(getMinPower(), launchAction[LAUNCH_POWER] + powerAdjustment);
			_vm->writeVar(_vm->VAR_U32_USER_VAR_D, newPower);

			assert(_vm->readVar(_vm->VAR_U32_USER_VAR_B) != -1);

			if (launchAction[LAUNCH_UNIT] != SKIP_TURN) {
				if ((launchAction[LAUNCH_SOURCE_HUB] > 0) && (launchAction[LAUNCH_SOURCE_HUB] <= 500)) {
					if (getBuildingState(launchAction[LAUNCH_SOURCE_HUB]) != 0) {
						_vm->writeVar(_vm->VAR_U32_USER_VAR_B, SKIP_TURN);
					}
				} else {
					_vm->writeVar(_vm->VAR_U32_USER_VAR_B, SKIP_TURN);
				}
			}

			if ((launchAction[LAUNCH_UNIT] == SKIP_TURN) || (launchAction[LAUNCH_POWER] == 0)) {
				_vm->writeVar(_vm->VAR_U32_USER_VAR_D, -1);
			}
		}


		if ((launchAction[LAUNCH_SOURCE_HUB] > 0) && (launchAction[LAUNCH_SOURCE_HUB] <= 500)) {
			int nearbyOpponents = getUnitsWithinRadius(getHubX(launchAction[LAUNCH_SOURCE_HUB]), getHubY(launchAction[LAUNCH_SOURCE_HUB]), 180);
			int opponentCounter = 0;
			int opponentBuilding = _vm->_moonbase->readFromArray(nearbyOpponents, 0, opponentCounter);
			int defenseOn = 0;
			int defenseOff = 0;

			while (opponentBuilding) {
				if (getBuildingOwner(opponentBuilding)) {
					if ((getBuildingType(opponentBuilding) == BUILDING_ANTI_AIR) && (getBuildingTeam(opponentBuilding) != getPlayerTeam(currentPlayer))) {
						if (getBuildingState(opponentBuilding) == 0) {
							defenseOn = 1;
						} else {
							defenseOff = 1;
						}
					}
				}

				opponentCounter++;
				opponentBuilding = _vm->_moonbase->readFromArray(nearbyOpponents, 0, opponentCounter);
			}

			_vm->nukeArray(nearbyOpponents);

			if (defenseOn && defenseOff) {
				if (!_vm->_rnd.getRandomNumber(1)) {
					_vm->writeVar(_vm->VAR_U32_USER_VAR_B, ITEM_TIME_EXPIRED);
				} else {
					_vm->writeVar(_vm->VAR_U32_USER_VAR_B, SKIP_TURN);
				}
			} else {
				if (defenseOn) {
					_vm->writeVar(_vm->VAR_U32_USER_VAR_B, ITEM_CLUSTER);
					int temp = _vm->_rnd.getRandomNumber(2);
					int tempPower = 0;

					switch (temp) {
					case 0:
						tempPower = getMinPower();
						break;

					case 1:
						tempPower = getMaxPower();
						break;

					default:
						tempPower = launchAction[LAUNCH_POWER];
					}

					_vm->writeVar(_vm->VAR_U32_USER_VAR_D, tempPower);
				}
			}
		}

		delete launchAction;
		launchAction = NULL;

		AIstate = STATE_CHOOSE_BEHAVIOR;

		int rSh, rU, rP, rA = 0;
		rSh = _vm->readVar(_vm->VAR_U32_USER_VAR_A);
		rU = _vm->readVar(_vm->VAR_U32_USER_VAR_B);
		rP = _vm->readVar(_vm->VAR_U32_USER_VAR_D);
		rA = _vm->readVar(_vm->VAR_U32_USER_VAR_C);

		warning("su: %d  unit: %d  power: %d  angle: %d", rSh, rU, rP, rA);

		{
			// Checking for patterns
			if ((AItype[currentPlayer]->getID() != CRAWLER_CHUCKER) &&
					(AItype[currentPlayer]->getID() != ENERGY_HOG) && (getBuildingStackPtr() > 5))
				moveList[currentPlayer]->addPattern(rSh, rU, rP, rA);

			int patternFound = moveList[currentPlayer]->evaluatePattern(rSh, rU, rP, rA);

			if (!_vm->_rnd.getRandomNumber(9))
				patternFound = 0;

			if (patternFound) {
				warning("------------------------------------------>Eliminating pattern");

				if (_vm->_rnd.getRandomNumber(1)) {
					behavior--;

					if (behavior < ENERGY_MODE)
						behavior = DEFENSE_MODE;
				} else {
					behavior++;

					if (behavior > DEFENSE_MODE)
						behavior = ENERGY_MODE;
				}

				if (behavior == ENERGY_MODE)
					if (!getNumberOfPools())
						behavior = OFFENSE_MODE;

				_vm->writeVar(_vm->VAR_U32_USER_VAR_A, 0);
				_vm->writeVar(_vm->VAR_U32_USER_VAR_B, 0);
				_vm->writeVar(_vm->VAR_U32_USER_VAR_C, 0);
				_vm->writeVar(_vm->VAR_U32_USER_VAR_D, 0);
				AIstate = STATE_CHOOSE_TARGET;
			}
		}

		if ((rSh > 0) && (rSh < 501)) {
			if ((rU == ITEM_ANTIAIR) || (rU == ITEM_TOWER) || (rU == ITEM_ENERGY) || (rU == ITEM_SHIELD) || (rU == ITEM_OFFENSE) || (rU == ITEM_HUB)) {
				int tryCount = 0;

				while (checkForAngleOverlap(rSh, rA) && tryCount < 25) {
					rA = _vm->_rnd.getRandomNumber(359);
					tryCount++;
				}

				if (tryCount < 25) {
					_vm->writeVar(_vm->VAR_U32_USER_VAR_C, rA);
				} else {
					_vm->writeVar(_vm->VAR_U32_USER_VAR_B, SKIP_TURN);
				}
			}
		}
	} else {
		_vm->writeVar(_vm->VAR_U32_USER_VAR_A, 0);
		_vm->writeVar(_vm->VAR_U32_USER_VAR_B, 0);
		_vm->writeVar(_vm->VAR_U32_USER_VAR_C, 0);
		_vm->writeVar(_vm->VAR_U32_USER_VAR_D, 0);
	}

	// Sending behavior to SCUMM side for profiling
	int selectedUnit = _vm->readVar(_vm->VAR_U32_USER_VAR_B);

	if (selectedUnit) {
		if (selectedUnit > 0)
			_vm->writeVar(_vm->VAR_U32_USER_VAR_E, behavior);
		else
			_vm->writeVar(_vm->VAR_U32_USER_VAR_E, -999);
	}

	return 1;
}

int chooseBehavior() {
	static int dominantMode = 0;

	int modeValue[3];

	const int DEFAULT_SCALE = 5;

	if (getBuildingStackPtr() < 5)
		return OFFENSE_MODE;

	int currentPlayer = getCurrentPlayer();

	int AIpersonality = AItype[currentPlayer]->getID();

	switch (AIpersonality) {
	case BRUTAKAS:
		modeValue[ENERGY_MODE] = DEFAULT_SCALE;
		modeValue[OFFENSE_MODE] = 2 * DEFAULT_SCALE;
		modeValue[DEFENSE_MODE] = 0;
		dominantMode = OFFENSE_MODE;
		break;

	case AGI:
		modeValue[ENERGY_MODE] = DEFAULT_SCALE;
		modeValue[OFFENSE_MODE] = 0;
		modeValue[DEFENSE_MODE] = 2 * DEFAULT_SCALE;
		dominantMode = DEFENSE_MODE;
		break;

	case EL_GATO:
		modeValue[ENERGY_MODE] = 2 * DEFAULT_SCALE;
		modeValue[OFFENSE_MODE] = DEFAULT_SCALE;
		modeValue[DEFENSE_MODE] = 0;
		dominantMode = ENERGY_MODE;
		break;

	case PIXELAHT:
		modeValue[ENERGY_MODE] = DEFAULT_SCALE;
		modeValue[OFFENSE_MODE] = 0;
		modeValue[DEFENSE_MODE] = DEFAULT_SCALE;
		dominantMode = DEFENSE_MODE;
		break;

	case CYBALL:
		modeValue[ENERGY_MODE] = 0;
		modeValue[OFFENSE_MODE] = 0;
		modeValue[DEFENSE_MODE] = 0;
		dominantMode = ENERGY_MODE;
		break;

	case NEEP:
		modeValue[ENERGY_MODE] = 0;
		modeValue[OFFENSE_MODE] = DEFAULT_SCALE;
		modeValue[DEFENSE_MODE] = DEFAULT_SCALE;
		dominantMode = DEFENSE_MODE;
		break;

	case WARCUPINE:
		modeValue[ENERGY_MODE] = 5 * DEFAULT_SCALE;
		modeValue[OFFENSE_MODE] = DEFAULT_SCALE;
		modeValue[DEFENSE_MODE] = 0;
		dominantMode = ENERGY_MODE;
		break;

	case AONE:
		modeValue[ENERGY_MODE] = 0;
		modeValue[OFFENSE_MODE] = DEFAULT_SCALE;
		modeValue[DEFENSE_MODE] = 2 * DEFAULT_SCALE;
		dominantMode = DEFENSE_MODE;
		break;

	case SPANDO:
		modeValue[ENERGY_MODE] = DEFAULT_SCALE;
		modeValue[OFFENSE_MODE] = DEFAULT_SCALE;
		modeValue[DEFENSE_MODE] = DEFAULT_SCALE;
		dominantMode = ENERGY_MODE;
		break;

	case ORBNU_LUNATEK:
		modeValue[ENERGY_MODE] = 0;
		modeValue[OFFENSE_MODE] = 0;
		modeValue[DEFENSE_MODE] = 0;
		dominantMode = ENERGY_MODE;
		break;

	case CRAWLER_CHUCKER:
		modeValue[ENERGY_MODE] = 0;
		modeValue[OFFENSE_MODE] = 2 * DEFAULT_SCALE;
		modeValue[DEFENSE_MODE] = 0;
		dominantMode = OFFENSE_MODE;
		break;

	case ENERGY_HOG:
		modeValue[ENERGY_MODE] = 2 * DEFAULT_SCALE;
		modeValue[OFFENSE_MODE] = 0;
		modeValue[DEFENSE_MODE] = 0;
		dominantMode = ENERGY_MODE;
		{
			int breakFlag = 0;

			for (int i = 500; i > 0; i--)
				if ((getBuildingOwner(i) == currentPlayer) && (getBuildingType(i) == BUILDING_ENERGY_COLLECTOR))
					breakFlag = 1;

			if (!breakFlag)
				return ENERGY_MODE;
		}
		break;

	case RANGER:
		modeValue[ENERGY_MODE] = 2 * DEFAULT_SCALE;
		modeValue[OFFENSE_MODE] = 0;
		modeValue[DEFENSE_MODE] = 0;
		dominantMode = OFFENSE_MODE;

		//random chance of defense if really early in game, otherwise offense
		if (_vm->_rnd.getRandomNumber(1) || getTurnCounter() > 4)
			return OFFENSE_MODE;

		return DEFENSE_MODE;
		break;

	default:  //BRUTAKAS
		modeValue[ENERGY_MODE] = DEFAULT_SCALE;
		modeValue[OFFENSE_MODE] = 2 * DEFAULT_SCALE;
		modeValue[DEFENSE_MODE] = 0;
		dominantMode = OFFENSE_MODE;
		break;
	}

	// get a list of all the visible enemy units
	int eneCon = 0;
	int offCon = 0;
	int defCon = 0;

	// energy mode
	{
		warning("Starting Energy Behavior Selection");
		int minEnergy = 8;
		int maxEnergy = 14;

		if (dominantMode == ENERGY_MODE) {
			eneCon = 3;
			maxEnergy = 21;
		} else {
			eneCon = 5;
		}


		// loop through energy pool array
		int energyPoolScummArray = getEnergyPoolsArray();
		int numPools = getNumberOfPools();

		// Prevent energy from being chosen if not enough energy to create
		int energyAmount = getPlayerEnergy();

		if ((energyAmount < 7))
			numPools = 0;

		for (int i = 1; i <= numPools; i++) {
			int poolX = _vm->_moonbase->readFromArray(energyPoolScummArray, i, ENERGY_POOL_X);
			int poolY = _vm->_moonbase->readFromArray(energyPoolScummArray, i, ENERGY_POOL_Y);

			int radius = energyPoolSize(i) / 2;
			int poolMaxCount = getMaxCollectors(i);

			int energyCount = 0;
			int energyUnits = getUnitsWithinRadius(poolX, poolY, radius + 30);
			int energyCounter = 0;
			int energyBuilding = _vm->_moonbase->readFromArray(energyUnits, 0, energyCounter);


			while (energyBuilding) {
				energyCounter++;

				if ((getBuildingType(energyBuilding) == BUILDING_ENERGY_COLLECTOR) && (getBuildingOwner(energyBuilding) != currentPlayer))
					energyCount = poolMaxCount;

				if ((getBuildingType(energyBuilding) == BUILDING_ENERGY_COLLECTOR) && (getBuildingOwner(energyBuilding) == currentPlayer))
					energyCount++;

				energyBuilding = _vm->_moonbase->readFromArray(energyUnits, 0, energyCounter);
			}

			_vm->nukeArray(energyUnits);

			if (energyCount < poolMaxCount) {
				int closestHub = getClosestUnit(poolX, poolY, 300, currentPlayer, 1, BUILDING_MAIN_BASE, 1);

				if (closestHub) {
					eneCon = MIN(1, eneCon);
				} else {
					int secondClosestHub = getClosestUnit(poolX, poolY, 480, currentPlayer, 1, BUILDING_MAIN_BASE, 1);

					if (secondClosestHub)
						eneCon = MIN(2, eneCon);
					else
						eneCon = MIN(3, eneCon);
				}
			}
		}

		int totalEnergy = estimateNextRoundEnergy(currentPlayer);

		if (totalEnergy < minEnergy)
			eneCon--;

		if (totalEnergy > maxEnergy)
			eneCon += 2;

		if ((totalEnergy > 34) || (!numPools))
			eneCon = 10;

		if (dominantMode == ENERGY_MODE)
			eneCon--;
	}


	// offense mode
	{
		warning("Starting Offense Behavior Selection");

		if (dominantMode == OFFENSE_MODE) offCon = 3;
		else offCon = 5;

		int enemyArray = getEnemyUnitsVisible(currentPlayer);
		int enemyX = 0;
		int enemyY = 0;
		int hubX = 0;
		int hubY = 0;

		int nearEnemyHub = 0;

		// cycle through the array of buildings
		for (int i = 0; i < 500; i++) {
			if (int thisBuilding = _vm->_moonbase->readFromArray(enemyArray, i, 0)) {
				enemyX = getHubX(thisBuilding);
				enemyY = getHubY(thisBuilding);
				int closestHub = getClosestUnit(enemyX, enemyY, 970, currentPlayer, 1, BUILDING_MAIN_BASE, 1);
				int closestOL = getClosestUnit(enemyX, enemyY, 970, currentPlayer, 1, BUILDING_MAIN_BASE, 1);
				int dist = 0;

				if (closestOL) {
					hubX = getHubX(closestOL);
					hubY = getHubY(closestOL);
					nearEnemyHub = 1;
				}

				if (closestHub) {
					hubX = getHubX(closestHub);
					hubY = getHubY(closestHub);
					dist = getDistance(hubX, hubY, enemyX, enemyY);

					if (dist < 480)
						nearEnemyHub = 1;
				}


				if (closestHub || closestOL) {
					int numDefenders = 0;
					int defArray = getUnitsWithinRadius(enemyX, enemyY, 170);
					int defCounter = 0;
					int defenseBuilding = _vm->_moonbase->readFromArray(defArray, 0, defCounter);

					while (defenseBuilding) {
						defCounter++;

						if (((getBuildingType(defenseBuilding) == BUILDING_ANTI_AIR) ||
								(getBuildingType(defenseBuilding) == BUILDING_SHIELD)) &&
									(getBuildingOwner(defenseBuilding) != currentPlayer)) {
							if (getBuildingState(defenseBuilding) == 0)
								numDefenders++;
						}

						defenseBuilding = _vm->_moonbase->readFromArray(defArray, 0, defCounter);
					}

					_vm->nukeArray(defArray);

					if (!numDefenders) {
						int defArray2 = getUnitsWithinRadius(hubX, hubY, 170);
						defCounter = 0;
						int defenseBuilding2 = _vm->_moonbase->readFromArray(defArray2, 0, defCounter);

						while (defenseBuilding2) {
							defCounter++;

							if (((getBuildingType(defenseBuilding2) == BUILDING_ANTI_AIR) ||
									(getBuildingType(defenseBuilding2) == BUILDING_SHIELD)) &&
										(getBuildingOwner(defenseBuilding2) != currentPlayer))
								if (getBuildingState(defenseBuilding2) == 0)
									numDefenders++;

							defenseBuilding2 = _vm->_moonbase->readFromArray(defArray, 0, defCounter);
						}

						_vm->nukeArray(defArray2);

					}

					if ((!numDefenders) && (nearEnemyHub)) {
						if (thisBuilding > 495)
							offCon = 1 + _vm->_rnd.getRandomNumber(1);
						else
							offCon = MIN(offCon, 2);
					} else {
						if (thisBuilding > 495) {
							if (nearEnemyHub) {
								offCon = MIN(offCon, 2);
								break;
							} else {
								offCon = MIN(offCon, 3);
								break;
							}
						} else {
							offCon = MIN(offCon, 4);
						}
					}
				}

				if (getBuildingType(thisBuilding) == BUILDING_CRAWLER) {
					if (getClosestUnit(enemyX, enemyY, 350, currentPlayer, 1, 0, 0)) {
						int closestHub1 = getClosestUnit(enemyX, enemyY, 460, currentPlayer, 1, BUILDING_MAIN_BASE, 1);
						int closestMine = getClosestUnit(enemyX, enemyY, 80, 0, 0, BUILDING_EXPLOSIVE_MINE, 1);

						if (closestHub1 && !closestMine)
							offCon = -1;
					}
				}
			} else {
				break;
			}
		}

		_vm->nukeArray(enemyArray);
		offCon--;
	}

	// defense mode
	{
		warning("Starting Defense Behavior Selection");

		if (dominantMode == DEFENSE_MODE)
			defCon = 3;
		else
			defCon = 5;

		int numDefenders = 0;
		int openFlag = 0;

		int mainHubX = getHubX(0);
		int mainHubY = getHubY(0);
		int mainHub = getClosestUnit(mainHubX + 10, mainHubY, 20, currentPlayer, 1, BUILDING_MAIN_BASE, 0);

		int damageFlag = 0;

		// cycle through the array of buildings
		for (int i = 500; i >= 1; i--) {
			if ((i < 497) && (defCon < 3))
				break;

			if (getBuildingOwner(i) == currentPlayer) {
				int type = getBuildingType(i);
				int hubX = getHubX(i);
				int hubY = getHubY(i);

				if (type == BUILDING_MAIN_BASE || type == BUILDING_ENERGY_COLLECTOR || type == BUILDING_OFFENSIVE_LAUNCHER) {
					int nearEnemy = 0;
					int closeBuildingsArray = getUnitsWithinRadius(hubX, hubY, 480);
					int closeBuildingCounter = 0;
					int closeBuilding = _vm->_moonbase->readFromArray(closeBuildingsArray, 0, closeBuildingCounter);

					while (closeBuilding) {
						closeBuildingCounter++;

						if ((getBuildingOwner(closeBuilding) != currentPlayer) && (getBuildingType(closeBuilding) == BUILDING_MAIN_BASE)) {
							nearEnemy = 1;
							break;
						}

						closeBuilding = _vm->_moonbase->readFromArray(closeBuildingsArray, 0, closeBuildingCounter);
					}

					_vm->nukeArray(closeBuildingsArray);

					int defCounter = 0;
					int defArray = getUnitsWithinRadius(hubX, hubY, 170);
					int defenseBuilding = _vm->_moonbase->readFromArray(defArray, 0, defCounter);
					numDefenders = 0;

					while (defenseBuilding) {
						defCounter++;

						if (((getBuildingType(defenseBuilding) == BUILDING_ANTI_AIR) || (getBuildingType(defenseBuilding) == BUILDING_SHIELD)) && (getBuildingOwner(defenseBuilding) == currentPlayer)) {
							//main base has enemies near, but is defended
							if (getBuildingState(defenseBuilding) == 0)
								numDefenders++;
						}

						defenseBuilding = _vm->_moonbase->readFromArray(defArray, 0, defCounter);
					}

					_vm->nukeArray(defArray);

					if (numDefenders > 2)
						defCon++;

					if (numDefenders < 2)
						if (dominantMode == DEFENSE_MODE)
							openFlag = 1;

					if (!numDefenders) {
						if (nearEnemy) {
							if (i == mainHub) {
								defCon = 1;
								break;
							} else {
								defCon = MIN(defCon, 2);
							}
						} else {
							if (i == mainHub)
								defCon = MIN(defCon, 3);
							else
								defCon = MIN(defCon, 4);
						}
					}

					if (getBuildingArmor(i) < getBuildingMaxArmor(i))
						damageFlag = 1;
				}
			}
		}

		if (damageFlag && (defCon > 1))
			defCon--;

		if (!openFlag && defCon == 3)
			defCon += 2;
	}

	warning("%s-------------------------------> Energy: %d     Offense: %d     Defense: %d", AItype[currentPlayer]->getNameString(), eneCon, offCon, defCon);

	if (dominantMode == DEFENSE_MODE)
		if ((defCon <= offCon) && (defCon <= eneCon))
			return DEFENSE_MODE;

	if (dominantMode == OFFENSE_MODE)
		if ((offCon <= eneCon) && (offCon <= defCon))
			return OFFENSE_MODE;

	if (dominantMode == ENERGY_MODE)
		if ((eneCon <= offCon) && (eneCon <= defCon))
			return ENERGY_MODE;

	if ((defCon <= offCon) && (defCon <= eneCon))
		return DEFENSE_MODE;

	if ((offCon <= eneCon) && (offCon <= defCon))
		return OFFENSE_MODE;

	if ((eneCon <= offCon) && (eneCon <= defCon))
		return ENERGY_MODE;

	return -1;
}

int chooseTarget(int behavior1) {
	int numPools = getNumberOfPools();
	int currentPlayer = getCurrentPlayer();

	int selection = 0;
	int selectionValues[50] = {0};
	int selectionDist = 10000000;

	if (behavior1 == ENERGY_MODE) {
		// loop through energy pool array
		int energyPoolScummArray = getEnergyPoolsArray();

		for (int i = 1; i <= numPools; i++) {
			// check # units on pool
			int numPoolSpots = _vm->_moonbase->readFromArray(energyPoolScummArray, i, ENERGY_POOL_UNITS_ON);

			if (numPoolSpots == 0) {
				// put this one in the middle
				warning("Empty pool #%d", i);
				selectionValues[i] = 2;
			} else {
				// get units w/in radius of pool
				int poolUnitsArray = getUnitsWithinRadius(_vm->_moonbase->readFromArray(energyPoolScummArray, i, ENERGY_POOL_X), _vm->_moonbase->readFromArray(energyPoolScummArray, i, ENERGY_POOL_Y), 50);
				int enemyPool = 0;
				int j = 1;
				int thisPoolUnit = _vm->_moonbase->readFromArray(poolUnitsArray, 0, j);

				while (thisPoolUnit) {
					if (getBuildingOwner(thisPoolUnit) != currentPlayer)
						enemyPool = 1;

					j++;
					thisPoolUnit = _vm->_moonbase->readFromArray(poolUnitsArray, 0, j);
				}

				_vm->nukeArray(poolUnitsArray);

				// if enemy collector, put at bottom
				if (enemyPool) {
					selectionValues[i] = 1;
				} else if (numPoolSpots < getMaxCollectors(i)) {
					selectionValues[i] = 3;
				} else {
					// this pool is filled
					selectionValues[i] = 0;
				}
			}

			if (selectionValues[i] > selectionValues[selection]) {
				selection = i;
			} else if (selectionValues[i] == selectionValues[selection]) {
				int poolX = _vm->_moonbase->readFromArray(energyPoolScummArray, i, ENERGY_POOL_X);
				int poolY = _vm->_moonbase->readFromArray(energyPoolScummArray, i, ENERGY_POOL_Y);

				int closestHub = getClosestUnit(poolX, poolY, getMaxX(), currentPlayer, 1, BUILDING_MAIN_BASE, 1, 100);
				int thisDist = getDistance(poolX, poolY, getHubX(closestHub), getHubY(closestHub));

				if (thisDist < selectionDist) {
					selection = i;
					selectionDist = thisDist;
				}
			}

		}

		warning("Pool selected: %d   dist: %d", selection, selectionDist);
		return selection;
	}

	if (behavior1 == OFFENSE_MODE) {
		int returnBuilding = 0;
		int attackableArray[500];
		int nearAttackableArray[500];
		int attackableIndex = 0;
		int nearAttackableIndex = 0;

		int enemyArray = getEnemyUnitsVisible(currentPlayer);

		for (int i = 500; i >= 1; i--) {
			int thisBuilding = _vm->_moonbase->readFromArray(enemyArray, i - 1, 0);

			if (thisBuilding) {
				if (getBuildingType(thisBuilding) == BUILDING_CRAWLER) {
					if ((getTerrain(getHubX(thisBuilding), getHubY(thisBuilding)) != TERRAIN_TYPE_WATER) || (getPlayerEnergy() > 6)) {
						if (getClosestUnit(getHubX(thisBuilding), getHubY(thisBuilding), 380, currentPlayer, 1, BUILDING_MAIN_BASE, 1)) {
							returnBuilding = thisBuilding;
							_vm->nukeArray(enemyArray);
							return returnBuilding;
						}
					} else {
						continue;
					}
				}

				int enemyX = getHubX(thisBuilding);
				int enemyY = getHubY(thisBuilding);
				int closestHub = getClosestUnit(enemyX, enemyY, 930, currentPlayer, 1, BUILDING_MAIN_BASE, 1);

				int dist = getDistance(enemyX, enemyY, getHubX(closestHub), getHubY(closestHub));

				if (getBuildingType(thisBuilding) != BUILDING_BALLOON) {
					if (dist < 470) {
						attackableArray[attackableIndex] = thisBuilding;
						attackableIndex++;
					} else {
						nearAttackableArray[nearAttackableIndex] = thisBuilding;
						nearAttackableIndex++;
					}
				}
			}
		}

		_vm->nukeArray(enemyArray);

		if (attackableIndex) {
			int thisWorth = 0;
			int savedWorth = 1;
			int closestSavedShield = 0;
			int closestSavedAntiAir = 0;

			for (int i = 0; i < attackableIndex; i++) {
				thisWorth = getBuildingWorth(attackableArray[i]);

				if (thisWorth == savedWorth) {
					int closestShield = getClosestUnit(getHubX(attackableArray[i]), getHubY(attackableArray[i]), 180, currentPlayer, 0, BUILDING_SHIELD, 1);
					int closestAntiAir = getClosestUnit(getHubX(attackableArray[i]), getHubY(attackableArray[i]), 180, currentPlayer, 0, BUILDING_ANTI_AIR, 1);

					if (closestSavedShield > closestShield) {
						savedWorth = thisWorth;
						closestSavedShield = closestShield;
						closestSavedAntiAir = closestAntiAir;
						returnBuilding = attackableArray[i];
					} else {
						if (closestSavedAntiAir > closestAntiAir) {
							savedWorth = thisWorth;
							closestSavedShield = closestShield;
							closestSavedAntiAir = closestAntiAir;
							returnBuilding = attackableArray[i];
						}
					}
				}

				if (thisWorth > savedWorth) {
					savedWorth = thisWorth;
					returnBuilding = attackableArray[i];
				}
			}
		} else {
			if (nearAttackableIndex) {
				int thisWorth = 0;
				int savedWorth = 1;
				int closestSavedShield = 0;
				int closestSavedAntiAir = 0;

				for (int i = 0; i < nearAttackableIndex; i++) {
					thisWorth = getBuildingWorth(nearAttackableArray[i]);

					if (thisWorth == savedWorth) {
						int closestShield = getClosestUnit(getHubX(nearAttackableArray[i]), getHubY(nearAttackableArray[i]), 180, currentPlayer, 0, BUILDING_SHIELD, 1);
						int closestAntiAir = getClosestUnit(getHubX(nearAttackableArray[i]), getHubY(nearAttackableArray[i]), 180, currentPlayer, 0, BUILDING_ANTI_AIR, 1);

						if (closestSavedShield > closestShield) {
							savedWorth = thisWorth;
							closestSavedShield = closestShield;
							closestSavedAntiAir = closestAntiAir;
							returnBuilding = nearAttackableArray[i];
						} else {
							if (closestSavedAntiAir > closestAntiAir) {
								savedWorth = thisWorth;
								closestSavedShield = closestShield;
								closestSavedAntiAir = closestAntiAir;
								returnBuilding = nearAttackableArray[i];
							}
						}
					}

					if (thisWorth > savedWorth)  {
						savedWorth = thisWorth;
						returnBuilding = nearAttackableArray[i];
					}
				}
			}
		}

		if (!returnBuilding) {
			for (int i = 500; i > 496; i--) {
				if (getBuildingOwner(i)) {
					if (getBuildingTeam(i) != getPlayerTeam(currentPlayer)) {
						returnBuilding = i;
						i = 0;
					}
				}
			}
		}

		warning("Attack target: %d", returnBuilding);

		assert(returnBuilding);
		return returnBuilding;
	}

	if (behavior1 == DEFENSE_MODE) {
		int returnBuilding = 0;

		int savedTally = 0;
		int savedDamage;
		float savedNumDefenses = 0;
		int savedWorth = 0;

		float numDefenses = 0;
		int tally = 0;
		int attackable = 0;
		int attacked = 0;
		int damage = 0;

		int type = 0;
		int worth = 0;


		int attackedX = 0;
		int attackedY = 0;
		int attackedUnit = 0;

		if (getLastAttacked(attackedX, attackedY)) {
			(void)getClosestUnit(attackedX + 10, attackedY, 50, currentPlayer, 1, 0, 0); // Unused?
		}

		// loop through own units
		for (int i = 1; i <= 500; i++) {
			numDefenses = 0;
			attackable = 0;
			attacked = 0;
			damage = 0;
			type = 0;
			worth = 0;

			int owner = getBuildingOwner(i);

			if (owner == currentPlayer) {
				type = getBuildingType(i);

				// if current unit in [hub, offense, energy, tower]
				if ((type == BUILDING_MAIN_BASE) || (type == BUILDING_ENERGY_COLLECTOR) || (type == BUILDING_OFFENSIVE_LAUNCHER) || (type == BUILDING_TOWER)) {
					worth = getBuildingWorth(i);

					// Calculate current defenses
					int x = getHubX(i);
					int y = getHubY(i);
					assert(x >= 0);
					assert(y >= 0);

					int defenseArray = getUnitsWithinRadius(x, y, 180);
					int j = 0;
					// cycle through the defense units close to the target building
					int defenseBuilding = _vm->_moonbase->readFromArray(defenseArray, 0, j);

					// loop on all defenses w/in 180
					while (defenseBuilding != 0) {
						int defenseType = getBuildingType(defenseBuilding);

						// sub for each
						if ((defenseType == BUILDING_ANTI_AIR) || (defenseType == BUILDING_SHIELD)) {
							if (getBuildingState(defenseBuilding) == 0)
								numDefenses += 1;
							else
								numDefenses += .5;
						}

						j++;
						defenseBuilding = _vm->_moonbase->readFromArray(defenseArray, 0, j);
					}

					_vm->nukeArray(defenseArray);

					// Calculate if this unit is attackable
					// get dist to nearest enemy hub, offense
					int closestHub = getClosestUnit(x, y, getMaxX(), getCurrentPlayer(), 0, BUILDING_MAIN_BASE, 0);
					int numStridesToHub = getDistance(getHubX(closestHub), getHubY(closestHub), x, y) / 480;
					closestHub = getClosestUnit(x, y, getMaxX(), getCurrentPlayer(), 0, BUILDING_OFFENSIVE_LAUNCHER, 0);
					int numStridesToOL = getDistance(getHubX(closestHub), getHubY(closestHub), x, y) / 800;

					// sub for each stride away
					if (!numStridesToOL || !numStridesToHub)
						attackable = 1;

					// Check if this unit was just attacked
					if (attackedUnit == i)
						attacked = 1;

					if (!numDefenses) {
						tally = 1;

						if (attackable) {
							tally = 4;

							if (attacked) {
								tally = 5;
							}
						}
					} else {
						if (attackable) {
							tally = 2;

							if (attacked) {
								tally = 3;
							}
						}
					}

					// Check if this unit is damaged
					int saveFlag = 0;

					if (tally > savedTally) {
						saveFlag = 1;
					} else {
						if (tally == savedTally) {
							if (worth > savedWorth) {
								saveFlag = 1;

								if (numDefenses > savedNumDefenses) {
									saveFlag = 0;
								}
							}

							if (damage > savedDamage) {
								saveFlag = 1;

								if (numDefenses > savedNumDefenses) {
									saveFlag = 0;
								}
							}

							if (numDefenses < savedNumDefenses) {
								saveFlag = 1;
							}

							if (numDefenses >= 3) {
								saveFlag = 0;
							}
						}
					}

					if (saveFlag) {
						savedTally = tally;
						savedWorth = worth;
						savedDamage = damage;
						savedNumDefenses = numDefenses;
						returnBuilding = i;
					}
				}
			}
		}

		return returnBuilding;
	}

	return 0;
}

Tree *initApproachTarget(int targetX, int targetY, Node **retNode) {
	int sourceHub = 0;

	if (behavior == 2)
		sourceHub = getClosestUnit(targetX + 10, targetY, getMaxX(), getCurrentPlayer(), 1, BUILDING_MAIN_BASE, 1);
	else
		sourceHub = getClosestUnit(targetX + 10, targetY, getMaxX(), getCurrentPlayer(), 1, BUILDING_MAIN_BASE, 1, MIN_DIST);

	Traveller *myTraveller = new Traveller(getHubX(sourceHub), getHubY(sourceHub));
	myTraveller->setSourceHub(sourceHub);

	//target adjustment so that room is allowed for the appropriate shot
	int tempAngle = calcAngle(getHubX(sourceHub), getHubY(sourceHub), targetX, targetY);
	int adjX = -120 * cos(degToRad(tempAngle));
	int adjY = -120 * sin(degToRad(tempAngle));

	Traveller::setTargetPosX(targetX + adjX);
	Traveller::setTargetPosY(targetY + adjY);
	Traveller::setMaxDist(340);

	Tree *myTree = new Tree(myTraveller, TREE_DEPTH);
	*retNode = myTree->aStarSearch_singlePassInit();

	return myTree;
}

int *approachTarget(Tree *myTree, int &xTarget, int &yTarget, Node **currentNode) {
	int *retVal = NULL;

	*currentNode = NULL;
	Node *retNode = myTree->aStarSearch_singlePass(currentNode);

	if (*currentNode != NULL)
		warning("########################################### Got a possible solution");

	if (myTree->IsBaseNode(retNode)) {
		warning("########################################### Returning Base Node");
		retVal = new int[4];
		retVal[0] = -1;
		return retVal;
	}

	if (retNode == NULL) {
		return retVal;
	} else {
		retVal = new int[4];

		Traveller *retTraveller = reinterpret_cast<Traveller *>(retNode->getFirstStep()->getContainedObject());

		// set launching hub, item to launch, angle of launch, power of launch
		// if water flag is set, launch bridge instead of hub
		retVal[0] = static_cast<Traveller *>(myTree->getBaseNode()->getContainedObject())->getSourceHub();

		if (retTraveller->getWaterFlag()) {
			int powAngle = getPowerAngleFromPoint(retTraveller->getWaterSourceX(),
												retTraveller->getWaterSourceY(),
												retTraveller->getWaterDestX(),
												retTraveller->getWaterDestY(),
												15);

			powAngle = abs(powAngle);
			int power = powAngle / 360;
			int angle = powAngle - (power * 360);

			retVal[0] = getClosestUnit(retTraveller->getWaterSourceX() + 10, retTraveller->getWaterSourceY(), getMaxX(), getCurrentPlayer(), 1, BUILDING_MAIN_BASE, 1, 0);

			retVal[1] = ITEM_BRIDGE;
			retVal[2] = angle;
			retVal[3] = power;

			warning("Target Bridge Coords: <%d, %d> ", retTraveller->getWaterDestX(), retTraveller->getWaterDestY());
		} else {
			retVal[1] = ITEM_HUB;
			retVal[2] = retTraveller->getAngleTo();
			retVal[3] = retTraveller->getPowerTo();
		}


		int whoseTurn = getCurrentPlayer();

		if ((lastXCoord[whoseTurn]).size() >= MAX_MEMORY) {
			(lastXCoord[whoseTurn]).erase((lastXCoord[whoseTurn]).begin());
			(lastYCoord[whoseTurn]).erase((lastYCoord[whoseTurn]).begin());
		}

		(lastXCoord[whoseTurn]).push_back(retTraveller->getPosX());
		(lastYCoord[whoseTurn]).push_back(retTraveller->getPosY());

		int temp = static_cast<int>(retTraveller->calcT());
		int temp2 = static_cast<int>(retTraveller->getValueG());
		int x = static_cast<int>(retTraveller->getPosX());
		int y = static_cast<int>(retTraveller->getPosY());
		warning("Target Coords: <%d, %d>  G-value: %d    T-value: %d", x, y, temp2, temp);
		xTarget = x;
		yTarget = y;
	}

	return retVal;
}

Tree *initAcquireTarget(int targetX, int targetY, Node **retNode) {
	int sourceHub = getClosestUnit(targetX, targetY, getMaxX(), getCurrentPlayer(), 1, BUILDING_MAIN_BASE, 1, MIN_DIST);
	warning("My coords (%d): %d %d", sourceHub, getHubX(sourceHub), getHubY(sourceHub));

	Sortie::setSourcePos(getHubX(sourceHub), getHubY(sourceHub));
	Sortie::setTargetPos(targetX, targetY);
	Sortie *myBaseTarget = new Sortie();
	myBaseTarget->setValueG(0);

	myBaseTarget->setUnitType(ITEM_BOMB);
	myBaseTarget->setShotPos(-1, -1);

	int unitsArray = getUnitsWithinRadius(targetX + 7, targetY, 211);

	warning("Target Coords: <%d, %d>    Source Coords: <%d, %d>", targetX, targetY, getHubX(sourceHub) , getHubY(sourceHub));

	myBaseTarget->setEnemyDefenses(unitsArray, targetX, targetY);

	int thisElement = _vm->_moonbase->readFromArray(unitsArray, 0, 0);

	_vm->nukeArray(unitsArray);

	if (!thisElement) {
		delete myBaseTarget;
		return NULL;
	}

	Tree *myTree = new Tree(myBaseTarget, 4);
	*retNode = myTree->aStarSearch_singlePassInit();

	return myTree;
}


int *acquireTarget(int targetX, int targetY, Tree *myTree, int &errorCode) {
	int currentPlayer = getCurrentPlayer();
	int *retVal = NULL;

	Node *currentNode = NULL;
	Node *retNode = myTree->aStarSearch_singlePass(&currentNode);

	if (myTree->IsBaseNode(retNode))
		return acquireTarget(targetX, targetY);

	if (retNode == NULL) {
		errorCode = 0;
		return retVal;
	}

	Sortie *thisSortie = static_cast<Sortie *>(retNode->getFirstStep()->getContainedObject());
	int unitToShoot = thisSortie->getUnitType();

	if (unitToShoot < 0) {
		errorCode = 1;
		return retVal;
	}

	if (unitToShoot == ITEM_CRAWLER) {
		warning("target acquisition is launching a crawler");
	}

	int shotTargetX = thisSortie->getShotPosX();
	int shotTargetY = thisSortie->getShotPosY();
	int theTarget = getClosestUnit(shotTargetX + 5, shotTargetY, getMaxX(), 0, 0, 0, 0, 0);


	int sourceOL = 0;
	int sourceX = thisSortie->getSourcePosX();
	int sourceY = thisSortie->getSourcePosY();

	int sourceHub = getClosestUnit(sourceX + 5, sourceY, getMaxX(), currentPlayer, 1, BUILDING_MAIN_BASE, 1, 0);

	sourceOL = getClosestUnit(sourceX, sourceY, 900, currentPlayer, 1, BUILDING_OFFENSIVE_LAUNCHER, 1, 110);

	if (sourceOL) {
		sourceHub = sourceOL;
		sourceX = getHubX(sourceOL);
		sourceY = getHubY(sourceOL);
	}

	if (!sourceHub) sourceHub = getClosestUnit(sourceX + 5, sourceY, getMaxX(), currentPlayer, 1, BUILDING_MAIN_BASE, 1, 0);

	int powAngle = getPowerAngleFromPoint(sourceX, sourceY, shotTargetX, shotTargetY, 15, sourceOL);
	warning("The source (%d: <%d, %d>)    The target (%d: <%d, %d>)", sourceHub, sourceX, sourceY, theTarget, shotTargetX, shotTargetY);

	powAngle = abs(powAngle);
	int power = powAngle / 360;
	int angle = powAngle - (power * 360);

	retVal = new int[4];

	retVal[0] = sourceHub;
	retVal[1] = unitToShoot;
	retVal[2] = angle;
	retVal[3] = power;

	warning("Unit to shoot: %d", unitToShoot);

	return retVal;
}

int *acquireTarget(int targetX, int targetY) {
	int *retVal = new int[4];
	int sourceHub = getClosestUnit(targetX, targetY, getMaxX(), getCurrentPlayer(), 1, BUILDING_MAIN_BASE, 1, 110);

	if (!sourceHub)
		sourceHub = getClosestUnit(targetX, targetY, getMaxX(), getCurrentPlayer(), 1, BUILDING_MAIN_BASE, 1, 0);

	int directAngle = calcAngle(getHubX(sourceHub), getHubY(sourceHub), targetX, targetY);
	int directDistance = getDistance(getHubX(sourceHub), getHubY(sourceHub), targetX, targetY);

	retVal[0] = sourceHub;
	retVal[1] = ITEM_OFFENSE;
	retVal[2] = directAngle;
	retVal[3] = MAX(MIN(getMaxPower() * directDistance / 500, getMaxPower()), getMinPower());

	return retVal;
}

int *energizeTarget(int &targetX, int &targetY, int index) {
	int n = 10;
	static int currentPlayer = 0;
	static int pool = 0;
	static int radius = 0;
	static int poolUnitsArray = 0;
	static int j = 0;
	static int k = 0;
	static int sameUnit = 0;
	static int nextUnit = 0;
	static int attempt = 0;

	if (!index) {
		warning("index is 0!");
		currentPlayer = getCurrentPlayer();
		pool = 0;

		// get the pool that's closest to the target coords
		for (int i = 1; i <= getNumberOfPools(); i++) {
			int poolX = _vm->_moonbase->readFromArray(getEnergyPoolsArray(), i, ENERGY_POOL_X);
			int poolY = _vm->_moonbase->readFromArray(getEnergyPoolsArray(), i, ENERGY_POOL_Y);

			if ((poolX == targetX) && (poolY == targetY)) {
				pool = i;
			}
		}

		// calculate the appropriate radius
		radius = energyPoolSize(pool) / 2;

		// create test points
		k = 0;
		j = 0;
		nextUnit = 0;
		sameUnit = 0;
		attempt = 0;
	}

	if (poolUnitsArray)
		_vm->nukeArray(poolUnitsArray);

	poolUnitsArray = getUnitsWithinRadius(targetX, targetY, 450);
	assert(poolUnitsArray);

	// 0 is for energy collectors, 1 is for circumnavigating hubs
	if (k < 2) {
		if (!sameUnit) {
			sameUnit = 1;
			attempt = 0;
			nextUnit = _vm->_moonbase->readFromArray(poolUnitsArray, 0, j);
			j++;
		}

		if (nextUnit) {
			if ((getBuildingType(nextUnit) == BUILDING_MAIN_BASE) && (getBuildingOwner(nextUnit) == currentPlayer)) {
				int minAngle = 0;
				int hubToPoolAngle = 0;
				int testAngle = 0;
				int testDist = 0;
				static int xPos = 0;
				static int yPos = 0;
				static int newAttempt = 1;

				if (sameUnit) {
					if (k == 0) {
						int poolToHubAngle = calcAngle(targetX, targetY, getHubX(nextUnit), getHubY(nextUnit));
						minAngle = poolToHubAngle - 45;
					} else {
						hubToPoolAngle = calcAngle(getHubX(nextUnit), getHubY(nextUnit), targetX, targetY);
					}
				}

				if (attempt < n) {
					static int power = 0;
					static int angle = 0;

					if (newAttempt) {
						newAttempt = 0;

						if (k == 0) {
							testAngle = (_vm->_rnd.getRandomNumber(89) + minAngle) % 360;
							testDist = radius;

							xPos = targetX + testDist * cos(degToRad(testAngle));
							yPos = targetY + testDist * sin(degToRad(testAngle));
						} else {
							switch (_vm->_rnd.getRandomNumber(1)) {
							case 0:
								testAngle = (hubToPoolAngle + (45 + _vm->_rnd.getRandomNumber(19))) % 360;
								break;

							default:
								testAngle = (hubToPoolAngle + (315 - _vm->_rnd.getRandomNumber(19))) % 360;
								break;
							}

							testDist = ((((n - attempt) / n) * .5) + .5) * (getDistance(getHubX(nextUnit), getHubY(nextUnit), targetX, targetY) / .8);
							xPos = getHubX(nextUnit) + testDist * cos(degToRad(testAngle));
							yPos = getHubY(nextUnit) + testDist * sin(degToRad(testAngle));
						}

						// check if points are good
						int powAngle = getPowerAngleFromPoint(getHubX(nextUnit), getHubY(nextUnit), xPos, yPos, 15);
						powAngle = abs(powAngle);

						power = powAngle / 360;
						angle = powAngle - (power * 360);
					}

					int result = 0;
					result = simulateBuildingLaunch(getHubX(nextUnit), getHubY(nextUnit), power, angle, 10, 1);

					if (result) {
						newAttempt = 1;

						if (result > 0) {
							xPos = (xPos + getMaxX()) % getMaxX();
							yPos = (yPos + getMaxY()) % getMaxY();

							result = 1;
						} else {
							// Drop a bridge for the cord
							int yCoord  = -result / getMaxX();
							int xCoord = -result - (yCoord * getMaxX());

							if (checkIfWaterState(xCoord, yCoord)) {
								int terrainSquareSize = getTerrainSquareSize();
								xCoord = ((xCoord / terrainSquareSize * terrainSquareSize) + (terrainSquareSize / 2));
								yCoord = ((yCoord / terrainSquareSize * terrainSquareSize) + (terrainSquareSize / 2));

								int xDist = xCoord - xPos;
								int yDist = yCoord - yPos;
								xPos = xCoord + (terrainSquareSize * 1.414 * (xDist / (abs(xDist) + 1)));
								yPos = yCoord + (terrainSquareSize * 1.414 * (yDist / (abs(yDist) + 1)));

								nextUnit = getClosestUnit(xPos, yPos, 480, getCurrentPlayer(), 1, BUILDING_MAIN_BASE, 1, 120);
								int powAngle = getPowerAngleFromPoint(getHubX(nextUnit), getHubY(nextUnit), xPos, yPos, 15);

								powAngle = abs(powAngle);
								power = powAngle / 360;
								angle = powAngle - (power * 360);

								int *retVal = new int[4];

								retVal[0] = nextUnit;
								retVal[1] = ITEM_BRIDGE;
								retVal[2] = angle;
								retVal[3] = power;

								if (nextUnit <= 0)
									retVal[0] = 0;

								_vm->nukeArray(poolUnitsArray);
								poolUnitsArray = 0;
								return retVal;
							}
						}

						if (result > 0) {
							_vm->nukeArray(poolUnitsArray);
							poolUnitsArray = 0;

							targetX = xPos;
							targetY = yPos;

							int *retVal = new int[4];

							retVal[0] = nextUnit;

							if (k == 0) {
								retVal[1] = ITEM_ENERGY;
							} else {
								retVal[1] = ITEM_HUB;
							}

							retVal[2] = angle;
							retVal[3] = power;
							return retVal;
						}
					} else {
						int *retVal = new int[4];
						retVal[0] = 0;
						_vm->nukeArray(poolUnitsArray);
						poolUnitsArray = 0;

						return retVal;
					}

					attempt++;
				} else {
					sameUnit = 0;
				}
			} else {
				sameUnit = 0;
			}
		} else {
			sameUnit = 0;
			k++;
			j = 0;
		}
	} else {
		_vm->nukeArray(poolUnitsArray);
		poolUnitsArray = 0;
		return NULL;
	}

	_vm->nukeArray(poolUnitsArray);
	poolUnitsArray = 0;
	int *retVal = new int[4];
	retVal[0] = 0;

	return retVal;
}

int *offendTarget(int &targetX, int &targetY, int index) {
	int *retVal = NULL;

	int target = getClosestUnit(targetX + 10, targetY, 20, 0, 0, 0, 0);

	if (!target)
		target = getClosestUnit(targetX + 10, targetY, 0, 0, 0, 0, 0);

	warning("The target inside the offendTarget routine is: %d", target);
	int type = getBuildingType(target);
	int unit = 0;

	DefenseUnit *thisUnit;

	switch (type) {
	case BUILDING_OFFENSIVE_LAUNCHER:
		thisUnit = new OffenseUnit();
		break;

	case BUILDING_TOWER:
		thisUnit = new TowerUnit();
		break;

	case BUILDING_MAIN_BASE:
		thisUnit = new HubUnit();
		break;

	case BUILDING_ENERGY_COLLECTOR:
		thisUnit = new EnergyUnit();
		break;

	case BUILDING_CRAWLER:
		thisUnit = new CrawlerUnit();
		break;

	case BUILDING_BRIDGE:
		thisUnit = new BridgeUnit();
		break;

	case BUILDING_SHIELD:
		thisUnit = new ShieldUnit();
		break;

	default:
		thisUnit = new HubUnit();
		break;
	}

	thisUnit->setPos(targetX, targetY);
	thisUnit->setID(target);

	int sourceHub = getClosestUnit(targetX, targetY, getMaxX(), getCurrentPlayer(), 1, BUILDING_MAIN_BASE, 1, 110);
	int sourceOL = 0;
	sourceOL = getClosestUnit(targetX, targetY, 900, getCurrentPlayer(), 1, BUILDING_OFFENSIVE_LAUNCHER, 1, 110);

	unit = thisUnit->selectWeapon(_vm->_rnd.getRandomNumber(4));

	if (sourceOL) {
		if ((unit == ITEM_BOMB) || (unit == ITEM_CLUSTER) || (unit == ITEM_GUIDED) || (unit == ITEM_EMP) || (unit == ITEM_SPIKE) || (unit == ITEM_CRAWLER) || (unit == ITEM_VIRUS)) {
			sourceHub = sourceOL;
		}
	}

	if (!sourceHub) {
		retVal = new int[4];

		retVal[1] = SKIP_TURN;
		return retVal;
	}


	if ((thisUnit->getType() == BUILDING_CRAWLER) && (unit == SKIP_TURN)) {
		retVal = new int[4];
		retVal[1] = unit;
		delete thisUnit;
		return retVal;
	}

	if (unit == ITEM_CRAWLER) {
		warning("******** offense is launching a crawler ********");
		warning("The defensive unit is: %d", unit);
	}

	Common::Point *targetCoords;
	int dist = getDistance(getHubX(sourceHub), getHubY(sourceHub), targetX, targetY);
	targetCoords = thisUnit->createTargetPos(0, dist, unit, getHubX(sourceHub), getHubY(sourceHub));

	int powAngle = getPowerAngleFromPoint(getHubX(sourceHub), getHubY(sourceHub), targetCoords->x, targetCoords->y, 15, sourceOL);
	powAngle = abs(powAngle);
	int power = powAngle / 360;
	int angle = powAngle % 360;

	if (unit == ITEM_MINE)
		power -= 30;

	targetX = targetCoords->x;
	targetY = targetCoords->y;

	if (targetX < 0)
		targetX = (targetX + getMaxX()) % getMaxX();

	if (targetY < 0)
		targetY = (targetY + getMaxY()) % getMaxY();

	assert(targetX >= 0 && targetY >= 0);
	delete targetCoords;
	delete thisUnit;

	retVal = new int[4];

	retVal[0] = sourceHub;
	retVal[1] = unit;
	retVal[2] = angle;
	retVal[3] = power;

	return retVal;
}

int *defendTarget(int &targetX, int &targetY, int index) {
	int *retVal = NULL;
	Defender *thisDefender = new Defender;
	int defStatus = thisDefender->calculateDefenseUnitPosition(targetX, targetY, index);

	if (defStatus > 0) {
		targetX = thisDefender->getTargetX();
		targetY = thisDefender->getTargetY();
		retVal = new int[4];

		retVal[0] = thisDefender->getSourceUnit();
		retVal[1] = thisDefender->getUnit();
		retVal[2] = thisDefender->getAngle();
		retVal[3] = thisDefender->getPower();
	}

	if (defStatus == 0) {
		retVal = new int[4];
		retVal[0] = 0;
	}

	if (defStatus == -1) {
		if (thisDefender->getTargetX() || thisDefender->getTargetY()) {
			targetX = thisDefender->getTargetX();
			targetY = thisDefender->getTargetY();
		}

		retVal = new int[4];
		retVal[0] = thisDefender->getSourceUnit();
		retVal[1] = thisDefender->getUnit();
		retVal[2] = thisDefender->getAngle();
		retVal[3] = thisDefender->getPower();
	}

	if (defStatus == -3) {
		retVal = new int[4];
		retVal[0] = 0;
		retVal[1] = SKIP_TURN;
		retVal[2] = 0;
		retVal[3] = 0;
	}

	assert(targetX >= 0 && targetY >= 0);

	if (retVal[1] == ITEM_CRAWLER) {
		warning("defend target is launching a crawler");
	}

	delete thisDefender;
	return retVal;
}

int getClosestUnit(int x, int y, int radius, int player, int alignment, int unitType, int checkUnitEnabled) {
	assert((unitType >= 0) && (unitType <= 12));

	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_CLOSEST_UNIT], 7, x, y, radius, player, alignment, unitType, checkUnitEnabled);
	return retVal;
}

int getClosestUnit(int x, int y, int radius, int player, int alignment, int unitType, int checkUnitEnabled, int minDist) {
	assert((unitType >= 0) && (unitType <= 12));
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_CLOSEST_UNIT], 8, x, y, radius, player, alignment, unitType, checkUnitEnabled, minDist);
	return retVal;
}

int getDistance(int originX, int originY, int endX, int endY) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_WORLD_DIST], 4, originX, originY, endX, endY);
	return retVal;
}

int calcAngle(int originX, int originY, int endX, int endY) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_WORLD_ANGLE], 5, originX, originY, endX, endY, 0);
	return retVal;
}

int calcAngle(int originX, int originY, int endX, int endY, int noWrapFlag) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_WORLD_ANGLE], 5, originX, originY, endX, endY, noWrapFlag);
	return retVal;
}

int getTerrain(int x, int y) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_TERRAIN_TYPE], 2, x, y);
	return retVal;
}

int estimateNextRoundEnergy(int player) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_ESTIMATE_NEXT_ROUND_ENERGY], 1, player);
	return retVal / 10;
}

int getHubX(int hub) {
	assert(hub >= 0 && hub <= 500);
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_HUB_X, hub);
	return retVal;
}

int getHubY(int hub) {
	assert(hub >= 0 && hub <= 500);
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_HUB_Y, hub);
	return retVal;
}

int getMaxX() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_WORLD_X_SIZE);
	return retVal;
}

int getMaxY() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_WORLD_Y_SIZE);
	return retVal;
}

int getCurrentPlayer() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_CURRENT_PLAYER);
	assert(retVal != 0);
	return retVal;
}

int getMaxPower() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_MAX_POWER);
	return retVal;
}

int getMinPower() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_MIN_POWER);
	return retVal;
}

int getTerrainSquareSize() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_TERRAIN_SQUARE_SIZE);
	return retVal;
}

int getBuildingOwner(int building) {
	assert((building > 0) && (building < 501));
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_BUILDING_OWNER, building);
	return retVal;
}

int getBuildingState(int building) {
	assert((building > 0) && (building < 501));
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_BUILDING_STATE, building);
	return retVal;
}

int getBuildingType(int building) {
	assert((building > 0) && (building < 501));
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_BUILDING_TYPE, building);
	return retVal;
}

int getBuildingArmor(int building) {
	assert((building > 0) && (building < 501));
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_BUILDING_ARMOR, building);
	return retVal;
}

int getBuildingWorth(int building) {
	assert((building > 0) && (building < 501));
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_BUILDING_WORTH, building);
	return retVal;
}

int getEnergyPoolsArray() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_ENERGY_POOLS_ARRAY);
	return retVal;
}

int getCoordinateVisibility(int x, int y, int playerNum) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 4, D_GET_COORDINATE_VISIBILITY, x, y, playerNum);
	return retVal;
}

int getUnitVisibility(int unit, int playerNum) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 3, D_GET_UNIT_VISIBILITY, unit, playerNum);
	return retVal;
}

int getEnergyPoolVisibility(int pool, int playerNum) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 3, D_GET_ENERGY_POOL_VISIBILITY, pool, playerNum);
	return retVal;
}

int getNumberOfPools() {
	int retVal = 0;

	if (AItype[getCurrentPlayer()]->getID() == ENERGY_HOG) {
		retVal = 1;
	} else {
		retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_NUMBER_OF_POOLS);
	}

	return retVal;
}

int getNumberOfPlayers() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_NUMBER_OF_PLAYERS);
	return retVal;
}

int getPlayerEnergy() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_PLAYER_ENERGY);
	return static_cast<int>(static_cast<float>(retVal) / 10.0);
}

int getPlayerMaxTime() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_PLAYER_MAX_TIME);
	return retVal;
}

int getWindXSpeed() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_WIND_X_SPEED);
	return retVal;
}

int getWindYSpeed() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_WIND_Y_SPEED);
	return retVal;
}

int getTotalWindSpeed() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_TOTAL_WIND_SPEED);
	return retVal;
}

int getWindXSpeedMax() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_WIND_X_SPEED_MAX);
	return retVal;
}

int getWindYSpeedMax() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_WIND_Y_SPEED_MAX);
	return retVal;
}

int getBigXSize() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_BIG_X_SIZE);
	return retVal;
}

int getBigYSize() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_BIG_Y_SIZE);
	return retVal;
}

int getEnergyPoolWidth(int pool) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_ENERGY_POOL_WIDTH, pool);
	return retVal;
}

int getBuildingMaxArmor(int building) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_BUILDING_MAX_ARMOR, building);
	return retVal;
}

int getTimerValue(int timerNum) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_TIMER_VALUE, timerNum);
	return retVal;
}

int getLastAttacked(int &x, int &y) {
	int currentPlayer = getCurrentPlayer();
	x = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_LAST_ATTACKED_X, currentPlayer);
	y = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_LAST_ATTACKED_Y, currentPlayer);

	if (x || y) return 1;

	return 0;
}

int getPlayerTeam(int player) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_PLAYER_TEAM, player);
	return retVal;
}

int getBuildingTeam(int building) {
	assert((building >= 1) && (building <= 500));

	if (getBuildingOwner(building) == 0) return 0;

	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 2, D_GET_BUILDING_TEAM, building);
	return retVal;
}

int getFOW() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_FOW);
	return retVal;
}

int getAnimSpeed() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_ANIM_SPEED);
	return retVal;
}

int getBuildingStackPtr() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_BUILDING_STACK_PTR);
	return retVal;
}

int getTurnCounter() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_SCUMM_DATA], 1, D_GET_TURN_COUNTER);
	return retVal;
}

int getGroundAltitude(int x, int y) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_GROUND_ALTITUDE], 2, x, y);
	return retVal;
}

int checkForCordOverlap(int xStart, int yStart, int affectRadius, int simulateFlag) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_CHECK_FOR_CORD_OVERLAP], 4, xStart, yStart, affectRadius, simulateFlag);
	return retVal;
}

int checkForAngleOverlap(int unit, int angle) {
	assert(angle > -721);
	assert(angle < 721);

	if (!unit) return 0;

	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_CHECK_FOR_ANGLE_OVERLAP], 2, unit, angle);
	return retVal;
}

int checkForUnitOverlap(int x, int y, int radius, int ignoredUnit) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_CHECK_FOR_UNIT_OVERLAP], 4, x, y, radius, ignoredUnit);
	return retVal;
}

int checkForEnergySquare(int x, int y) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_CHECK_FOR_ENERGY_SQUARE], 2, x, y);
	return retVal;
}

int aiChat() {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_AI_CHAT], 0);
	return retVal;
}

int getPowerAngleFromPoint(int originX, int originY, int endX, int endY, int threshold, int olFlag) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_POWER_ANGLE_FROM_POINT], 6, originX, originY, endX, endY, threshold, olFlag);
	return retVal;
}

int getPowerAngleFromPoint(int originX, int originY, int endX, int endY, int threshold) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_POWER_ANGLE_FROM_POINT], 5, originX, originY, endX, endY, threshold);
	return retVal;
}

int checkIfWaterState(int x, int y) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_CHECK_IF_WATER_STATE], 2, x, y);
	return retVal;
}

int checkIfWaterSquare(int x, int y) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_CHECK_IF_WATER_SQUARE], 2, x, y);
	return retVal;
}

int getUnitsWithinRadius(int x, int y, int radius) {
	assert(x >= 0);
	assert(y >= 0);
	assert(radius >= 0);

	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_UNITS_WITHIN_RADIUS], 3, x, y, radius);
	return retVal;
}

int getLandingPoint(int x, int y, int power, int angle) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_LANDING_POINT], 4, x, y, power, angle);
	return retVal;
}

int getEnemyUnitsVisible(int playerNum) {
	int retVal = _vm->_moonbase->callScummFunction(MCP_params[F_GET_ENEMY_UNITS_VISIBLE], 1, playerNum);
	return retVal;
}

float degToRad(float degrees) {
	return degrees * M_PI / 180.;
}

void limitLocation(int &a, int &b, int c, int d) {
	if (a >= 0) {
		a = (a % c);
	} else {
		a = (c - (abs(a) % c));
	}

	if (b >= 0) {
		b = (b % d);
	} else {
		b = (d - (abs(b) % d));
	}
}

int energyPoolSize(int pool) {
	int width = getEnergyPoolWidth(pool);

	switch (width) {
	case 126:
		return 115;

	case 116:
		return 100;

	case 63:
		return 60;
	}

	return 0;
}

int getMaxCollectors(int pool) {
	int width = getEnergyPoolWidth(pool);

	switch (width) {
	case 126:
		return 4;

	case 116:
		return 3;

	case 63:
		return 2;
	}

	return 0;
}

int simulateBuildingLaunch(int x, int y, int power, int angle, int numSteps, int isEnergy) {
	static int sXSpeed = 0;
	static int sYSpeed = 0;
	static int sZSpeed = 0;
	static int sXLoc = 0;
	static int sYLoc = 0;
	static int sZLoc = 0;
	static int sFrictionCount = 0;
	static int sWhichRadius = 0;
	static int sWhichUnit = 0;

	int gWindXSpeed = getWindXSpeed();
	int gWindYSpeed = getWindYSpeed();
	int gTotalWindSpeed = getTotalWindSpeed();
	int gWindXSpeedMax = getWindXSpeedMax();
	int gWindYSpeedMax = getWindYSpeedMax();
	int bigXSize = getBigXSize();
	int bigYSize = getBigYSize();

	int groundAltitude = 0;
	int totalSpeed = 0;
	int resultingPoint = 0;
	int unscaledXLoc = 0;
	int unscaledYLoc = 0;
	int terrainType = 0;
	int passedBeyondUnit = 0;
	int currentDist = 0;


	if (!numSteps)
		numSteps = 1;

	if (!sXSpeed && !sYSpeed) {
		sZSpeed = (static_cast<int>(.70711 * power)) ;
		sXSpeed = (static_cast<int>(cos(degToRad(angle)) * sZSpeed)) ;
		sYSpeed = (static_cast<int>(sin(degToRad(angle)) * sZSpeed)) ;

		sZSpeed *= SCALE_Z;

		sZLoc = (getGroundAltitude(x, y) + HEIGHT_LOW + 10) * SCALE_Z;

		sXLoc = x * SCALE_X;
		sYLoc = y * SCALE_Y;

		sFrictionCount = 0;
		sWhichRadius = NODE_DETECT_RADIUS + 1;

		sWhichUnit = getClosestUnit(x + 10, y, 30, getCurrentPlayer(), 1, BUILDING_MAIN_BASE, 0, 0);
	}

	int savedUnscaledXLoc = 0;
	int savedUnscaledYLoc = 0;

	for (int i = 1; i <= numSteps; i++) {
		unscaledXLoc = sXLoc / SCALE_X;
		unscaledYLoc = sYLoc / SCALE_Y;

		groundAltitude = getGroundAltitude(unscaledXLoc, unscaledYLoc);
		groundAltitude *= SCALE_Z;

		sZLoc += sZSpeed / SCALE_Z;

		resultingPoint = MAX(1, unscaledXLoc + unscaledYLoc * getMaxX());

		if (sZLoc <= groundAltitude) {
			terrainType = getTerrain(unscaledXLoc, unscaledYLoc);

			sXSpeed = 0;
			sYSpeed = 0;
			sFrictionCount = 0;

			if (terrainType == TERRAIN_TYPE_GOOD)
				return resultingPoint;
			else
				return 0 - resultingPoint;
		} else {
			if (checkIfWaterState(unscaledXLoc, unscaledYLoc)) {
				sXSpeed = 0;
				sYSpeed = 0;
				sFrictionCount = 0;

				return 0 - resultingPoint;
			} else {
				int cfco = 0;
				int cfuo = 0;
				int cfes = 0;
				int cfao = 0;
				cfao = checkForAngleOverlap(sWhichUnit, angle);

				cfco = checkForCordOverlap(unscaledXLoc, unscaledYLoc, sWhichRadius, 1);
				cfuo = checkForUnitOverlap(unscaledXLoc, unscaledYLoc, sWhichRadius, sWhichUnit);

				if (!isEnergy)
					cfes = checkForEnergySquare(unscaledXLoc, unscaledYLoc);

				savedUnscaledXLoc = unscaledXLoc;
				savedUnscaledYLoc = unscaledYLoc;

				if (cfco || cfuo || cfes || cfao) {
					sXSpeed = 0;
					sYSpeed = 0;
					sFrictionCount = 0;

					return 0 - resultingPoint;
				} else {
					sFrictionCount++;

					if (sFrictionCount == 10) {
						sFrictionCount = 0;

						if (!gWindXSpeed)
							sXSpeed = sXSpeed * .95;

						if (!gWindYSpeed)
							sYSpeed = sYSpeed * .95;
					}

					if (passedBeyondUnit) {
						totalSpeed = getDistance(0, 0, sXSpeed, sYSpeed);

						if (totalSpeed > gTotalWindSpeed) {
							if (gWindXSpeed > 0) {
								if (sXSpeed < gWindXSpeedMax)
									sXSpeed += gWindXSpeed;
							} else {
								if (sXSpeed > gWindXSpeedMax)
									sXSpeed += gWindXSpeed;
							}

							if (gWindYSpeed > 0) {
								if (sYSpeed < gWindYSpeedMax)
									sYSpeed += gWindYSpeed;
							} else {
								if (sYSpeed > gWindYSpeedMax)
									sYSpeed += gWindYSpeed;
							}
						}
					} else {
						currentDist = getDistance(unscaledXLoc, unscaledYLoc, x, y);

						if (currentDist > BUILDING_HUB_RADIUS + NODE_DIAMETER)
							passedBeyondUnit = 1;
					}

					sXLoc += sXSpeed;
					sYLoc += sYSpeed;

					limitLocation(sXLoc, sYLoc, bigXSize, bigYSize);

					sZSpeed -= GRAVITY_CONSTANT;
				}
			}
		}
	}

	return 0;
}

int simulateWeaponLaunch(int x, int y, int power, int angle, int numSteps) {
	static int sXSpeed = 0;
	static int sYSpeed = 0;
	static int sZSpeed = 0;
	static int sXLoc = 0;
	static int sYLoc = 0;
	static int sZLoc = 0;
	static int sFrictionCount = 0;

	int gWindXSpeed = getWindXSpeed();
	int gWindYSpeed = getWindYSpeed();
	int gTotalWindSpeed = getTotalWindSpeed();
	int gWindXSpeedMax = getWindXSpeedMax();
	int gWindYSpeedMax = getWindYSpeedMax();
	int bigXSize = getBigXSize();
	int bigYSize = getBigYSize();

	int groundAltitude = 0;
	int totalSpeed = 0;
	int resultingPoint = 0;
	int unscaledXLoc = 0;
	int unscaledYLoc = 0;
	int terrainType = 0;
	int passedBeyondUnit = 0;
	int currentDist = 0;

	if (!numSteps) numSteps = 1;

	if (!sXSpeed && !sYSpeed) {
		sZSpeed = (static_cast<int>(.70711 * power)) ;
		sXSpeed = (static_cast<int>(cos(degToRad(angle)) * sZSpeed)) ;
		sYSpeed = (static_cast<int>(sin(degToRad(angle)) * sZSpeed)) ;

		sZSpeed *= SCALE_Z;

		sZLoc = (getGroundAltitude(x, y) + HEIGHT_LOW + 10) * SCALE_Z;

		sXLoc = x * SCALE_X;
		sYLoc = y * SCALE_Y;

		sFrictionCount = 0;
	}

	for (int i = 1; i <= numSteps; i++) {
		unscaledXLoc = sXLoc / SCALE_X;
		unscaledYLoc = sYLoc / SCALE_Y;

		groundAltitude = getGroundAltitude(unscaledXLoc, unscaledYLoc);
		groundAltitude *= SCALE_Z;
		sZLoc += sZSpeed / SCALE_Z;
		resultingPoint = MAX(1, unscaledXLoc + unscaledYLoc * getMaxX());

		if (sZLoc <= groundAltitude) {
			terrainType = getTerrain(unscaledXLoc, unscaledYLoc);

			sXSpeed = 0;
			sYSpeed = 0;
			sFrictionCount = 0;

			if (terrainType == TERRAIN_TYPE_GOOD)
				return resultingPoint;
			else
				return 0 - resultingPoint;
		} else {
			sFrictionCount++;

			if (sFrictionCount == 10) {
				sFrictionCount = 0;

				if (!gWindXSpeed)
					sXSpeed = sXSpeed * .95;

				if (!gWindYSpeed)
					sYSpeed = sYSpeed * .95;
			}

			if (passedBeyondUnit) {
				totalSpeed = getDistance(0, 0, sXSpeed, sYSpeed);

				if (totalSpeed > gTotalWindSpeed) {
					if (gWindXSpeed > 0) {
						if (sXSpeed < gWindXSpeedMax)
							sXSpeed += gWindXSpeed;
					} else {
						if (sXSpeed > gWindXSpeedMax)
							sXSpeed += gWindXSpeed;
					}

					if (gWindYSpeed > 0) {
						if (sYSpeed < gWindYSpeedMax)
							sYSpeed += gWindYSpeed;
					} else {
						if (sYSpeed > gWindYSpeedMax)
							sYSpeed += gWindYSpeed;
					}
				}
			} else {
				currentDist = getDistance(unscaledXLoc, unscaledYLoc, x, y);

				if (currentDist > BUILDING_HUB_RADIUS + NODE_DIAMETER)
					passedBeyondUnit = 1;
			}

			sXLoc += sXSpeed;
			sYLoc += sYSpeed;

			limitLocation(sXLoc, sYLoc, bigXSize, bigYSize);

			sZSpeed -= GRAVITY_CONSTANT;
		}
	}

	return 0;
}

int fakeSimulateWeaponLaunch(int x, int y, int power, int angle) {
	int distance = power * 480 / getMaxPower();
	float radAngle = degToRad(angle);
	int maxX = getMaxX();
	int maxY = getMaxY();

	x += distance * cos(radAngle);
	y += distance * sin(radAngle);

	x = (x + maxX) % maxX;
	y = (y + maxY) % maxY;

	return MAX(1, x + y * maxX);
}

int getEnergyHogType() {
	return energyHogType;
}

} // End of namespace Scumm