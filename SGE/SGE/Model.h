#ifndef MODEL_H
#define MODEL_H

#include <string>
#include "Engine.h"
#include "Vector.h"
#include "Angle.h"
#include <list>

struct ObjLexNode
{
	enum NodeType { None, Vertex, Normal, TextureUV, Face, Group, ObjectName } Type;
	union UData
	{
		struct Face_lex
		{
			struct Face_part_lex
			{
				unsigned int VertID;
				unsigned int TextureCordsID;
				unsigned int NormID;
			} Part1, Part2, Part3;
		} Face;
		struct Vertex_lex
		{
			double X, Y, Z;
		} Vertex;
		struct Normal_lex
		{
			double X, Y, Z;
		} Normal;
		struct TextureCord_lex
		{
			double U, V, W;
		} TextureCord;
	} Data;
};

class CVector;
typedef CVector vert_t;
typedef CVector norm_t;
struct text_t{ double u; double v; double w;};
struct face_t{ vert_t v1; vert_t v2; vert_t v3; };

class CModel
{
public:
	CModel();
	~CModel();
	void SetModel(const std::string& Model, bool SwapYZ = false);
	void Draw(CVector& Pos, CAngle& Ang);
	void DrawNormals(CVector& Pos, CAngle& Ang);
private:
	bool ParseAndLoad(std::list<ObjLexNode*>& lexed, bool SwapXY);
	void FreeResources();
	unsigned int m_VBOID; // Vertex
	unsigned int m_TBOID; // Texture
	unsigned int m_NBOID; // Normal

	// These might not be freed right away due to debugging
	vert_t* m_pVertexes;
	text_t* m_pTextureCords;
	norm_t* m_pNormals;

	unsigned int m_VertCount;

	IConfigorNode* m_DebugNormals;
};

#endif