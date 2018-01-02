#ifndef CINDER
#define CINDER

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"

#include "../CinderProject/vc2013/Particle.h"
#include "Resources.h"
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderProjectApp : public App {
	ParticleController s;
	int n;
	GLfloat*            vertices;
	ColorA*             colors;

	// Buffer holding raw particle data on GPU, written to every update().
	gl::VboRef			mParticleVbo;
	// Batch for rendering particles with  shader.
	gl::BatchRef		mParticleBatch;
	gl::GlslProgRef     mGlsl;

public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;
	void prepareSettings(Settings *settings);
};

#endif