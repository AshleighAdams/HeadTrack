#include "Engine.h"
#include "World.h"

CWorld::CWorld()
{
}

CWorld::~CWorld()
{
}

bool CWorld::AddEntity(CBaseEntity* Ent)
{
	if(!ValidEntity(Ent) && Ent)
	{
		m_Ents.push_back(Ent);
		return true;
	}

	return false; // Was not added
}

bool CWorld::RemoveEntity(CBaseEntity* Ent)
{
	if(ValidEntity(Ent) && Ent)
	{
		m_Ents.remove(Ent);
		delete Ent;
		return true;
	}

	return false;
}

bool CWorld::ValidEntity(CBaseEntity* Ent)
{
	for(auto it = m_Ents.begin(); it != m_Ents.end(); it++)
	{
		if(*it == Ent) // Yes, we found it!
			return true;
	}

	return false;
}

void CWorld::Think()
{
	for(auto it = m_Ents.begin(); it != m_Ents.end(); it++)
		(*it)->Think();
}

void CWorld::Draw()
{
	for(auto it = m_Ents.begin(); it != m_Ents.end(); it++)
		(*it)->Draw();
}

void CWorld::PostDraw()
{
	for(auto it = m_Ents.begin(); it != m_Ents.end(); it++)
		(*it)->PostDraw();
}

void CWorld::DrawDebug()
{
	
	for(auto it = m_Ents.begin(); it != m_Ents.end(); it++)
		(*it)->DrawDebug();
		
}

void CWorld::Simulate(double Time)
{
	for(auto it = m_Ents.begin(); it != m_Ents.end(); it++)
		(*it)->Simulate(Time);
}