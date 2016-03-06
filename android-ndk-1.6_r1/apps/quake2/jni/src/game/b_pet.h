// b_pet.h
// blinky
// Created: 1999
// Edited: 2000/10/17


enum {
	PET_DEFAULT=0x00
	,PET_FREE=0x01 // free ranging flag
	, PET_STAY=0x02 // frozen flag
	, PET_FREETARGET=0x04 // choose own target
	, PET_FOLLOW=0x08 // follow master around instead of fighting
	};


void Pet_Killed(edict_t * targ, edict_t * inflictor);
qboolean Pet_FindTarget(edict_t * self);
void Pet_Cam(edict_t *ent);
void Pet_CamClass(edict_t *ent, char * szClass);
void Pet_CamOff(edict_t *ent);
void Pet_AdjustPetcam(struct gclient_s * client, float pyaw);
void Pet_Exchange(edict_t * ent);
void Pet_Riot(edict_t * ent);
int Pet_DoesMonsterMove(edict_t * ent);

void Blinky_Cam (edict_t * self);
void Blinky_NoCam (edict_t * self);

edict_t * GetOwner(edict_t * ent);

void Cmd_Pet_f (edict_t *ent, char * szPet);
// control free ranging flag
void Cmd_PetFollow_f (edict_t * ent);
void Cmd_PetFree_f (edict_t * ent);
// display pets
void Cmd_PetList_f (edict_t * ent);
// pet camming
void Cmd_Petcam_f (edict_t * ent);
void Cmd_PetcamOff_f (edict_t * ent);
// freezing pets
void Cmd_PetStop_f (edict_t * ent);
void Cmd_PetGo_f (edict_t * ent);
// clear current enemy
void Cmd_PetClear_f (edict_t * ent);
// release pets to wild
void Cmd_Pet_Riot_f (edict_t * ent);

