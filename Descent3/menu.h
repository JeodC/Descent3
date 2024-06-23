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

// Array of level names for Descent 3: Retribution
inline const char* RetributionNames[] = {
  "PTMC Data Retention Center",     // Level 1
  "Novak Corporate Prison",         // Level 2
  "Piccu Station & SRAD Research",  // Level 3
  "PTMC Corporate Headquarters",    // Level 4
  "Red Acropolis Research Station", // Level 5
  "Martian Nomad Caverns",          // Level 6
  "PTMC Research Bunker",           // Level 7
  "PTMC Dol Ammad Fuel Refinery",   // Level 8
  "PTMC Spacecraft Factory",        // Level 9
  "CED Lunar Command Base",         // Level 10
  "PTMC Storage Facility",          // Level 11
  "CED Expediator Dreadnaught",     // Level 12
  "CED Orbital Network Transmitter",// Level 13
  "Dravis' Stronghold"              // Level 14
};

// Array of level names for Descent 3: Mercenary
inline const char* MercenaryNames[] = {
  "PTMC Mars Colony",                      // Level 1
  "CED S.O.G Command Center",              // Level 2
  "CED Tacking Station",                   // Level 3
  "Alien Planetoid-Zeta Aquilae System",   // Level 4
  "PTMC Secret Laboratory",                // Level 5
  "PTMC Celium Mine",                      // Level 6
  "PTMC Corporate Headquarters (Mercenary)"// Level 7
};

#endif
