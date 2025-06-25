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

#ifndef M4_RIDDLE_ROOMS_SECTION4_ROOM407_H
#define M4_RIDDLE_ROOMS_SECTION4_ROOM407_H

#include "m4/riddle/rooms/room.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

class Room407 : public Room {
private:
	int _val1 = 0;
	int _val2 = 0;
	int _valveState2 = 0;
	int _valveState1 = 0;
	int _drawerState = 0;
	int _val6 = 0;
	int _val7 = 0;
	int _faucetPipeState = 0;
	int _airValveState = 0;
	int _leverKeyState = 0;
	int _rodState = 0;
	int _hoseState = 0;
	int _tubeState = 0;
	int _pumpState = 0;
	int _stopperState = 0;
	int _periodicTableState = 0;
	int _xyzzy7 = 0;
	int _items2State = 0;
	int _corkState = 0;
	int _xyzzy10 = 0;
	int _faucetHookedToJar = 0;
	int _frotz2 = 0;
	int _frotz3 = 0;
	int _frotz4 = 0;
	int _frotz5 = 0;
	int _frotz6 = 0;
	int _frotz7 = 0;
	int _frotz8 = 0;
	int _lookedAtLever = 0;
	int _tabletopState = 0;
	int _pivotState = 0;
	int _fullGlassState = 0;
	int _glassTopState = 0;
	int _glassBottomState = 0;
	int _roofPistonState = 0;
	int _glassGoneState = 0;
	int _comboLockState = 0;
	int _buttonFrame1 = 0;
	int _buttonFrame2 = 0;
	int _buttonFrame3 = 0;
	int _buttonFrame4 = 0;
	bool _codeCorrect = false;
	int _val11 = 0;
	int _val15 = 0;
	int _val16 = 0;
	int _waterFlowTube = 0;
	int _waterFlowFaucet = 0;
	int _waterFlowStem = 0;
	machine *_bottle = nullptr;
	machine *_chart = nullptr;
	machine *_cpist = nullptr;
	machine *_drawer = nullptr;
	machine *_escape = nullptr;
	machine *_faucetPipe = nullptr;
	machine *_airValve = nullptr;
	machine *_lever = nullptr;
	machine *_niche = nullptr;
	machine *_tpist = nullptr;
	machine *_pump = nullptr;
	machine *_bits = nullptr;
	machine *_star2 = nullptr;
	machine *_gears = nullptr;
	machine *_letter = nullptr;
	int _ripMedReach = 0;
	machine *_drawerPopup = nullptr;
	machine *_drawerPopupHose = nullptr;
	machine *_stopperInDrawer = nullptr;
	machine *_tubeInDrawer = nullptr;
	machine *_grips = nullptr;
	machine *_tabletopPopup = nullptr;
	machine *_tabletopPopupWithItems1 = nullptr;
	machine *_tabletopPopupWithItems2 = nullptr;
	machine *_tabletopPopupWithItems3 = nullptr;
	machine *_pivotPopup = nullptr;
	machine *_glassJarPopup = nullptr;
	machine *_glassTopPopupWithItems1 = nullptr;
	machine *_glassTopPopupWithItems2 = nullptr;
	machine *_glassBottomWithItems1 = nullptr;
	machine *_glassBottomWithItems2 = nullptr;
	machine *_roofPiston = nullptr;
	machine *_roofPistonWithItems = nullptr;
	machine *_glassGone = nullptr;
	machine *_comboLockPopup = nullptr;
	int _407pu08a = 0;
	int _407pu08b = 0;
	int _407pu08c = 0;
	int _407pu08d = 0;
	machine *_lockButton1 = nullptr;
	machine *_lockButton2 = nullptr;
	machine *_lockButton3 = nullptr;
	machine *_lockButton4 = nullptr;
	int _ripEnters = 0;
	int _stair = 0;
	machine *_ripley = nullptr;
	machine *_microscopeCloseup = nullptr;
	int _ripHiHand1 = 0;
	int _ripHiHand2 = 0;
	int _ripMedHand1 = 0;
	int _ripLowHand1 = 0;
	int _placeLeverKey = 0;
	machine *_safariShadow = nullptr;
	int _rollStuff = 0;
	int _407rp98 = 0;
	int _407rp99 = 0;
	int _pump407 = 0;
	int _hangRip = 0;
	int _rptmhr11 = 0;
	int _407tpis2 = 0;
	int _ripHeadTurn = 0;
	int _jarWaterFallingLeft = 0;
	int _jarWaterFallingCentre = 0;
	int _jarWaterFallingRight = 0;
	machine *_jarLeft = nullptr;
	machine *_jarCentre = nullptr;
	machine *_jarRight = nullptr;
	machine *_sink = nullptr;
	int _407a = 0;
	int _407b = 0;
	int _407c = 0;
	int _407d = 0;
	int _407e = 0;
	int _407h = 0;
	int _407j = 0;
	int _407k = 0;
	int _407l = 0;
	int _407m = 0;
	int _407o = 0;
	int _407p = 0;
	int _407q = 0;
	int _407r = 0;
	int _407s = 0;
	int _exit = 0;

	void setHotspots();
	void lookItem(const char *item, const char *digi);
	void useMicroscope();
	void roofPistonPopup();
	void glassBottomPopup();
	void glassTopPopup();
	void glassGonePopup();
	void pivotPopup();
	void tabletopPopup();
	bool lookGlassJar();
	void glassJarPopup();
	bool lookEmeraldCork();
	void periodicTablePopup();
	bool lookLeverKey();
	void gardenHoseSurgicalTube();
	void gardenHoseSurgicalTube2();
	void surgicalTubeFaucetPipe();
	void surgicalTubeStem();
	void reachHand(int frame);
	void reachLeverKey();
	void faucetPipeGardenHose();
	void faucetPipeGlassJar();
	void gardenHoseFaucetPipe();
	void surgicalTubeGardenHose();
	void emeraldIronSupport();
	void placeLeverKey();
	void rubberPlugGlassJar();
	void rubberPlugGlassJar2();
	void corkGlassJar1();
	void corkGlassJar2();
	void pumpGripsGlassJar();
	void periodicTableGlassJar();
	void surgicalTubeNozzles();
	void leverKeyTablePivot();
	void pumpRodBracket();
	void leverKey1();
	void leverKey2();
	void faucetHandleAirValve();
	void faucetPipeFaucetHandle1();
	void faucetPipeFaucetHandle2();
	void pumpRodPump();
	void pumpGripsPump();
	void useButton1();
	void useButton2();
	void useButton3();
	void useButton4();
	void checkCode();
	void useButtons();
	void useLeverKey();
	void useFaucet();
	void useValveHandle();
	bool usePump();
	void usePump2();
	void takeLetter();
	void takeFaucetPipe1();
	void takeFaucetPipe2();
	void takeFaucetPipe3();
	void takeSurgicalTube1();
	void takeSurgicalTube2();
	void takeSurgicalTube3();
	void takeGardenHose1();
	void takeGardenHose2();
	void takeGardenHose3();
	void takeGardenHose4();
	void takeGlassJar1();
	void takeGlassJar2();
	void takePeriodicTable1();
	void takePeriodicTable2();
	void takeJarGrips();
	void takeJarCork1();
	void takeJarCork2();
	void takeJarRubberPlug1();
	void takeJarRubberPlug2();
	void takeNozzlesTube();
	void takeLeverKeyFromWall();
	void takeLeverKeyFromBench();
	void takeLeverKey3();
	void takeAirValveHandle();
	void takeFaucetHandle();
	void takePumpRod1();
	void takePumpRod2();
	void takePumpRod3();
	void takePumpGrips1();
	void takePumpGrips2();
	void takeRubberPlug();
	void takeSurgicalTube();

public:
	Room407() : Room() {}
	~Room407() override {}

	void preload() override;
	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
	void syncGame(Common::Serializer &s) override;
};

} // namespace Rooms
} // namespace Riddle
} // namespace M4

#endif
