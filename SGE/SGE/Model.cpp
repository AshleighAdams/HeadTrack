#include "Model.h"
#include "Vector.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <vector>

using namespace std;

void LexObj(const char* pContents, std::list<ObjLexNode*>& out)
{
	unsigned int len = strlen(pContents);
	unsigned int i;

	function<void(void)> ReadToNewLine = [&]()
	{
		while(i < len)
		{
			char x = pContents[i];
			if(x == '\n')
				return;
			i++;
		}
	};

	function<void(void)> ReadToNoneSpace = [&]()
	{
		while(i < len)
		{
			char x = pContents[i];
			if(x != ' ' && x != '\t')
				return;
			i++;
		}
	};

	function<bool(const char*)> Peak = [&](const char* pStr) -> bool
	{
		size_t len2 = strlen(pStr);
		for(size_t n = 0; n < len2; n++)
			if(n + i >= len || pStr[n] != pContents[i+n])
				return false;
		return true;
	};

	function<bool(void)> PeakIsNumber = [&]() -> bool
	{
		char pc = pContents[i];
		return pc == '-' || (pc >= 48 && pc <= 57);
	};

	function<bool(double&)> ReadDouble = [&](double& out) -> bool
	{
		stringstream ss;
		bool wrotesome = false;

		while(i < len)
		{
			char x = pContents[i];

			if( (x >= 48 && x <= 57) || x == '.' || x == '-' )
			{
				ss << x;
				wrotesome = true;
			}else break;

			i++;
		}

		out = 0.0;
		if(wrotesome)
			ss >> out;
		
		return wrotesome;
	};

	function<bool(int&)> ReadInt = [&](int& out) -> bool
	{
		stringstream ss;
		bool wrotesome = false;

		while(i < len)
		{
			char x = pContents[i];

			if( x >= 48 && x <= 57 )
			{
				ss << x;
				wrotesome = true;
			}else break;

			i++;
		}

		if(wrotesome)
			ss >> out;
		
		return wrotesome;
	};

	function<bool(void)> HandelVert = [&]() -> bool
	{
		double x,y,z;
		i++;
		ReadToNoneSpace();
		if(!ReadDouble(x))
			return false;
		ReadToNoneSpace();
		if(!ReadDouble(y))
			return false;
		ReadToNoneSpace();
		if(!ReadDouble(z))
			return false;

		ObjLexNode* node = new ObjLexNode;
		node->Type = node->Vertex;
		node->Data.Vertex.X = x;
		node->Data.Vertex.Y = y;
		node->Data.Vertex.Z = z;
		out.push_back(node);

		return true;
	};

	function<bool(void)> HandelNorm = [&]() -> bool
	{
		double x,y,z;
		i+=2;
		ReadToNoneSpace();
		if(!ReadDouble(x))
			return false;
		ReadToNoneSpace();
		if(!ReadDouble(y))
			return false;
		ReadToNoneSpace();
		if(!ReadDouble(z))
			return false;

		ObjLexNode* node = new ObjLexNode;
		node->Type = node->Normal;
		node->Data.Normal.X = x;
		node->Data.Normal.Y = y;
		node->Data.Normal.Z = z;
		out.push_back(node);

		return true;
	};

	function<bool(void)> HandelTextCord = [&]() -> bool
	{
		double u,v,w;
		i += 2;
		ReadToNoneSpace();
		if(!ReadDouble(u))
			return false;
		ReadToNoneSpace();
		ReadDouble(v);
		ReadToNoneSpace();
		ReadDouble(w);

		ObjLexNode* node = new ObjLexNode;
		node->Type = node->TextureUV;
		node->Data.TextureCord.U = u;
		node->Data.TextureCord.V = v;
		node->Data.TextureCord.W = w;
		out.push_back(node);

		return true;
	};

	function<bool(void)> HandelFace = [&]() -> bool
	{
		i++;
		int v1_lvl = 1, v2_lvl = 1, v3_lvl = 1, v4_lvl = 1;
		int v1 = 0, v1tc = 0, v1n = 0, // Vertex 1, Vert1 text cord, vert 1 normal
			v2 = 0, v2tc = 0, v2n = 0,
			v3 = 0, v3tc = 0, v3n = 0,
			v4 = 0, v4tc = 0, v4n = 0;
		ReadToNoneSpace();

		// Vert 1
		if(!ReadInt(v1))
			return false;
		if(Peak("/"))
		{
			i++;
			ReadInt(v1tc);
			if(Peak("/"))
			{
				i++;
				ReadInt(v1n);
			}
		}
		ReadToNoneSpace();

		//Vert 2
		if(!ReadInt(v2))
			return false;
		if(Peak("/"))
		{
			i++;
			ReadInt(v2tc);
			if(Peak("/"))
			{
				i++;
				ReadInt(v2n);
			}
		}
		ReadToNoneSpace();

		//Vert 3
		if(!ReadInt(v3))
			return false;
		if(Peak("/"))
		{
			i++;
			ReadInt(v3tc);
			if(Peak("/"))
			{
				i++;
				ReadInt(v3n);
			}
		}
		ReadToNoneSpace();
		
		ObjLexNode* node = new ObjLexNode;

		node->Type = node->Face;
		node->Data.Face.Part1.VertID = v1;
		node->Data.Face.Part1.TextureCordsID = v1tc;
		node->Data.Face.Part1.NormID = v1n;

		node->Data.Face.Part2.VertID = v2;
		node->Data.Face.Part2.TextureCordsID = v2tc;
		node->Data.Face.Part2.NormID = v2n;

		node->Data.Face.Part3.VertID = v3;
		node->Data.Face.Part3.TextureCordsID = v3tc;
		node->Data.Face.Part3.NormID = v3n;

		out.push_back(node);

		// Vert 4?
		if(PeakIsNumber()) // Is a number present
		{
			// Read vert4
			if(!ReadInt(v4))
				return false;
			if(Peak("/"))
			{
				i++;
				ReadInt(v4tc);
				if(Peak("/"))
					ReadInt(v4n);
			}
			

			ObjLexNode* node = new ObjLexNode; // Lets convert the quad to a triangle

			node->Type = node->Face;
			node->Data.Face.Part1.VertID = v3;
			node->Data.Face.Part1.TextureCordsID = v3tc;
			node->Data.Face.Part1.NormID = v3n;

			node->Data.Face.Part2.VertID = v4;
			node->Data.Face.Part2.TextureCordsID = v4tc;
			node->Data.Face.Part2.NormID = v4n;

			node->Data.Face.Part3.VertID = v1;
			node->Data.Face.Part3.TextureCordsID = v1tc;
			node->Data.Face.Part3.NormID = v1n;

			out.push_back(node);
		}

		return true;
	};

	for(i = 0; i < len; i++)
	{
		char x = pContents[i];

		if(x == '#')
		{
			ReadToNewLine();
			continue;
		}

		ReadToNoneSpace();

		if(Peak("v "))
			HandelVert();
		else if(Peak("f "))
			HandelFace();
		else if(Peak("vn "))
			HandelNorm();
		else if(Peak("vt "))
			HandelTextCord();
		else
			ReadToNewLine();
	}
}

bool CModel::ParseAndLoad(list<ObjLexNode*>& lexed, bool SwapXY)
{
	// First pass, count the number of verts, norms textures and shit so we know how much memory we need to allocate
	unsigned int verts = 0, norms = 0, texts = 0, faces = 0;
	for(auto it = lexed.begin(); it != lexed.end(); it++)
	{
		ObjLexNode* node = *it;
		switch(node->Type)
		{
		case node->Face:
			faces++;
			break;
		case node->Vertex:
			verts++;
			break;
		case node->Normal:
			norms++;
			break;
		case node->TextureUV:
			texts++;
			break;
		}
	}

	unsigned int VertPos = 0, TextsPos = 0, NormsPos = 0, FacePos = 0;
	vert_t* pVerts = new vert_t[verts];
	text_t* pTexts = new text_t[texts];
	norm_t* pNorms = new norm_t[norms];

	for(unsigned int i = 0; i < verts; i++)
		pVerts[i].X = pVerts[i].Y = pVerts[i].Z = 0.0;
	for(unsigned int i = 0; i < texts; i++)
		pTexts[i].u = pTexts[i].v = pTexts[i].w = 0.0;
	for(unsigned int i = 0; i < norms; i++)
		pNorms[i].X = pNorms[i].Y = pNorms[i].Z = 0.0;

	// This will be what is actually sent to the card
	m_pVertexes = new vert_t[faces * 3];
	m_pTextureCords = new text_t[faces * 3];
	m_pNormals = new norm_t[faces * 3];
	
	for(auto it = lexed.begin(); it != lexed.end(); it++)
	{
		ObjLexNode* node = *it;
		unsigned int  debug1 = node->Data.Face.Part1.VertID - 1;
		unsigned int  debug2 = node->Data.Face.Part1.NormID - 1;
		unsigned int  debug3 = node->Data.Face.Part1.TextureCordsID - 1;
		switch(node->Type)
		{
		case node->Face:
			m_pVertexes[FacePos] = pVerts[node->Data.Face.Part1.VertID - 1];
			m_pTextureCords[FacePos] = pTexts[node->Data.Face.Part1.TextureCordsID - 1];
			m_pNormals[FacePos] = pNorms[node->Data.Face.Part1.NormID - 1];
			FacePos++;
			m_pVertexes[FacePos] = pVerts[node->Data.Face.Part2.VertID - 1];
			m_pTextureCords[FacePos] = pTexts[node->Data.Face.Part2.TextureCordsID - 1];
			m_pNormals[FacePos] = pNorms[node->Data.Face.Part2.NormID - 1];
			FacePos++;
			m_pVertexes[FacePos] = pVerts[node->Data.Face.Part3.VertID - 1];
			m_pTextureCords[FacePos] = pTexts[node->Data.Face.Part3.TextureCordsID - 1];
			m_pNormals[FacePos] = pNorms[node->Data.Face.Part3.NormID - 1];
			FacePos++;
			break;
		case node->Vertex:
			pVerts[VertPos].X = node->Data.Vertex.X;
			if(!SwapXY)
			{
				pVerts[VertPos].Y = node->Data.Vertex.Y;
				pVerts[VertPos].Z = node->Data.Vertex.Z;
			}
			else
			{
				pVerts[VertPos].Z = node->Data.Vertex.Y;
				pVerts[VertPos].Y = node->Data.Vertex.Z;
			}
			VertPos++;
			break;
		case node->Normal:
			pNorms[NormsPos].X = node->Data.Normal.X;
			if(!SwapXY)
			{
				pNorms[NormsPos].Y = node->Data.Normal.Y;
				pNorms[NormsPos].Z = node->Data.Normal.Z;
			}
			else
			{
				pNorms[NormsPos].Y = node->Data.Normal.Z;
				pNorms[NormsPos].Z = node->Data.Normal.Y;
			}
			NormsPos++;
			break;
		case node->TextureUV:
			pTexts[TextsPos].u = node->Data.TextureCord.U;
			pTexts[TextsPos].v = node->Data.TextureCord.V;
			pTexts[TextsPos].w = node->Data.TextureCord.W;
			TextsPos++;
			break;
		}
	}

	delete [] pVerts; // Obj is loaded now, we don't need these anymore
	delete [] pTexts;
	delete [] pNorms;

	// No normals were present in the model
	if(!norms)
	{
		for(unsigned int i = 0; i < faces * 3; i += 3)
		{
			face_t* face = (face_t*)&m_pVertexes[i];
			CVector& a = face->v1;
			CVector& b = face->v2;
			CVector& c = face->v3;;
			CVector norm = -(b - a).Cross(c - a); // My normals were backwards...
			//cout << i << ": " << norm << '\n';
			m_pNormals[i + 0] += norm;
			m_pNormals[i + 1] += norm;
			m_pNormals[i + 2] += norm;
		}

		for(unsigned int i = 0; i < faces * 3; i++)
			m_pNormals[i].Normalize();
		
		// Now, lets smooth the normals, as long as we don't have too many verts

		if(faces < 20000)
		{
			bool* pDoneVert = new bool[faces * 3];

			for(unsigned int i = 0; i < faces * 3; i++)
				pDoneVert[i] = false;

			for(unsigned int i = 0; i < faces * 3; i++)
			{
				if(pDoneVert[i])
					continue;
				pDoneVert[i] = true;
			
				vector<CVector*> group;

				group.push_back(&m_pNormals[i]);
				CVector& apos = m_pVertexes[i];
				CVector& anrm = m_pNormals[i];

				for(unsigned int n = i; n < faces * 3; n++)
				{
					if(pDoneVert[n])
						continue;
				
					CVector& bpos = m_pVertexes[n];
					CVector& bnrm = m_pNormals[n];
				
					// TODO: Angle!

					double ang = acos(bnrm.Dot(anrm));
					double elurang = ang / 3.14159265358979323 * 180.0;

					if(apos == bpos && elurang < 45.0) //(apos-bpos).Length() < 0.00001 )
					{
						group.push_back(&m_pNormals[n]);
						pDoneVert[n] = true;
					}
				}

				if(group.size() == 1)
					continue;

				//cout << "Found a group of " << group.size() << " normals\n";

				CVector avg = CVector(0, 0, 0);

				for(size_t i = 0; i < group.size(); i++)
					avg += *group[i];
			
				avg.Div(group.size());
				avg.Normalize();

				for(size_t i = 0; i < group.size(); i++)
					*group[i] = avg;
			}
			delete [] pDoneVert;
		}
		else
			std::cout << "Warning: the model has too many faces, not calculating smooth normals!\n";
	}
	
	// Bind verts
	glGenBuffers(1, &m_VBOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert_t) * faces * 3, m_pVertexes, GL_STATIC_DRAW);

	// Bind texturecords
	glGenBuffers(1, &m_TBOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_TBOID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vert_t) * faces * 3, m_pTextureCords, GL_STATIC_DRAW);

	// Bind normals
	glGenBuffers(1, &m_NBOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_NBOID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(norm_t) * faces * 3, m_pNormals, GL_STATIC_DRAW);

	if(!pEngineInstance->GetOptions()->GetSetting("debug.normals").GetValue(false))
	{
		delete [] m_pVertexes;
		delete [] m_pTextureCords;
		delete [] m_pNormals;
		m_pVertexes = 0;
		m_pTextureCords = 0;
		m_pNormals = 0;
	}

	m_VertCount = faces * 3;

	return true;
}

CModel::CModel()
{
	m_pNormals = 0;
	m_pTextureCords = 0;
	m_pVertexes = 0;
	m_NBOID = 0;
	m_VBOID = 0;
	m_TBOID = 0;
	m_DebugNormals = &pEngineInstance->GetOptions()->GetSetting("debug.normals");
}

CModel::~CModel()
{
	FreeResources();
}

bool StringBeginsWith(const string& str, const string& what)
{
	return str.substr(0, what.size()) == what;
}

void CModel::SetModel(const std::string& Model, bool SwapYZ)
{
	FreeResources();
	
	string file = pEngineInstance->GetFileSystem()->GetModel(Model);

	ifstream ifs(file);
	if(!ifs.is_open())
		return;

	list<ObjLexNode*> lexed;

	char* inf;
	ifs.seekg(0, ios::end);
	size_t length = ifs.tellg();
	ifs.seekg(0, ios::beg);

	inf = new char[length];
	ifs.read(inf, length);
	
	LexObj(inf, lexed);
	delete [] inf;

	ParseAndLoad(lexed, SwapYZ);

	// Ok, time to delete the tokens
	for(auto it = lexed.begin(); it != lexed.end(); it++)
		delete *it;

	ifs.close();
}

void CModel::FreeResources()
{
	if(m_VBOID)
		glDeleteBuffers(1, &m_VBOID);
	if(m_NBOID)
		glDeleteBuffers(1, &m_NBOID);
	if(m_TBOID)
		glDeleteBuffers(1, &m_TBOID);
	
	if(m_pNormals)
		delete [] m_pNormals;
	if(m_pTextureCords)
		delete [] m_pTextureCords;
	if(m_pVertexes)
		delete [] m_pVertexes;

	m_pVertexes = 0;
	m_pTextureCords = 0;
	m_pNormals = 0;
	m_TBOID = 0;
	m_NBOID = 0;
	m_VBOID = 0;
}

// Very, very slow.
void CModel::DrawNormals(CVector& Pos, CAngle& Ang)
{
	if(!m_DebugNormals->GetValue(false))
		return;
	glPushMatrix();
		glTranslated(Pos.X, Pos.Y, Pos.Z);
		glRotated(-Ang.Yaw, 0, 0, 1);
		glRotated(-Ang.Pitch, 0, 1, 0);
		glRotated(-Ang.Roll, 1, 0, 0);
		glColor3d(1.0, 1.0, 1.0);

		glBegin(GL_LINES);
			glColor3d(1.0, 0.0, 0.0);
			for(unsigned int i = 0; i < m_VertCount; i+=3)
			{
				CVector& start = m_pVertexes[i];
				CVector end = (m_pNormals[i] * 0.1) + start;
				glVertex3d(start.X, start.Y, start.Z);
				glVertex3d(end.X, end.Y, end.Z);
			}
			glColor3d(0.0, 1.0, 0.0);
			for(unsigned int i = 1; i < m_VertCount; i+=3)
			{
				CVector& start = m_pVertexes[i];
				CVector end = (m_pNormals[i] * 0.1) + start;
				glVertex3d(start.X, start.Y, start.Z);
				glVertex3d(end.X, end.Y, end.Z);
			}
			glColor3d(0.0, 0.0, 1.0);
			for(unsigned int i = 2; i < m_VertCount; i+=3)
			{
				CVector& start = m_pVertexes[i];
				CVector end = (m_pNormals[i] * 0.1) + start;
				glVertex3d(start.X, start.Y, start.Z);
				glVertex3d(end.X, end.Y, end.Z);
			}
			glColor3d(1.0, 1.0, 1.0);
		glEnd();

	glPopMatrix();
}

void CModel::Draw(CVector& Pos, CAngle& Ang)
{
	glPushMatrix();
		glTranslated(Pos.X, Pos.Y, Pos.Z);
		glRotated(-Ang.Yaw, 0, 0, 1);
		glRotated(-Ang.Pitch, 0, 1, 0);
		glRotated(-Ang.Roll, 1, 0, 0);
		glColor3d(1.0, 1.0, 1.0);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		
			glBindBuffer(GL_ARRAY_BUFFER, m_VBOID); // Vertecies
			glVertexPointer(3, GL_DOUBLE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, m_TBOID); // Texture cords
			glTexCoordPointer(3, GL_DOUBLE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, m_NBOID); // And finally the normals
			glNormalPointer(GL_DOUBLE, 0, 0);

			glDrawArrays(GL_TRIANGLES, 0, m_VertCount);

		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();
}