#ifndef WORLD_H
#define WORLD_H

#include <list>

#include "BaseEntity.h"

class CWorld
{
public:
	CWorld();
	~CWorld();
	bool ValidEntity(CBaseEntity* Ent);
	bool AddEntity(CBaseEntity* Ent);
	bool RemoveEntity(CBaseEntity* Ent); // TODO: Later, this can serialize and stuff
	void Think();
	void Draw();
	void PostDraw();
	void DrawDebug();
	void Simulate(double Time);
private:
	std::list<CBaseEntity*> m_Ents;
};

#endif