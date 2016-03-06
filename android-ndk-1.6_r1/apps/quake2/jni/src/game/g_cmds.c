/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "g_local.h"
#include "m_player.h"

// debut mod :qmenu
#include "qmenu.h"
// fin mod :qmenu

// debut mod : pet

#include "b_pet.h"

// fin mod : pet

// debut mod : scanner
#include "scanner.h"
// fin mod : scanner

// debut mod : hook
 void Cmd_Hook_f (edict_t *ent);
// fin mod : hook

// debut mod : sat
void Cmd_Satellite_Nuke(edict_t *ent);
// fin mod : sat


// debut mod : drone
void Cmd_LaserDrone_f(edict_t *ent);
// fin mod : drone


// debut : swv

void trimicons(int *icons)
{
  if (icons[6] == icons[5]) {
    icons[6] = -1;
    icons[4] = -1;
    icons[7] = -1;
    icons[3] = -1;
    icons[8] = -1;
    icons[2] = -1;
    icons[9] = -1;
    icons[1] = -1;
    icons[10] = -1;
    icons[0] = -1;
  }
  else if (icons[4] == icons[6]) {
    icons[4] = -1;
    icons[7] = -1;
    icons[3] = -1;
    icons[8] = -1;
    icons[2] = -1;
    icons[9] = -1;
    icons[1] = -1;
    icons[10] = -1;
    icons[0] = -1;
  }
  else if (icons[7] == icons[4]) {
    icons[7] = -1;
    icons[3] = -1;
    icons[8] = -1;
    icons[2] = -1;
    icons[9] = -1;
    icons[1] = -1;
    icons[10] = -1;
    icons[0] = -1;
  }
  else if (icons[3] == icons[7]) {
    icons[3] = -1;
    icons[8] = -1;
    icons[2] = -1;
    icons[9] = -1;
    icons[1] = -1;
    icons[10] = -1;
    icons[0] = -1;
  }
  else if (icons[8] == icons[3]) {
    icons[8] = -1;
    icons[2] = -1;
    icons[9] = -1;
    icons[1] = -1;
    icons[10] = -1;
    icons[0] = -1;
  }
  else if (icons[2] == icons[8]) {
    icons[2] = -1;
    icons[9] = -1;
    icons[1] = -1;
    icons[10] = -1;
    icons[0] = -1;
  }
  else if (icons[9] == icons[2]) {
    icons[9] = -1;
    icons[1] = -1;
    icons[10] = -1;
    icons[0] = -1;
  }
  else if (icons[1] == icons[9]) {
    icons[1] = -1;
    icons[10] = -1;
    icons[0] = -1;
  }
  else if (icons[10] == icons[1]) {
    icons[10] = -1;
    icons[0] = -1;
  }
  else if (icons[0] == icons[10]) {
    icons[0] = -1;
  }

}

void c14_SetFastIcons(edict_t *ent, int itemtype)
{
  gclient_t *cl;
  int i, index;
  gitem_t *it;
  int icons[11];
  int iconindex;

  cl = ent->client;

  icons[5] = cl->pers.selected_item;
  for (iconindex = 6; iconindex < 11; iconindex++) {
    icons[iconindex] = -1;
    for (i = 1;  i <= MAX_ITEMS; i++) {
      index = (icons[iconindex-1] + i) % MAX_ITEMS;
      if (!cl->pers.inventory[index])
        continue;
      it = &itemlist[index];
      if (!it->use)
        continue;
      if (!(it->flags & itemtype))
        continue;
      icons[iconindex] = index;
      break;
    }
  }

  for (iconindex = 4; iconindex >= 0; iconindex--) {
    icons[iconindex] = -1;
    for (i = 1;  i <= MAX_ITEMS; i++) {
      index = (icons[iconindex+1] + MAX_ITEMS -  i) % MAX_ITEMS;
      if (!cl->pers.inventory[index])
        continue;
      it = &itemlist[index];
      if (!it->use)
        continue;
      if (!(it->flags & itemtype))
        continue;
      icons[iconindex] = index;
      break;
    }
  }

  trimicons(icons);

  for (i = 0; i < 11; i++) {
    if (icons[i] == -1)
      ent->client->ps.stats[STAT_FAST_PREV5 + i] = 0;
    else
      ent->client->ps.stats[STAT_FAST_PREV5 + i] =
        gi.imageindex((&itemlist[icons[i]])->icon);
  }

  ent->client->fs_frame = level.framenum + 10;
  ent->client->ps.stats[STAT_FAST_NAME] = CS_ITEMS + icons[5];
  return;
}

void c14_SetFastWIcons(edict_t *ent)
{
  gclient_t *cl;
  int i, index;
  gitem_t *it;
  int icons[11];
  int iconindex;

  cl = ent->client;

  if (!cl->newweapon)
    return;

  icons[5] = ITEM_INDEX(cl->newweapon);
  for (iconindex = 6; iconindex < 11; iconindex++) {
    icons[iconindex] = -1;
    for (i = 1;  i <= MAX_ITEMS; i++) {
      index = (icons[iconindex-1] + i) % MAX_ITEMS;
      if (!cl->pers.inventory[index])
        continue;
      it = &itemlist[index];
      if (!it->use)
        continue;
      if (!(it->flags & IT_WEAPON))
        continue;
      icons[iconindex] = index;
      break;
    }
  }

  for (iconindex = 4; iconindex >= 0; iconindex--) {
    icons[iconindex] = -1;
    for (i = 1;  i <= MAX_ITEMS; i++) {
      index = (icons[iconindex+1] + MAX_ITEMS - i) % MAX_ITEMS;
      if (!cl->pers.inventory[index])
        continue;
      it = &itemlist[index];
      if (!it->use)
        continue;
      if (!(it->flags & IT_WEAPON))
        continue;
      icons[iconindex] = index;
      break;
    }
  }

  trimicons(icons);

  for (i = 0; i < 11; i++) {
    if (icons[i] == -1)
      ent->client->ps.stats[STAT_FAST_PREV5 + i] = 0;
    else
      ent->client->ps.stats[STAT_FAST_PREV5 + i] =
        gi.imageindex((&itemlist[icons[i]])->icon);
  }

  ent->client->fs_frame = level.framenum + 10;
  ent->client->ps.stats[STAT_FAST_NAME] = CS_ITEMS + icons[5];
  return;
}

// fin mod : swv

char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

// debut mod : pet

	ent1=GetOwner(ent1);
	ent2=GetOwner(ent2);
	if (!ent1) return !ent2;
	if (!ent2) return false;
	if (coop->value) return true;
	if (ent1 == ent2) return true; // this covers copets

// fin mod : pet

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;


// debut mod : qmenu
   // See if the menu is visible, and if it is, handle it accordingly.

   if (ent->client->showmenu)
   {
     Menu_Dn(ent);
     return;
   }
// fin mod : qmenu


	if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
// debut mod : swv
		c14_SetFastIcons(ent, itflags); 
// fin mod ! swv
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

// debut mod : qmenu
   // See if the menu is visible, and if it is, handle it accordingly.

   if (ent->client->showmenu)
   {
     Menu_Up(ent);
     return;
   }

// fin mod : qmenu



	if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
// debut mod : swv
		c14_SetFastIcons(ent, itflags); 
// fin mod : swv
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if ((deathmatch->value || coop->value) && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if ((deathmatch->value || coop->value) && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if ((deathmatch->value || coop->value) && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if ((deathmatch->value || coop->value) && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);

// debut mod : scanner
	if (cl->pers.scanner_active & 1)
		cl->pers.scanner_active = 2;
// fin mod : scanner
	
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

// debut mod : qmenu
// if we are in menu mode, call the menu handler.

   if (ent->client->showmenu)
   {
      Menu_Sel(ent);
      return;
   }
// fin mod : qmenu


	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
// debut mod : swv
//		if (cl->pers.weapon == it)
//			return;	// successful
		if (cl->newweapon == it) 
			{
			c14_SetFastWIcons(ent);
			return;  // successful
		    }


// fin mod : swv
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
// debut mod : swv
//		if (cl->pers.weapon == it)
//			return;	// successful
		if (cl->newweapon == it) 
			{
			c14_SetFastWIcons(ent);
			return;  // successful
		    }

// fin mod : swv
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
// debut mod : swv
	if (cl->newweapon == it) 
		{
		c14_SetFastWIcons(ent);
		return;  // successful
		}
// fin mod : swv


}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}
// debut mod : qmenu


void MyWave_Sel(edict_t *ent, int choice)
{

	ent->client->anim_priority = ANIM_WAVE;

	switch (choice)
	{
	case 0:
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
} // MyWave_Sel

void Cmd_MyWave_f(edict_t *ent)
{

   // Check to see if the menu is already open

   if (ent->client->showscores || ent->client->showinventory || 
        ent->client->showmenu || ent->client->showmsg)
        return;

   // send the layout
   
   Menu_Title(ent,"Pick a Wave");
   Menu_Add(ent,"Flip Off");
   Menu_Add(ent,"Salute");
   Menu_Add(ent,"Taunt");
   Menu_Add(ent,"Wave");

    // Setup the User Selection Handler

   ent->client->usr_menu_sel = MyWave_Sel;
   Menu_Open(ent);

} // Cmd_MyWave_f



// fin mod : qmenu

// debut mod : jetpack

/*
=================
Cmd_Thrust_f

MUCE:
To set jetpack on or off
=================
*/
void Cmd_Thrust_f (edict_t *ent)
{
	char    *string;

	string=gi.args();

	if (Q_stricmp ( string, "on") == 0)
	{
		ent->client->thrusting=1;
		ent->client->next_thrust_sound=0;
	}
	else
	{
 		ent->client->thrusting=0;
	}
}


// fin mod : jetpack

// debut mod : push

/*
=================
Cmd_Push_f
=================
*/
void Cmd_Push_f (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward;
	vec3_t	end;
	trace_t	tr;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
	if ( tr.ent && ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client)) )
	{
		VectorScale(forward, 5000, forward);
		VectorAdd(forward, tr.ent->velocity, tr.ent->velocity);
	}
}

/*
=================
Cmd_Pull_f
=================
*/
void Cmd_Pull_f (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward;
	vec3_t	end;
	trace_t	tr;

	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
	if ( tr.ent && ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client)) )
	{
		VectorScale(forward, -5000, forward);
		VectorAdd(forward, tr.ent->velocity, tr.ent->velocity);
	}
}

// fin mod : push

// debut mod : compass

  void Cmd_Compass_f (edict_t *ent) 
  { 
   gi.cprintf (ent, PRINT_HIGH, "%d %d %d \n",ent->client->ps.pmove.origin[1], ent->client->ps.pmove.origin[2], 
  ent->client->ps.pmove.origin[3]);  //should be one line 
  } 

  void Cmd_Speedometer_f (edict_t *ent) 
  { 
   gi.cprintf (ent, PRINT_HIGH, "%d %d %d \n",ent->client->ps.pmove.velocity[1], ent->client->ps.pmove.velocity[2], 
  ent->client->ps.pmove.velocity[3]); 
  } 


// fin mod : compass

// debut mod : cloak
/*
==================
Cmd_Cloak_f
==================
*/

void Cmd_Cloak_f (edict_t *ent)
{
	if (ent->client->cloakable ^= 1)
	{
		gi.centerprintf (ent, "Motion Cloaking Enabled!\n");
		ent->client->cloaktime = level.time + CLOAK_ACTIVATE_TIME;
		ent->client->cloaking = true;
		ent->client->cloakdrain = 0;
	}
	else
	{
		gi.centerprintf (ent, "Motion Cloaking Disabled!\n");
		ent->svflags &= ~SVF_NOCLIENT;
		ent->client->cloaking = false;
	}
}
// fin mod : cloak

/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_stricmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_stricmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_stricmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
// debut mod : chasecam
    else if (Q_stricmp (cmd, "chasecam") == 0)
        Cmd_Chasecam_Toggle (ent);
// fin mod : chasecam

// debut mod : hook
	else if (Q_stricmp (cmd, "hook") == 0)
      Cmd_Hook_f (ent);
// fin mod : hook
	
// debut mod : jetpack

	else if (Q_stricmp(cmd, "thrust") == 0 )
		Cmd_Thrust_f (ent);

// fin mod : jetpack

// debut mod : pet

	else if (Q_stricmp(cmd, "m_berserk") == 0)
		Cmd_Pet_f(ent, "monster_berserk");
	else if (Q_stricmp(cmd, "m_gladiator") == 0)
		Cmd_Pet_f(ent, "monster_gladiator");
	else if (Q_stricmp(cmd, "m_gunner") == 0)
		Cmd_Pet_f(ent, "monster_gunner");
	else if (Q_stricmp(cmd, "m_infantry") == 0)
		Cmd_Pet_f(ent, "monster_infantry");
	else if (Q_stricmp(cmd, "m_soldier") == 0)
		Cmd_Pet_f(ent, "monster_soldier");
	else if (Q_stricmp(cmd, "m_tank") == 0)
		Cmd_Pet_f(ent, "monster_tank");
	else if (Q_stricmp(cmd, "m_supertank") == 0)
		Cmd_Pet_f(ent, "monster_supertank");
	else if (Q_stricmp(cmd, "m_medic") == 0)
		Cmd_Pet_f(ent, "monster_medic");
	else if (Q_stricmp(cmd, "m_flipper") == 0)
		Cmd_Pet_f(ent, "monster_flipper");
	else if (Q_stricmp(cmd, "m_chick") == 0)
		Cmd_Pet_f(ent, "monster_chick");
	else if (Q_stricmp(cmd, "m_parasite") == 0)
		Cmd_Pet_f(ent, "monster_parasite");
	else if (Q_stricmp(cmd, "m_flyer") == 0)
		Cmd_Pet_f(ent, "monster_flyer");
	else if (Q_stricmp(cmd, "m_brain") == 0)
		Cmd_Pet_f(ent, "monster_brain");
	else if (Q_stricmp(cmd, "m_floater") == 0)
		Cmd_Pet_f(ent, "monster_floater");
	else if (Q_stricmp(cmd, "m_hover") == 0)
		Cmd_Pet_f(ent, "monster_hover");
	else if (Q_stricmp(cmd, "m_mutant") == 0)
		Cmd_Pet_f(ent, "monster_mutant");
	else if (Q_stricmp(cmd, "m_insane") == 0)
		Cmd_Pet_f(ent, "misc_insane");
	else if (Q_stricmp(cmd, "petcam") == 0)
		Cmd_Petcam_f(ent);
	else if (Q_stricmp(cmd, "petcamoff") == 0)
		Cmd_PetcamOff_f(ent);
	else if (Q_stricmp(cmd, "petstop") == 0)
		Cmd_PetStop_f(ent);
	else if (Q_stricmp(cmd, "petgo") == 0)
		Cmd_PetGo_f(ent);
	else if (Q_stricmp(cmd, "petfollow") == 0)
		Cmd_PetFollow_f(ent);
	else if (Q_stricmp(cmd, "petfree") == 0)
		Cmd_PetFree_f(ent);
	else if (Q_stricmp(cmd, "petlist") == 0)
		Cmd_PetList_f(ent);
	else if (Q_stricmp(cmd, "decoy") == 0)
		Cmd_Pet_f(ent, "monster_decoy");
	else if (Q_stricmp(cmd, "riot") == 0)
		Cmd_Pet_Riot_f(ent);

// fin mod : pet

// debut mod : file
	// FILEIO
	else if (Q_stricmp(cmd, "loadfile") == 0)
		LoadFile(ent);
	else if (Q_stricmp(cmd, "closefile") == 0)
		CloseFile(ent);
	else if (Q_stricmp(cmd, "showfile") == 0)
		ShowFile(ent);
// fin mod : file

// debut mod : qmenu
   else if (Q_stricmp (cmd, "menu") == 0)
     Menu_Hlp(ent);
// fin mod : qmenu

// debut mod : qmenu

else if (Q_stricmp (cmd, "mywave") == 0)
     Cmd_MyWave_f(ent);

// fin mod : qmenu

// debut mod : push

	else if (Q_stricmp (cmd, "push") == 0)
		Cmd_Push_f (ent);
	else if (Q_stricmp (cmd, "pull") == 0)
		Cmd_Pull_f (ent);

// fin mod : push

// debut mod : sat
	else if (Q_stricmp(cmd, "satellite") == 0)
		Cmd_Satellite_Nuke(ent);
// fin mod : sat

// debut mod : drone
	else if (Q_stricmp(cmd, "drone") == 0 ) 
		Cmd_LaserDrone_f(ent); 
// fin mod : drone

// debut mod : compass
   else if (Q_stricmp (cmd, "compass") == 0) 
		Cmd_Compass_f (ent); 
   else if (Q_stricmp (cmd, "speedometer") == 0) 
		Cmd_Speedometer_f (ent); 
// fin mod : compass

// debut mod : taunt
else if (Q_stricmp (cmd, "fov") == 0)
	{
		ent->client->ps.fov = atoi(gi.argv(1));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// taunt
	else if (Q_stricmp(cmd, "taunt") == 0)
		Taunt(ent);
// fin mod : taunt

// debut mod : cloak
	else if (Q_stricmp (cmd, "cloak") == 0)
		Cmd_Cloak_f (ent);
// fin mod : cloak

// debut mod : scanner
	else if (Q_stricmp (cmd, "scanner") == 0)
		Toggle_Scanner (ent);
// fin mod : scanner

	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}


