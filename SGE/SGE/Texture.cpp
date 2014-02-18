#include "Texture.h"
#include "Engine.h"

#include <iostream>
#include <fstream>

using namespace std;

CTexture::CTexture()
{
	m_pPixelData = 0;
	m_BPP = 0;
	m_PixelDataLength = 0;
	m_TextureID = 0;
}

CTexture::~CTexture()
{
	Free();
}

void CTexture::Bind()
{
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

void CTexture::Free()
{
	if(m_pPixelData)
		delete [] m_pPixelData;

	if(m_TextureID)
		glDeleteTextures(1, &m_TextureID);

	m_pPixelData = 0;
	m_BPP = 0;
	m_PixelDataLength = 0;
	m_TextureID = 0;
}

struct TGA_Header_t
{
	short width;              // image width in pixels
	short height;             // image height in pixels
	unsigned char bits;               // image bits per pixel 8,16,24,32
	unsigned char descriptor;         // image descriptor bits (vh flip bits)
};

bool CTexture::LoadFromFile(const string& File)
{
	ifstream ifs(pEngineInstance->GetFileSystem()->GetTexture(File), ios::binary);

	if(!ifs.is_open())
		return false;

	bool done = false;
	
	unsigned char Header[12] = {0, 0, 2 /* RGB */, 0, 0, 0, 0, 0, 0, 0, /* Image start > */0, 0};
	unsigned char FileHeader[12];

	ifs.read((char*)Header, 12);
	if(ifs.fail())
		return false;

	if(!memcmp(Header, FileHeader, 12))
		return false; // We can't read this file, features not implemented...
	
	TGA_Header_t header;

	ifs.read((char*)&header, sizeof(TGA_Header_t));

	if(header.width <= 0 || header.height <= 0)
		return false;

	if(header.bits != 24 && header.bits != 32)
		return false;
	
	m_PixelDataLength = header.width * header.height * (header.bits / 8);
	m_pPixelData = new unsigned char[m_PixelDataLength];

	ifs.read((char*)m_pPixelData, m_PixelDataLength);

	if(ifs.fail())
	{
		cout << strerror(errno) << '\n';
		delete [] m_pPixelData;
		return false;
	}

	unsigned char tmp;
	for(unsigned int i = 0; i < m_PixelDataLength; i += header.bits / 8)
	{
		tmp = m_pPixelData[i + 0];
		m_pPixelData[i + 0] = m_pPixelData[i + 2];

		m_pPixelData[i + 2] = tmp;
	}

	unsigned int type = header.bits == 32 ? GL_RGBA : GL_RGB;
	unsigned int format = header.bits == 32 ? GL_BGRA : GL_BGR;
	Width = header.width;
	Height = header.height;

	glGenTextures(1, &m_TextureID);
 
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 
	glTexImage2D(GL_TEXTURE_2D, 0, type, Width, Height, 0, type, GL_UNSIGNED_BYTE, m_pPixelData);

	sge_assert(GLenum, !glGetError());
	return true;
}