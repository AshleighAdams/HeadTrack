#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

class CTexture
{
public:
	CTexture();
	~CTexture();
	bool LoadFromFile(const std::string& File);
	void Bind();

	unsigned Width;
	unsigned Height;
private:
	void Free();

	unsigned int m_TextureID;
	int m_BPP;
	unsigned char* m_pPixelData;
	unsigned int m_PixelDataLength;
};

#endif