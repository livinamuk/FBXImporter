#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "FBXImporter.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "../Common.h"

int decodeIndex(int idx) {
	return (idx < 0) ? (-idx - 1) : idx;
}

std::string DataViewToString(ofbx::DataView data) {
	char out[128];
	data.toString(out);
	std::string result(out);
	return result;
}

void FBXImporter::LoadFile(std::string filepath, SkinnedModel* out) {

	std::ofstream outfile;
	FILE* fp = fopen(filepath.c_str(), "rb");
	if (!fp) {
		std::cout << "Failed to open: " << filepath << "\n";
		return;
	}
	fseek(fp, 0, SEEK_END);
	long file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	auto* content = new ofbx::u8[file_size];
	fread(content, 1, file_size, fp);
	ofbx::IScene*  scene = ofbx::load((ofbx::u8*)content, file_size, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
	delete[] content;
	fclose(fp);

	// Find all the bones
	for (int k = 0; k < scene->getMeshCount(); k++)
	{
		auto* fbxMesh = scene->getMesh(k);
		auto* geometry = fbxMesh->getGeometry();
		auto* skin = geometry->getSkin();

		// Bones
		if (skin) {
			for (int i = 0; i < skin->getClusterCount(); i++) {
				const auto* cluster = skin->getCluster(i);
				const ofbx::Object* link = cluster->getLink();
				if (link) {
					// Add bone if it doesn't already exist in the bones vector
					if (std::find(out->_bones.begin(), out->_bones.end(), link) != out->_bones.end()) {
					}
					else {
						std::cout << "Added bone: " << link->name << "\n";
						out->_bones.push_back(link);
					}
				}
			}
		}
		// Sort the bones into a flat array, so that no child is before its parent (required for skinning)
		for (int i = 0; i < out->_bones.size(); ++i) {
			for (int j = i + 1; j < out->_bones.size(); ++j) {
				if (out->_bones[i]->getParent() == out->_bones[j]) {
					const ofbx::Object* bone = out->_bones[j];
					out->_bones.erase(out->_bones.begin() + j);
					out->_bones.insert(out->_bones.begin() + i, bone);
					--i;
					break;
				}
			}
		}
		// Map bone names to their index
		for (int i = 0; i < out->_bones.size(); ++i) {
			out->_boneMapping[out->_bones[i]->name] = i;
		}
	}


	// Load vertex and index data
	for (int i = 0; i < scene->getMeshCount(); i++)
	{
		auto* fbxMesh = scene->getMesh(i);
		auto* geometry = fbxMesh->getGeometry();
		auto* skin = geometry->getSkin();
		auto* faceIndicies = geometry->getFaceIndices();
		auto indexCount = geometry->getIndexCount();

		// Get indices
		std::vector<unsigned int> indices;
		for (int q = 0; q < indexCount; q++) {
			int index = decodeIndex(faceIndicies[q]);
			indices.push_back(index);
		}

		// Get vertices
		std::vector<VertexWeighted> vertices;
		for (int j = 0; j < geometry->getVertexCount(); j++)
		{
			auto& v = geometry->getVertices()[j];
			auto& n = geometry->getNormals()[j];
			auto& t = geometry->getTangents()[j];
			auto& uv = geometry->getUVs()[j];

			VertexWeighted vertex;
			vertex.position = glm::vec3(v.x, v.y, v.z);
			vertex.normal = glm::vec3(n.x, n.y, n.z);

			if (geometry->getTangents() != nullptr) {
				vertex.tangent = glm::vec3(t.x, t.y, t.z);
				vertex.bitangent = glm::cross(vertex.tangent, vertex.normal);
			}
			if (geometry->getUVs() != nullptr)
				vertex.uv = glm::vec2(uv.x, 1.0f - uv.y);

			vertex.blendingIndex[0] = -1;
			vertex.blendingIndex[1] = -1;
			vertex.blendingIndex[2] = -1;
			vertex.blendingIndex[3] = -1;
			vertex.blendingWeight = glm::vec4(0, 0, 0, 0);
			vertices.push_back(vertex);
		}

		struct Weight {
			float influence;
			int boneID;
		};

		std::vector<std::vector<Weight>> vertexWeights;
		vertexWeights.resize(vertices.size());

		if (skin) {

			// Get vertex weights
			for (int i = 0; i < skin->getClusterCount(); i++)
			{
				const auto* cluster = skin->getCluster(i);
				const ofbx::Object* link = cluster->getLink();

				for (int j = 0; j < cluster->getIndicesCount(); ++j) {
					const int vertexIndex = cluster->getIndices()[j];
					const float influence = (float)cluster->getWeights()[j];
					const int boneIndex = out->_boneMapping[cluster->getLink()->name];
					vertexWeights[vertexIndex].push_back({ influence, boneIndex });
				}
			}
		}

		// Sort vertex weights by influence
		auto compare = [](Weight a, Weight b) {
			return (a.influence > b.influence);
		};

		for (int i = 0; i < vertexWeights.size(); i++) {
			std::sort(vertexWeights[i].begin(), vertexWeights[i].end(), compare);

			// Add the weight info to the actual main vertices
			for (int j = 0; j < 4 && j < vertexWeights[i].size(); j++) {
				vertices[i].blendingWeight[j] = vertexWeights[i][j].influence;
				vertices[i].blendingIndex[j] = vertexWeights[i][j].boneID;
			}
		}

		// Create the mesh, aka send the data to OpenGL
		out->_meshes.push_back(Mesh(vertices, indices, fbxMesh->name));



		////////////////////////////////////////////////////////////
		//														  //
		//	The code from here on is not working correctly !!!!!  //
		//														  //
		////////////////////////////////////////////////////////////



		// Gather animations
		for (int i = 0; i < scene->getAnimationStackCount(); ++i) {
			Animation& anim = out->_animations.emplace_back(Animation());

			const ofbx::AnimationStack* animationStack = (ofbx::AnimationStack*)scene->getAnimationStack(i);
			const ofbx::TakeInfo* takeInfo = scene->getTakeInfo(animationStack->name);

			if (takeInfo) {
				if (takeInfo->name.begin != takeInfo->name.end) {
					anim._name = DataViewToString(takeInfo->name);
				}
				if (anim._name.empty() && takeInfo->filename.begin != takeInfo->filename.end) {
					anim._name = DataViewToString(takeInfo->filename);
				}
				if (anim._name.empty()) anim._name = "anim";
			}
			else {
				anim._name = "";
			}

			const ofbx::AnimationLayer* anim_layer = animationStack->getLayer(0);
			if (!anim_layer || !anim_layer->getCurveNode(0)) {
				out->_animations.pop_back();
			}

			bool data_found = false;
			for (int k = 0; anim_layer->getCurveNode(k); ++k) {
				const ofbx::AnimationCurveNode* node = anim_layer->getCurveNode(k);
				std::string tempName = node->name;
				if (tempName == "R" || tempName == "T") {
					data_found = true;
					break;
				}
				else {
					std::cout << "something is the fuck up, look here\n";
				}
			}
			if (!data_found) out->_animations.pop_back();
		}

		if (out->_animations.size() == 1) {
			out->_animations[0]._name = "";
		}

		for (Animation& animation : out->_animations) {
			std::cout << "Animation found: " << animation._name << "\n";
		}
	}
}
