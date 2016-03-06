//====================================================================
//
// SWINGING GRAPPLING HOOK v1.4 for Quake2
// by: Perecli Manole AKA Bort
//
//====================================================================
// Aside from this new file added to the project, the following are
// the lines of code added to id's original source files:
//--------------------------------------------------------------------
// File: g_cmds.c
// Location: on top after the #includes
//
// void Cmd_Hook_f (edict_t *ent);
//--------------------------------------------------------------------
// File: g_cmds.c
// Procedure: ClientCommand
// Location: after line "if (level.intermissiontime) return;"
//
// if (Q_stricmp (cmd, "hook") == 0)
//      Cmd_Hook_f (ent);
// else
//--------------------------------------------------------------------
// File: p_view.c
// Procedure: P_FallingDamage
// Location: after line "if (ent->waterlevel == 3) return;"
//
// if (!(ent->client->ps.pmove.pm_flags & PMF_ON_GROUND)) return;
//--------------------------------------------------------------------
// File: g_local.h
// Structure: gclient_s 
// Location: after line "weaponstate_t weaponstate;"
// 
// int hookstate_;
//--------------------------------------------------------------------
// File: q_shared.h
// Location: sound channel definitions
//
// #define CHAN_HOOK 5
//--------------------------------------------------------------------
// File: g_local.h
// Location: after line "extern cvar_t *maxclients;"
// 
// extern cvar_t *hook_speed;
// extern cvar_t *hook_min_len;
// extern cvar_t *hook_max_len;
// extern cvar_t *hook_rpf;
// extern cvar_t *hook_no_pred;
//--------------------------------------------------------------------
// File: g_main.c
// Location: after line "cvar_t	*sv_cheats;"
//
// cvar_t *hook_speed;	// speed of hook launch
// cvar_t *hook_min_len;// minimum chain length
// cvar_t *hook_max_len;// maximum chain length
// cvar_t *hook_rpf;	// rate(lengthen/shrink chain) per FRAMETIME
// cvar_t *hook_no_pred;// disable prediction while suspended in air by hook
//--------------------------------------------------------------------
// File: g_save.c
// Procedure: InitGame
// Location: "change anytime vars" section
// 
// hook_speed = gi.cvar ("hook_speed", "1000", 0);
// hook_min_len = gi.cvar ("hook_min_len", "40", 0);
// hook_max_len = gi.cvar ("hook_max_len", "1000", 0);
// hook_rpf = gi.cvar ("hook_rpf", "40", 0);
// hook_no_pred = gi.cvar ("hook_no_pred", "0", 0);
//--------------------------------------------------------------------


#include "g_local.h"

// edict->hookstate_ bit constants
#define HOOK_ON		0x00000001		// set if hook command is active
#define HOOK_IN		0x00000002		// set if hook has attached
#define SHRINK_ON	0x00000004		// set if shrink chain is active 
#define GROW_ON		0x00000008		// set if grow chain is active

// edict->sounds constants
#define MOTOR_OFF	0		// motor sound has not been triggered
#define MOTOR_START	1		// motor start sound has been triggered
#define MOTOR_ON	2		// motor running sound has been triggered

// this is the same as function P_ProjectSource in p_weapons.c except it projects
// the offset distance in reverse since hook is launched with player's free hand
void P_ProjectSource_Reverse (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == RIGHT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}


void DropHook (edict_t *ent)
{
	// enable prediction back in case it was left disabled
	ent->owner->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	
	// remove all hook flags
	ent->owner->client->hookstate_ = 0;

	gi.sound (ent->owner, CHAN_HOOK, gi.soundindex("hook/retract.wav"), 1, ATTN_IDLE, 0);

	// removes hook
	G_FreeEdict (ent);
}


void MaintainLinks (edict_t *ent)
{
	float multiplier;		// prediction multiplier
	vec3_t pred_hookpos;	// predicted future hook origin
	vec3_t norm_hookvel;	// normalized hook velocity

	vec3_t	offset, start;
	vec3_t	forward, right;

	vec3_t chainvec;		// vector of the chain 
	vec3_t norm_chainvec;	// vector of chain with distance of 1

	// predicts hook's future position since chain links fall behind
	multiplier = VectorLength(ent->velocity) / 22; 
	VectorNormalize2 (ent->velocity, norm_hookvel); 
	VectorMA (ent->s.origin, multiplier, norm_hookvel, pred_hookpos);

	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet (offset, 8, 8, ent->owner->viewheight-8);
	P_ProjectSource_Reverse (ent->owner->client, ent->owner->s.origin, offset, forward, right, start);

	// get info about chain
	_VectorSubtract (pred_hookpos,start,chainvec);
	VectorNormalize2 (chainvec, norm_chainvec);
	
	// shorten ends of chain
	VectorMA (start, 10, norm_chainvec, start);
	VectorMA (pred_hookpos, -20, norm_chainvec, pred_hookpos);

	// create temp entity chain
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (ent - g_edicts);
	gi.WritePosition (pred_hookpos);
	gi.WritePosition (start);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}


void HookBehavior(edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	vec3_t	chainvec;		// chain's vector
	float chainlen;			// length of extended chain
	vec3_t velpart;			// player's velocity component moving to or away from hook
	float force;			// restrainment force
	qboolean chain_moving;
		
	// decide when to disconnect hook
	if ( (!(ent->owner->client->hookstate_ & HOOK_ON)) ||// if hook has been retracted
	     (ent->enemy->solid == SOLID_NOT) ||			// if target is no longer solid (i.e. hook broke glass; exploded barrels, gibs) 
	     (ent->owner->deadflag) ||						// if player died
	     (ent->owner->s.event == EV_PLAYER_TELEPORT) )	// if player goes through teleport
	{
		DropHook(ent);
		return;
	}

	// gives hook same velocity as the entity it is stuck in
	VectorCopy (ent->enemy->velocity,ent->velocity);

// chain sizing 

	chain_moving = false;

	// grow the length of the chain
	if ((ent->owner->client->hookstate_ & GROW_ON) && (ent->angle < hook_max_len->value))
	{
		ent->angle += hook_rpf->value;
		if (ent->angle > hook_max_len->value) ent->angle = hook_max_len->value;
		chain_moving = true;
	}

	// shrink the length of the chain
    if ((ent->owner->client->hookstate_ & SHRINK_ON) && (ent->angle > hook_min_len->value))
	{
		ent->angle -= hook_rpf->value;
		if (ent->angle < hook_min_len->value) ent->angle = hook_min_len->value;
		chain_moving = true;
	}

	// determine sound play if climbing or sliding
	if (chain_moving)
	{
		// play start of climb sound
		if (ent->sounds == MOTOR_OFF)
		{
			gi.sound (ent->owner, CHAN_HOOK, gi.soundindex("hook/motor1.wav"), 1, ATTN_IDLE, 0);
			ent->sounds = MOTOR_START;
		}
		// play repetitive climb sound
		else if (ent->sounds == MOTOR_START)
		{
			gi.sound (ent->owner, CHAN_HOOK, gi.soundindex("hook/motor2.wav"), 1, ATTN_IDLE, 0);
			ent->sounds = MOTOR_ON;
		}
	}
	else if (ent->sounds != MOTOR_OFF)
	{
		gi.sound (ent->owner, CHAN_HOOK, gi.soundindex("hook/motor3.wav"), 1, ATTN_IDLE, 0);
		ent->sounds = MOTOR_OFF;
	}

// chain physics

	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->owner->viewheight-8);
	P_ProjectSource_Reverse (ent->owner->client, ent->owner->s.origin, offset, forward, right, start);

	// get info about chain
	_VectorSubtract (ent->s.origin, start, chainvec);
	chainlen = VectorLength (chainvec);

	// if player's location is beyond the chain's reach
	if (chainlen > ent->angle)	
	{	 
		// determine player's velocity component of chain vector
		VectorScale (chainvec, _DotProduct (ent->owner->velocity, chainvec) / _DotProduct (chainvec, chainvec), velpart);
		
		// restrainment default force 
		force = (chainlen - ent->angle) * 5;

		// if player's velocity heading is away from the hook
		if (_DotProduct (ent->owner->velocity, chainvec) < 0)
		{
			// if chain has streched for 25 units
			if (chainlen > ent->angle + 25)
				// remove player's velocity component moving away from hook
				_VectorSubtract(ent->owner->velocity, velpart, ent->owner->velocity);
		}
		else  // if player's velocity heading is towards the hook
		{
			if (VectorLength (velpart) < force)
				force -= VectorLength (velpart);
			else		
				force = 0;
		}
	}
	else
		force = 0;

	// disable prediction while suspended in air by hook
	// if server console variable hook_no_pred is set 
	if (!(ent->owner->client->ps.pmove.pm_flags & PMF_ON_GROUND))
	{
		if (hook_no_pred->value)
			ent->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	}	
	else
		ent->owner->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

    // applys chain restrainment 
	VectorNormalize (chainvec);
	VectorMA (ent->owner->velocity, force, chainvec, ent->owner->velocity);
	
	MaintainLinks (ent);

	// prep for next think
	ent->nextthink = level.time + FRAMETIME;
}


void HookTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	vec3_t	chainvec;		// chain's vector

	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->owner->viewheight-8);
	P_ProjectSource_Reverse (ent->owner->client, ent->owner->s.origin, offset, forward, right, start);

	// member angle is used to store the length of the chain
	_VectorSubtract(ent->s.origin,start,chainvec);
	ent->angle = VectorLength (chainvec);	

	// don't attach hook to sky
	if (surf && (surf->flags & SURF_SKY))
	{
		DropHook(ent);
		return;
	}

	// inflict damage on damageable items
	if (other->takedamage)
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 100, 0, MOD_HIT);

	if (other->solid == SOLID_BBOX)
	{
		if ((other->svflags & SVF_MONSTER) || (other->client))
			gi.sound (ent, CHAN_VOICE, gi.soundindex("hook/smack.wav"), 1, ATTN_IDLE, 0);

		DropHook(ent);
		return;
	}
	
	if (other->solid == SOLID_BSP)
	{
		// create puff of smoke
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SHOTGUN);
		gi.WritePosition (ent->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		gi.sound (ent, CHAN_VOICE, gi.soundindex("hook/hit.wav"), 1, ATTN_IDLE, 0);
		VectorClear (ent->avelocity);
	}
	else if (other->solid == SOLID_TRIGGER)
	{
		// debugging line; don't know if this will ever happen 
		gi.cprintf (ent->owner, PRINT_HIGH, "Hook touched a SOLID_TRIGGER\n");
	}
	
	// hook gets the same velocity as the item it attached to
	VectorCopy (other->velocity,ent->velocity);

	// flags hook as being attached to something
	ent->owner->client->hookstate_ |= HOOK_IN;

	ent->enemy = other;
	ent->touch = NULL;
	ent->think = HookBehavior;
	ent->nextthink = level.time + FRAMETIME;
}


void HookAirborne (edict_t *ent)
{
    vec3_t chainvec;		// chain's vector
	float chainlen;			// length of extended chain

	// get info about chain
	_VectorSubtract (ent->s.origin, ent->owner->s.origin, chainvec);
	chainlen = VectorLength (chainvec);
	
	if ( (!(ent->owner->client->hookstate_ & HOOK_ON)) || (chainlen > hook_max_len->value) )
	{
		DropHook(ent);
		return;
	}

	MaintainLinks (ent);	

	ent->nextthink = level.time + FRAMETIME;
}


void FireHook (edict_t *ent)
{
	edict_t *newhook;
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;

	// determine the damage the hook will inflict
	damage = 10;
	if (ent->client->quad_framenum > level.framenum)
		damage *= 4;
	
	// derive point of hook origin
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource_Reverse (ent->client, ent->s.origin, offset, forward, right, start);

	// spawn hook
	newhook = G_Spawn();
	VectorCopy (start, newhook->s.origin);
	VectorCopy (forward, newhook->movedir);
	vectoangles (forward, newhook->s.angles);
	VectorScale (forward, hook_speed->value, newhook->velocity);
	VectorSet(newhook->avelocity,0,0,-800);
	newhook->movetype = MOVETYPE_FLYMISSILE;
	newhook->clipmask = MASK_SHOT;
	newhook->solid = SOLID_BBOX;
	VectorClear (newhook->mins);
	VectorClear (newhook->maxs);
	newhook->s.modelindex = gi.modelindex ("models/items/hook/tris.md2");
	newhook->owner = ent;
	newhook->dmg = damage;

	// keeps track of motor chain sound played 
	newhook->sounds = 0;   
    
	// play hook launching sound
	gi.sound (ent, CHAN_HOOK, gi.soundindex ("medic/medatck2.wav"), 1, ATTN_IDLE, 0);
	
	// specify actions to follow 
	newhook->touch = HookTouch;
	newhook->think = HookAirborne;
	newhook->nextthink = level.time + FRAMETIME;
	
	gi.linkentity (newhook);
}


void Cmd_Hook_f (edict_t *ent)
{
	char *s;
	int *hookstate_;

	// get the first hook argument
	s = gi.argv(1);

	// create intermediate value
	hookstate_ = &ent->client->hookstate_;

	if ((ent->solid != SOLID_NOT) && (ent->deadflag == DEAD_NO) &&
		(!(*hookstate_ & HOOK_ON)) && (Q_stricmp(s, "action") == 0))
	{
		// flags hook as being active 
		*hookstate_ = HOOK_ON;   

		FireHook (ent);
		return;
	}

	if  (*hookstate_ & HOOK_ON)
	{
		// release hook	
		if (Q_stricmp(s, "action") == 0)
		{
			*hookstate_ = 0;
			return;
		}

		// fixme:
		// hop of chain and release hook when the following conditions apply
//		if ((ent->client->ps.pmove.pm_flags & PMF_JUMP_HELD) )		// jump is pressed
//			(*hookstate_ & HOOK_IN) &&								// hook is attached
//			(!(ent->client->ps.pmove.pm_flags & PMF_ON_GROUND)))	// player not on ground
//				(!(self.flags & FL_INWATER))	)	// player not in water
//		{
//			ent->velocity[2] += 200;
//			gi.sound (ent, CHAN_BODY, gi.soundindex ("medic/medatck2.wav"), 1, ATTN_IDLE, 0);
//			*hookstate_ = 0;
//			return;
//		}

		// deactivate chain growth or shrink
		if (Q_stricmp(s, "stop") == 0)
		{
			*hookstate_ -= *hookstate_ & (GROW_ON | SHRINK_ON);
			return;
		}

		// activate chain growth
		if (Q_stricmp(s, "grow") == 0)
		{
			*hookstate_ |= GROW_ON;
			*hookstate_ -= *hookstate_ & SHRINK_ON;
			return;
		}

		// activate chain shrinking
		if (Q_stricmp(s, "shrink") == 0)
		{
			*hookstate_ |= SHRINK_ON;		
			*hookstate_ -= *hookstate_ & GROW_ON;
		}
	}
}