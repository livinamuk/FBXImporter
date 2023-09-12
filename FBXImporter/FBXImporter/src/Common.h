#pragma once
#define GLM_FORCE_SILENT_WARNINGS
#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Math.h"

#define HELL_PI 3.141592653589793f

#define MAP_WIDTH   32//24//48//24
#define MAP_HEIGHT  22
#define MAP_DEPTH   36

#define BLACK   glm::vec3(0,0,0)
#define WHITE   glm::vec3(1,1,1)
#define RED     glm::vec3(1,0,0)
#define GREEN   glm::vec3(0,1,0)
#define BLUE    glm::vec3(0,0,1)
#define YELLOW  glm::vec3(1,1,0)
#define PURPLE  glm::vec3(1,0,1)
#define GREY    glm::vec3(0.25f)
#define LIGHT_BLUE    glm::vec3(0,1,1)
#define GRID_COLOR    glm::vec3(0.509, 0.333, 0.490) * 0.5f

#define SMALL_NUMBER		(float)9.99999993922529e-9
#define KINDA_SMALL_NUMBER	(float)0.00001
#define MIN_RAY_DIST        (float)0.01f

#define NRM_X_FORWARD glm::vec3(1,0,0)
#define NRM_X_BACK glm::vec3(-1,0,0)
#define NRM_Y_UP glm::vec3(0,1,0)
#define NRM_Y_DOWN glm::vec3(0,-1,0)
#define NRM_Z_FORWARD glm::vec3(0,0,1)
#define NRM_Z_BACK glm::vec3(0,0,-1)

struct Transform {
	glm::vec3 position = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 scale = glm::vec3(1);
	glm::mat4 to_mat4() {
		glm::mat4 m = glm::translate(glm::mat4(1), position);
		m *= glm::mat4_cast(glm::quat(rotation));
		m = glm::scale(m, scale);
		return m;
	};
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

struct VertexWeighted {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    int blendingIndex[4];
    glm::vec4 blendingWeight;

    //bool operator==(const VertexWeighted& other) const {
	//    return position == other.position && normal == other.normal && uv == other.uv;
    //}
};

struct Point {
    glm::vec3 pos = { glm::vec3(0) };
    glm::vec3 color = { glm::vec3(0) };
    Point() {};
    Point(glm::vec3 pos, glm::vec3 color) {
        this->pos = pos;
        this->color = color;
    }
};

struct Line {
    Point p1;
    Point p2;
    Line() {};
    Line(glm::vec3 start, glm::vec3 end, glm::vec3 color) {
        p1.pos = start;
        p2.pos = end;
        p1.color = color;
        p2.color = color;
    }
};

struct VoxelFace {

    int x = 0;
    int y = 0;
    int z = 0;
    glm::vec3 baseColor;
    glm::vec3 normal;
    glm::vec3 accumulatedDirectLighting = { glm::vec3(0) };
    glm::vec3 indirectLighting = { glm::vec3(0) };

    VoxelFace(int x, int y, int z, glm::vec3 baseColor, glm::vec3 normal) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->baseColor = baseColor;
        this->normal = normal;
    }
};



struct Light {
    int x = 0;
    int y = 0;
    int z = 0;
    float strength = 1.0f;
    float radius = 6.0f;
    glm::vec3 color = glm::vec3(1, 0.7799999713897705, 0.5289999842643738);
};

struct Triangle {
    glm::vec3 p1;
    glm::vec3 p2;
    glm::vec3 p3;
    glm::vec3 normal;
    glm::vec3 color;
    //void* parent = nullptr;
};

struct IntersectionResult {
    bool found = false;
    float distance = 0;
    float dot = 0;
    glm::vec2 baryPosition = glm::vec2(0);
};

struct GridProbe {
    glm::vec3 worldPositon;
    glm::vec3 color = BLACK;
    int samplesRecieved = 0;
    bool ignore = false; // either blocked by geometry, or out of map range
};

struct Extent2Di {
    int width;
    int height;
};

struct UIRenderInfo {
    std::string textureName;
    int screenX;
    int screenY;
    glm::mat4 modelMatrix;
    bool centered = false;
};