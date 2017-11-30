#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;
using namespace std;
//===================================================================
class Particle{
public :
	Particle(vec2 loc);
	void update();
	void draw();

public:
	vec2 mLoc;
	vec2 mDir;
	vec2 mAccel;
	vec2 mVel;
	float limitVelx;
	float limitVely;
	float mRadius;
};

Particle::Particle(vec2 loc){
	mLoc = loc;
	mDir = Rand::randVec2();
	mVel = Rand::randVec2();
	float x = (Rand::randFloat() -0.5) * 20;
	float y = Rand::randFloat() * -10;
	mAccel = vec2(x, y);
	mRadius = 2.5f;
	limitVelx = 10.0f;
	limitVely = 10.0f;
}

void Particle::update(){
	//friction x-axis
	if (mVel.x != 0){
		mAccel.x += -mVel.x*0.1;
	}

	//apply gravity
	mAccel.y += 2.0;

	//stop x movement
	if (mVel.x < 0.2){
		mVel.x = 0;
	}

	//change velocity
	mVel += mAccel;

	//limit velocity
	if (mVel.x > limitVelx){
		mVel.x = limitVelx;
	}
	if (mVel.y > limitVely){
		mVel.y = limitVely;
	}

	//change position
	mLoc += mVel;
}

void Particle::draw(){
	gl::drawSolidCircle(mLoc, mRadius);
}
//===================================================================
class ParticleController{
public:
	ParticleController(){
		//create template circle
		gl::GlslProgRef solidShader = gl::GlslProg::create(
			// vertex code
			CI_GLSL(150,
			uniform mat4        ciModelViewProjection;
		in vec4                ciPosition;
		in vec4                ciColor;
		in vec4					newPosition;
		out lowp vec4        Color;
		void main(void)
		{
			vec4 position = ciPosition;
			position.xyz += newPosition.xyz;
			gl_Position = ciModelViewProjection * position;
			Color = ciColor;
		}
		),
			// fragment code
			CI_GLSL(150,
			in vec4          Color;
		out vec4         oColor;
		void main(void)
		{
			oColor = Color;
		}
		)
			);
		//[commentout]//batch = gl::Batch::create(geom::Circle().radius(30), solidShader);

		//create holder for circle data(location and such)
		mInstanceData.resize(1024);
		for (auto &circle : mInstanceData) {
			circle.x = Rand::randFloat(float(getWindowWidth()));
			circle.y = Rand::randFloat(float(getWindowHeight()));
			circle.z = 0.0f;
			circle.w = 1.0f;
		}

		geom::BufferLayout instanceDataLayout;
		instanceDataLayout.append(geom::Attrib::CUSTOM_0, 4, 0 /* stride */, 0 /* offset */, 1 /* per instance */);

		mInstanceVbo = gl::Vbo::create(GL_ARRAY_BUFFER, mInstanceData.size() * sizeof(ci::vec4), mInstanceData.data(), GL_DYNAMIC_DRAW);

		// Create a mesh for our circle. Position it with its center at the origin with a radius of 1.
		// This makes it easy to manipulate the position and radius in our vertex shader.
		mVboMesh = gl::VboMesh::create(geom::Circle().center(vec2(0)).radius(4).subdivisions(30));

		// Append the instanced data to the mesh, so we have access to all the data in our vertex shader.
		mVboMesh->appendVbo(instanceDataLayout, mInstanceVbo);

		batch = gl::Batch::create(mVboMesh, solidShader, { { geom::Attrib::CUSTOM_0, "newPosition" } });
	}

	void update(){
		int width = app::getWindowWidth();
		int height = app::getWindowHeight();
		for (list<Particle>::iterator p = particles.begin(); p != particles.end(); p){
			p->update();

			if		(p->mLoc.x > width  || p->mLoc.x < 0) p = particles.erase(p);
			else if (p->mLoc.y > height || p->mLoc.y < 0) p = particles.erase(p);
			else p++;
		}

		//update batch
		auto ptr = (ci::vec4 *)mInstanceVbo->mapReplace();
		for (list<Particle>::iterator p = particles.begin(); p != particles.end(); p++){
			vec4 circle = vec4(p->mLoc.x, p->mLoc.y, 0, 1);

			*ptr++ = circle;
		}
		mInstanceVbo->unmap();
	}

	void draw(){
		batch->drawInstanced(particles.size());
	}

	void AddParticle(int n){
		for (int i = 0; i < n; i++){
			float x = Rand::randFloat(app::getWindowWidth());
			float y = Rand::randFloat(app::getWindowHeight());
			Particle p = Particle(vec2(x, y));
			particles.push_back(p);
		}
	}

	void AddParticle(int n, double x, double y){
		for (int i = 0; i < n; i++){
			Particle p = Particle(vec2(x, y));
			particles.push_back(p);
		}
	}

	void AddParticle(vec2 loc){
		Particle p = Particle(loc);
		particles.push_back(p);
	}

public:
	std::list<Particle> particles;
	gl::BatchRef batch;
	std::vector<ci::vec4> mInstanceData;
	gl::VboRef            mInstanceVbo;
	gl::VboMeshRef        mVboMesh;
};
//===================================================================
class CinderProjectApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

public:
	ParticleController p;
	int delay = 0;
};

void CinderProjectApp::setup()
{
	p.AddParticle(50);	
}

void CinderProjectApp::mouseDown( MouseEvent event )
{
	if (event.isLeft()){
		console() << event.getPos() << std::endl;
		p.AddParticle(event.getPos());
	}
}

void CinderProjectApp::update()
{
	p.update();
	delay++;
	if (delay > 0){
		p.AddParticle(5, 200, 200);
		delay = 0;
	}
}

void CinderProjectApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	p.draw();
}

CINDER_APP( CinderProjectApp, RendererGl ) 
