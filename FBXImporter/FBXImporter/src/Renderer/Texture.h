#pragma once
#include <stb_image.h>
#include <string>

class Texture
{
public: // Methods
	Texture();
	Texture(std::string filepath);
	void Bind(unsigned int slot);
	unsigned int GetID();
	int GetWidth();
	int GetHeight();
	std::string& GetFilename();

private:
	unsigned int _ID = 0;
	std::string _filename;
	std::string _filetype;
	unsigned char* _data = nullptr;
	int _NumOfChannels = 0;
	int _width = 0;
	int _height = 0;
};
