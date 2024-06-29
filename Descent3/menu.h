/*
* Descent 3 
* Copyright (C) 2024 Parallax Software
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
 */

#ifndef MENU_H
#define MENU_H

#include "pstypes.h"
#include "grdefs.h"

#define MAX_MENUS 5

#define MAX_MENU_ITEMS 50
#define MENU_STRING_LEN 70

#define MF_NORMAL_ANIMATED 1
#define MF_SELECT_ANIMATED 2

#define MAX_MENU_TYPES 5

enum menu_type {
  MENU_TYPE_BUTTON,
  MENU_TYPE_TEXT,
  MENU_TYPE_RADIOBUTTON,
  MENU_TYPE_CHECKBOX,
  MENU_TYPE_TEXTBOX
};

struct menu_item {
  menu_type mtype;
  char name[MENU_STRING_LEN];

  int normal_handle; // The anim/bitmap associated with this unhighlighted item
  int select_handle; // The anim/bitmap associated with the selected menu

  int flags; // see MF types above

  int x, y;

  ddgr_color normal_color, selected_color;

};

struct menu {
  int num_items;

  menu_item items[MAX_MENU_ITEMS];
  uint8_t used;
};

//	returns whether we decided to quit the game or not
//	this displays the main menu options and runs the menu system
int MainMenu();

//	opens options menu.
// ingame = true (if calling while playing the game)
void MenuOptions(bool ingame);

// DisplayLevelWarpDlg
//	pass in the max level allowed to be chosen, if -1, than all levels are allowed (a.k.a level warp cheat)
int DisplayLevelWarpDlg(int max_level);

// DisplayLevelSelectDlg
// displays a list of levels associated with the selected mission
int DisplayLevelSelectDlg(int max_level);

// Array of special missions
inline const char* SpecialMissions[] = {
  "d3.mn3",
  "merc.mn3"
};

// Array of level names for Descent 3: Retribution
inline const char* RetributionNames[] = {
  "Deimos - PTMC Data Retention Center",              // Level 1
  "Phobos - Novak Corporate Prison",                  // Level 2
  "Tiris - PTMC Piccu Station Facility",              // Level 3
  "Earth - PTMC Corporate Headquarters",              // Level 4
  "Mars - Red Acropolis Research Facility",           // Level 5
  "Mars - Noctis Labyrinthus Canyons",                // Level 6
  "Ceres - PTMC Secret Research Bunker C4",           // Level 7
  "Europa - PTMC Fuel Refinery Dol Ammad",            // Level 8
  "Mercury - PTMC Spacecraft Factory",                // Level 9
  "The Moon - CED Lunar Command Center",              // Level 10
  "Titan - PTMC Storage Facility",                    // Level 11
  "Charon - PTMC Proving Grounds",                    // Level 12 
  "Mars Orbit - CED Expediator Flagship Carrier",     // Level 13
  "Earth Orbit - CED Strategic Operations Platform",  // Level 14
  "Venus - Dravis' Stronghold",                       // Level 15
  "Origin Zero",                                       // Secret 1
  "H.V.W.R.C."                                        // Secret 2
};

// Array of level names for Descent 3: Mercenary
inline const char* MercenaryNames[] = {
  "PTMC Mars Colony",                         // Level 1
  "CED Special Operations - Central Europe",  // Level 2
  "CED Deep Space Tracking Station",          // Level 3
  "Alien Planetoid-Zeta Aquilae System",      // Level 4
  "Tiris - PTMC Secret Laboratory",           // Level 5
  "PTMC Cesium Mine - Io, Moon of Jupiter",   // Level 6
  "Earth - PTMC Corporate Headquarters"       // Level 7
};

#endif
