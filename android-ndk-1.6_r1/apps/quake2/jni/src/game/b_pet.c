// blinky
// Created 1999
// Edited: 2000/10/18

#include "g_local.h"
#include "b_pet.h"

// missing from g_local.h
void ED_CallSpawn (edict_t *ent);

// utility functions
qboolean EntIsMonster(edict_t *ent)
{
	return ent->svflags & SVF_MONSTER;
}
qboolean EntIsPet(edict_t *ent)
{
	return EntIsMonster(ent) && ent->monsterinfo.PetOwner;
}
qboolean EntIsClient(edict_t *ent)
{
	return !!ent->client;
}
edict_t * GetOwner(edict_t * ent)
{
	if (EntIsMonster(ent))
		return ent->monsterinfo.PetOwner;
	if (EntIsClient(ent))
		return ent;
	return 0;
}

void Pet_Create (edict_t *ent, char * szClass, const char * name)
{
	edict_t * pet;
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;

	if (ent->svflags & SVF_NOCLIENT)
		return;
	if (ent->client->PetStrength > petquota->value + 200 * deathmatch->value)
		return;
	if (ent->client->PetCount > petquota->value/50)
		return;


	pet = G_Spawn();
	pet->classname = szClass;
	pet->monsterinfo.PetOwner = ent;
	pet->monsterinfo.TargetCounter = 0;
	pet->monsterinfo.PetState = PET_DEFAULT;
	strcpy(pet->monsterinfo.name, name);

	VectorSet(offset, 40, 40, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	
	
	pet->spawnflags = 69;
	VectorCopy(start, pet->s.origin);
	VectorCopy(ent->s.angles, pet->s.angles);
	ED_CallSpawn(pet);


	// copied from KillBox
	{
		trace_t		tr;
		// unlink to make sure it can't possibly interfere with KillBox
		gi.unlinkentity (pet);
		tr = gi.trace (pet->s.origin, pet->mins, pet->maxs, pet->s.origin, NULL, MASK_PLAYERSOLID);
		if (tr.fraction < 1.0)
		{
			G_FreeEdict(pet);
			return;
		}
		gi.linkentity (pet);
	}

	if (!strcmp(szClass, "monster_decoy"))
	{
		//Use same model and skin as the person creating decoy
		pet->model = ent->model;
		pet->s.skinnum = ent->s.skinnum;
		pet->s.modelindex = ent->s.modelindex;
		pet->s.modelindex2 = ent->s.modelindex;
	}

	ent->client->PetCount++;
	ent->client->PetStrength += pet->max_health;
}

static void MasterKilled(edict_t * ent, edict_t * inflictor)
{
	edict_t * obj;
	if (!ent->client->PetCount)
		return;
	for ( obj = g_edicts; obj < &g_edicts[globals.num_edicts]; obj++)
	{
		// pets vanish
		if (EntIsMonster(obj) && (obj->monsterinfo.PetOwner == ent))
		{
//			if (random() < .6) {
			if (random() < 2) {
				G_FreeEdict(obj);
			} else {
				obj->monsterinfo.PetOwner = 0;
				obj->health = obj->health/2;
			}
		}
	}
}

static edict_t * 
FindEnemy(edict_t * self, int radius)
{
// loop through all nearby entities, looking for targets
	edict_t * ent = 0;
	int nPriority = 0; // 1 grenades, 2 clients, 3 pets
	int nPotentialPriority = 0;
	edict_t * enemy = 0;
	while ((ent = findradius(ent, self->s.origin, 500)) != NULL)

	{
		if ((ent->flags & FL_NOTARGET)||(ent->svflags & SVF_NOCLIENT))
			continue;
		if (!ent->inuse || !ent->takedamage || (ent->health <= 0))
			continue;
		if (EntIsMonster(ent))
		{
			if (OnSameTeam(ent, self))
				continue;
			nPotentialPriority = 3;
		}
		else if (EntIsClient(ent))
		{
			if (OnSameTeam(ent, self))
				continue;
			nPotentialPriority = 2;
		}
		else
		{
			// first priority is grenades
			// modify this if you don't want pets shooting grenades
			nPotentialPriority = 1;
		}

		if (nPriority>nPotentialPriority)
				continue;

		if (!visible(self, ent))
			continue;

		// remember this candidate enemy
		enemy = ent;
		nPriority = nPotentialPriority;
	}
	// return best we found (might be zero)
	return enemy;
}

static double DistanceTo(edict_t * self, edict_t * other)
{
	vec3_t dir;
	double distance;
	VectorSubtract(self->s.origin,other->s.origin, dir);
	distance = VectorLength(dir);
	return distance;
}

qboolean
Pet_FindTarget(edict_t * self)
{
	edict_t * enemy = 0;
	edict_t * PetOwner = self->monsterinfo.PetOwner;
	int radius = 500;
//	int bCammed = false;

	// follow mode means ignore combat
	if (self->monsterinfo.PetState & PET_FOLLOW)
	{
		self->enemy = 0;
		self->goalentity = PetOwner;
		return true;
	}

// optional bonus for a pet actively being cammed
//	if (PetOwner->client->petcam == self)
//	{
//		bCammed = true;
//		radius = 1000;
//	}

	// help master if appropriate
	if (!(self->monsterinfo.PetState & PET_FREETARGET)
		&& PetOwner->enemy)
	{
		vec3_t dir;
		double distance;
		VectorSubtract(self->s.origin,PetOwner->s.origin, dir);
		distance = VectorLength(dir);
		if ( distance < 400)
			enemy = PetOwner->enemy;
	}

	if (!enemy)
		enemy = FindEnemy(self, radius);

	if (enemy)
	{
		self->enemy = enemy;
		FoundTarget (self);

		if (!(self->monsterinfo.aiflags & AI_SOUND_TARGET) && (self->monsterinfo.sight))
			self->monsterinfo.sight (self, self->enemy);
		return true;
	}

	// no enemy, what about following owner ?
	self->enemy = 0; // 2000/05/27
	self->goalentity = 0;
	if (!(self->monsterinfo.PetState & PET_FREE)
			&& visible(self, PetOwner))
	{
		if (DistanceTo(self, PetOwner) > 100)
		{
			self->goalentity = PetOwner;
			self->monsterinfo.pausetime = level.time;
		}
		else
		{
			self->monsterinfo.stand (self);
			self->monsterinfo.pausetime = level.time + 30;
		}
	}
	return false;
}

void Pet_Cam(edict_t *ent)
{
	Pet_CamClass(ent, 0);
}

static void MonsterKilledOrReleased(edict_t * targ, edict_t * inflictor)
{
	// if pet, decrement master's count
	edict_t * owner = targ->monsterinfo.PetOwner;
// 2000/05 - nah, you have to wait until you die to make any more...
	if (0 && owner && owner->client)
	{
		owner->client->PetStrength -= targ->max_health;
		owner->client->PetCount--;
		if (inflictor && inflictor->client)
		{
			inflictor->max_health++;
		}
		if (owner->health > 1)
			owner->health--;
		targ->monsterinfo.PetOwner = 0;
	}
}

void Pet_Riot(edict_t *ent)
{
	edict_t * obj;
	if (ent->client->PetCount < 2)
	{
		gi.bprintf (PRINT_MEDIUM,"%s has insufficient monsters to start a riot.\n", ent->client->pers.netname);
		return;
	}
	gi.bprintf (PRINT_MEDIUM,"%s starts a riot.\n", ent->client->pers.netname);
	for (obj = g_edicts; obj<&g_edicts[globals.num_edicts]; obj++)
	{
		if ((obj->svflags & SVF_MONSTER) && obj->monsterinfo.PetOwner)
		{
			if ((ent == obj->monsterinfo.PetOwner)||(random()<.5))
			{
				MonsterKilledOrReleased(obj, 0);
				obj->monsterinfo.PetOwner = 0;
			}
		}
	}
}

int Pet_DoesMonsterMove(edict_t * ent)
{
	if (ent->monsterinfo.PetOwner
	 && (ent->monsterinfo.PetState & PET_STAY))
		return 0;
	return 1;
}

void Pet_CamClass(edict_t *ent, char * szClass)
{
	edict_t * obj1 = ent->client->petcam;
	edict_t * obj;
	if (!obj1)
		obj1 = ent;
	obj = obj1;
	while(1)
	{
		if (obj < &g_edicts[globals.num_edicts]-1)
			obj++;
		else
			obj = g_edicts;
		if (obj == obj1)
			break;
		if (szClass)
			if (!obj->classname || !strcmp(szClass, obj->classname))
				continue;
//		if ((obj->svflags & SVF_MONSTER) && (obj->monsterinfo.PetOwner == ent))
		if (obj->inuse && (obj->svflags & SVF_MONSTER) && (obj->monsterinfo.PetOwner == ent))
		{
			// found pet to cam
			ent->client->petcam = obj;
			break;
		}
		if (obj == ent)
		{
			// cycled to self
			ent->client->petcam = 0;
			break;
		}
	}

	// set cam facing pet's direction
	if (ent->client->petcam)
	{
		int i;
		obj = ent->client->petcam;
		for (i=0 ; i<3 ; i++)
		{
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(obj->s.angles[i] - ent->client->resp.cmd_angles[i]);
		}
		ent->client->LastCampetYaw = obj->s.angles[YAW];
	}

}

void Pet_CamOff(edict_t *ent)
{
	ent->client->petcam = 0;
}


void Pet_AdjustPetcam(struct gclient_s * client, float pyaw)
{
	if (!client->petcam)
		return;

	return;

	// if monster turned, turn client
	if (client->LastCampetYaw != client->petcam->s.angles[YAW])
		client->ps.pmove.delta_angles[YAW] = ANGLE2SHORT(client->petcam->s.angles[YAW] - client->LastCampetYaw);

	// if client turned, turn monster
	if (pyaw != client->resp.cmd_angles[YAW])
	{
		// haven't decided
	}
}


void Pet_Killed(edict_t * targ, edict_t * inflictor)
{
	if (EntIsMonster(targ))
		MonsterKilledOrReleased(targ, inflictor);
	else if (EntIsClient(targ))
		MasterKilled(targ, inflictor);
}


void Pet_Exchange(edict_t * ent)
{
	if (ent->client->petcam)
	{
		int i;
		for (i=0; i<3;i++)
		{
			vec3_t vec;
			VectorCopy(ent->s.origin, vec);
			VectorCopy(ent->client->petcam->s.origin, ent->s.origin);
			VectorCopy(vec, ent->client->petcam->s.origin);
		}
		return;
	}
}

static void CamOff(edict_t * self)
{
	self->client->chase_target = NULL;
	gi.unlinkentity(self);
	self->svflags &= ~SVF_NOCLIENT;
	self->solid = SOLID_BBOX;
	gi.linkentity(self);
}

static void CamOn(edict_t * self, edict_t * targ)
{
	self->client->chase_target = targ;
	gi.unlinkentity(self);
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_NOT;
	gi.linkentity(self);
}

void Blinky_Cam (edict_t * self)
{
	edict_t *e;

	e = self->client->chase_target;
	if (!e) e = g_edicts;
	e++;
	for ( ; e < g_edicts+(int)(maxclients->value); e++)
	{
		if (e->inuse && (e->solid != SOLID_NOT) && (e != self) && !(e->client->chase_target))
		{
			if (OnSameTeam(self, e))
			{
				CamOn(self, e);
				self->client->update_chase = true;
				return;
			}
		}
	}


	CamOff(self);
}

void Blinky_NoCam (edict_t * self)
{
	edict_t *e;

	CamOff(self);

	e = self->client->chase_target;
	if (!e) e = g_edicts;
	for (e = g_edicts; e < g_edicts+(int)(maxclients->value); e++)
	{
		if (e->inuse && e->client && (e->client->chase_target == self))
			CamOff(e);
	}
}

static void ApplyPetStateMask(edict_t * ent, int AndMask, int OrMask)
// change pet states
{
	edict_t * obj;
	const char * name = gi.args();

	if (!ent->client->PetCount)
		return;
	
	for (obj = g_edicts; obj<&g_edicts[globals.num_edicts]; obj++)
	{
		if ((obj->svflags & SVF_MONSTER)
			&& (ent == obj->monsterinfo.PetOwner))
		{ // it is a pet - is it one they named ?
			if (!name || !name[0] || 0 == strcmp(name, obj->monsterinfo.name))
			{
				obj->monsterinfo.PetState &= AndMask;
				obj->monsterinfo.PetState |= OrMask;
			}
		}
	}
}
static void SetPetStateFlag(edict_t * ent, int flag)
{
	ApplyPetStateMask(ent, 0, flag);
}
static void ClearPetStateFlag(edict_t * ent, int flag)
{
	ApplyPetStateMask(ent, ~flag, 0);
}


/*********************************
  Player command entry points
  ***********************************/
void Cmd_Pet_f (edict_t *ent, char * szPet)
{
	const char * name = gi.args();
	Pet_Create(ent, szPet, name);
}
void Cmd_Petcam_f (edict_t * ent)
{
	Pet_Cam(ent);
}
void Cmd_PetcamOff_f (edict_t * ent)
{
	Pet_CamOff(ent);
}
void Cmd_PetStop_f (edict_t * ent)
{
	SetPetStateFlag(ent, PET_STAY);
}

static void ClearPetEnemies(edict_t * ent)
{
	edict_t * obj;
	const char * name = gi.args();

	if (!ent->client->PetCount)
		return;
	
	for (obj = g_edicts; obj<&g_edicts[globals.num_edicts]; obj++)
	{
		if ((obj->svflags & SVF_MONSTER)
			&& (ent == obj->monsterinfo.PetOwner)
			&& (!name || 0 == strcmp(name, obj->monsterinfo.name)))
		{
			obj->enemy = 0;
			obj->oldenemy = 0;
			obj->movetarget = 0;
			obj->goalentity = 0;
		}
	}
}

void Cmd_PetClear_f (edict_t * ent)
{
	ClearPetEnemies(ent);
}
void Cmd_PetGo_f (edict_t * ent)
{
	ClearPetStateFlag(ent, PET_STAY);
}
void Cmd_PetFree_f (edict_t * ent)
{
	SetPetStateFlag(ent, PET_FREE);
	ClearPetEnemies(ent);
}
void Cmd_PetFollow_f (edict_t * ent)
{
	ClearPetStateFlag(ent, PET_FREE);
}

void Cmd_PetList_f (edict_t * ent)
{
	edict_t * obj;
	char text[2048];
	char buffer[128];
	char * name;

	strcpy(text, "");
	if (!ent->client->PetCount)
	{
		strcat(text, "No pets.");
	}
	else
	{
		for (obj = g_edicts; obj<&g_edicts[globals.num_edicts]; obj++)
		{
			if ((obj->svflags & SVF_MONSTER)
				&& (ent == obj->monsterinfo.PetOwner))
			{
				if (obj->monsterinfo.name)
					name = obj->monsterinfo.name;
				else
					name = "<unnamed>";
				sprintf(buffer, "%12s:  %12s\n", name, obj->classname);
				strcat(text, buffer);
			}
		}
	}
	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, text);
	else
		gi.cprintf(ent, PRINT_CHAT, text);

}

void Cmd_Pet_Riot_f (edict_t *ent)
{
	Pet_Riot(ent);
}
