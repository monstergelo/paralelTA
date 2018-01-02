#ifndef PARTICLE
#define PARTICLE

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define numMaterials 4
//===================================================================
class Material{
public:
	float mass, 
		  restDensity, 
		  stiffness, 
		  bulkViscosity, 
		  surfaceTension, 
		  kElastic, 
		  maxDeformation, 
		  meltRate, 
		  viscosity, 
		  damping, 
		  friction, 
		  stickiness, 
		  smoothing, 
		  gravity;
	int materialIndex;

	Material();
};
//===================================================================
class Particle{
public:
	void update();
	void draw();
	Particle(Material* mat);
	Particle(Material* mat, float x, float y);
	Particle(Material* mat, float x, float y, ColorA c);
	Particle(Material* mat, float x, float y, float u, float v);
	void initializeWeights(int gSizeY);

public:
	vec3		pos;
	vec3        trail;
	ColorA		color;

	Material* mat;
	float x, y, u, v, gu, gv, T00, T01, T11;
	int cx, cy, gi;
	float px[3];
	float py[3];
	float gx[3];
	float gy[3];
};
//===================================================================
class Node{
public:
	float mass, 
		  particleDensity, 
		  gx, 
		  gy, 
		  u, 
		  v,
		  u2,
		  v2,
		  ax,
		  ay;
	float cgx[numMaterials];
	float cgy[numMaterials];
	bool active;
	Node();
};
//===================================================================
class ParticleController{
public:
	float uscip(float p00, float x00, float y00, float p01, float x01, float y01, float p10, float x10, float y10, float p11, float x11, float y11, float u, float v);
	ParticleController();
	void initializeGrid(int sizeX, int sizeY);
	void addParticles();
	void update();

public:
	int gSizeX, 
		gSizeY, 
		gSizeY_3,
		scale;
	Node* grid;
	vector<Node*> active;
	vector<Particle> particles;
	Material materials[numMaterials];
};

#endif