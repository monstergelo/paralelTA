#include "cinder/Rand.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class InstancedCirclesApp : public App {
public:
	static void prepare(Settings *settings);

	void setup() override;
	void update() override;
	void draw() override;

private:
	std::vector<ci::vec4> mInstanceData;
	size_t                mInstanceCount;
	gl::GlslProgRef       mGlslProg;
	gl::VboRef            mInstanceVbo;
	gl::VboMeshRef        mVboMesh;
	gl::BatchRef          mBatch;
};

void InstancedCirclesApp::prepare(Settings *settings)
{
	settings->setDefaultWindowFormat(Window::Format().resizable(false).size(800, 600));
	settings->disableFrameRate();
}

void InstancedCirclesApp::setup()
{
	mInstanceCount = 0;

	// Create a bunch of randomly positioned circles and store them in our vector.
	// The w-component will hold the radius. Since this is our initial data, the
	// radius value is not important, so we set it to 1. See update().
	mInstanceData.resize(1024);
	for (auto &circle : mInstanceData) {
		circle.x = Rand::randFloat(float(getWindowWidth()));
		circle.y = Rand::randFloat(float(getWindowHeight()));
		circle.z = 0.0f;
		circle.w = 1.0f;
	}

	// Create a buffer for our instanced data. We'll use this buffer to offset and scale the mesh
	// and render it once for each circle instance. The buffer will be used to update the CUSTOM_0 attribute.
	geom::BufferLayout instanceDataLayout;
	instanceDataLayout.append(geom::Attrib::CUSTOM_0, 4, 0 /* stride */, 0 /* offset */, 1 /* per instance */);

	mInstanceVbo = gl::Vbo::create(GL_ARRAY_BUFFER, mInstanceData.size() * sizeof(ci::vec4), mInstanceData.data(), GL_DYNAMIC_DRAW);

	// Create a mesh for our circle. Position it with its center at the origin with a radius of 1.
	// This makes it easy to manipulate the position and radius in our vertex shader.
	mVboMesh = gl::VboMesh::create(geom::Circle().center(vec2(0)).radius(1).subdivisions(30));

	// Append the instanced data to the mesh, so we have access to all the data in our vertex shader.
	mVboMesh->appendVbo(instanceDataLayout, mInstanceVbo);

	// Create the shader.
	auto fmt = gl::GlslProg::Format();
	fmt.version(150);
	fmt.vertex(
		"uniform mat4 ciModelViewProjection;\n"

		"in vec4 ciPosition;\n"         // The vertices of our geom::Circle.
		"in vec4 ciColor;\n"            // The current draw color.
		"in vec4 iPositionAndRadius;\n" // The per-instance position and radius of our circles.

		"out vec4 vertColor;\n"

		"void main(void) {\n"
		// Scale and translate the circle.
		"	vec4 position = ciPosition;\n"
		"	position.xyz += iPositionAndRadius.xyz;\n"
		// Output the vertex color.
		"	vertColor = ciColor;\n"
		// Required: output the clip space position.
		"	gl_Position = ciModelViewProjection * position;\n"
		"}");
	fmt.fragment(
		"in vec4 vertColor;\n"

		"out vec4 fragColor;\n"

		"void main(void) {\n"
		"	fragColor = vertColor;\n"
		"}");

	mGlslProg = gl::GlslProg::create(fmt);

	// Create the batch. Tell the shader it can find the 'iPositionAndRadius' attribute in the CUSTOM_0 location.
	mBatch = gl::Batch::create(mVboMesh, mGlslProg, { { geom::Attrib::CUSTOM_0, "iPositionAndRadius" } });

	// Disable vertical sync, so we can see the maximum frame rate.
	gl::enableVerticalSync(false);
}

void InstancedCirclesApp::update()
{
	Rand::randSeed(12345);

	// As an example, we'll animate the circle's radii. If the radius is less than zero,
	// the circle will be invisible and should not be drawn. In that case, we'll skip it.

	mInstanceCount = 0;

	auto ptr = (ci::vec4 *)mInstanceVbo->mapReplace(); // Using mapReplace() will give us the best performance. The GPU can discard existing data.
	for (auto &circle : mInstanceData) {
		circle.w = float(5.0 + 10.0 * sin(Rand::randInt() + getElapsedSeconds()));

		if (circle.w > 0.0f) {
			*ptr++ = circle; // Copy circle to the buffer, then increment the pointer.
			mInstanceCount++;
		}
	}
	mInstanceVbo->unmap(); // Never forget to unmap.
}

void InstancedCirclesApp::draw()
{
	gl::clear();

	if (mInstanceCount > 0) {
		// Draw the geom::Circle 'mInstanceCount' times. For each instance, use the next value
		// in our iPositionAndRadius buffer. The vertex shader will adjust the vertices and
		// render the circle with the correct size and position.
		mBatch->drawInstanced(mInstanceCount);
	}

	// Render FPS and instance count.
	{
		gl::ScopedColor      scpColor(0, 0, 0, 0.8f);
		gl::ScopedBlendAlpha scpBlend;
		gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), 40));

		std::stringstream strFps, strCount;
		strFps << int(getAverageFps()) << " FPS";
		strCount << mInstanceCount << " Instances";
		gl::drawString(strFps.str(), vec2(10, 10), Color(1, 1, 0));
		gl::drawString(strCount.str(), vec2(10, 20), Color(1, 1, 0));
	}
}

//CINDER_APP(InstancedCirclesApp, RendererGl(RendererGl::Options().msaa(16)), &InstancedCirclesApp::prepare)