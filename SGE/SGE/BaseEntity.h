#ifndef BASE_ENTITY_H
#define BASE_ENTITY_H

#include <string>

class CBaseEntity
{
public:
	virtual ~CBaseEntity(){}

	virtual const char* GetClass(){ return "CBaseEntity"; } // Discribes the entity and what it implements// return something like: "CMyEntity:" + CParentType::GetClass() + CParent2ndType
	bool Implements(const std::string& What) //  See if it implements X, such as: For a physical prop, CPropEntity:CPhysicsEntity:CModelEntity:CBaseEntity: (looking for CPhysicsEntity)
	{
		const std::string& self_class = GetClass();
		return self_class.find(What) != std::string::npos;
	}

	virtual void Simulate(double Time) = 0;
	virtual void Think() = 0;
	virtual void Draw() = 0;
	virtual void PostDraw() = 0;
	virtual void DrawDebug() = 0;
	// Server only
	virtual bool ShouldTransmit(CBaseEntity* Ent){ return false; }
	unsigned int EntIdx;
};

#endif