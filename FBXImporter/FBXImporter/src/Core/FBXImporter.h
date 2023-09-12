#pragma once
#include <string>
#include <map>
#include <vector>
#include "../Renderer/Mesh.h"
#include "ofbx.h"

struct Animation {
	std::string _name;
};

struct SkinnedModel {
	std::vector<Mesh> _meshes;
	std::map<std::string, unsigned int> _boneMapping; 
	std::vector<Animation> _animations;
	std::vector<const ofbx::Object*> _bones;			// Ideally remove this obfx dependancy
};

namespace FBXImporter {

	void LoadFile(std::string filepath, SkinnedModel* out);

}