//
//  Rendering with Vertex Buffer Object, by Dody Dharma on May 2016
//
#include "../CinderProject/vc2013/CinderProjectApp.h"
//=============================================================================================================
void CinderProjectApp::setup()
{
	s.initializeGrid(400, 200);
	s.addParticles();
	s.scale = 4.0f;
	n = s.particles.size();
	printf("jumlah partikel %d", n);

	mParticleVbo = gl::Vbo::create(GL_ARRAY_BUFFER, s.particles, GL_STREAM_DRAW);
	// Describe particle semantics for GPU.
	geom::BufferLayout particleLayout;
	particleLayout.append(geom::Attrib::POSITION, 3, sizeof(Particle), offsetof(Particle, pos));
	particleLayout.append(geom::Attrib::CUSTOM_9, 3, sizeof(Particle), offsetof(Particle, trail));
	particleLayout.append(geom::Attrib::COLOR, 4, sizeof(Particle), offsetof(Particle, color));

	// Create mesh by pairing our particle layout with our particle Vbo.
	// A VboMesh is an array of layout + vbo pairs
	auto mesh = gl::VboMesh::create(s.particles.size(), GL_POINTS, { { particleLayout, mParticleVbo } });

#if ! defined( CINDER_GL_ES )
	// setup shader
	try {
		mGlsl = gl::GlslProg::create(
			CI_GLSL(150, 
				uniform     mat4    ciModelViewProjection;

				in          vec4    ciPosition;
				in          vec4    ciColor;
				in          vec4    trailPosition;

				out         vec4    vColor;
				out         vec4    trailPos;

				void main(void){
					gl_Position = ciModelViewProjection * ciPosition;
					trailPos = ciModelViewProjection * trailPosition;
					vColor = ciColor;
				}
			),
			CI_GLSL(150, 
				in       vec4 gColor;
				out      vec4 oColor;

				void main(void){
					oColor = gColor;
				}
			),
			CI_GLSL(150, 
				layout(points) in;
				layout(line_strip, max_vertices = 3) out;

				in vec4 trailPos[];
				in vec4 vColor[]; // Output from vertex shader for each vertex

				out vec4 gColor; // Output to fragment shader

				void main()
				{
					gColor = vColor[0];

					gl_Position = gl_in[0].gl_Position;
					EmitVertex();

					gl_Position = trailPos[0];
					EmitVertex();

					EndPrimitive();
				}
			)
		);
	}
	catch (gl::GlslProgCompileExc ex) {
		cout << ex.what() << endl;
		quit();
	}

	gl::Batch::AttributeMapping mapping({ { geom::Attrib::CUSTOM_9, "trailPosition" } });
	mParticleBatch = gl::Batch::create(mesh, mGlsl, mapping);
	gl::pointSize(4.0f);

#else
	mParticleBatch = gl::Batch::create(mesh, gl::GlslProg::create(loadAsset("draw_es3.vert"), loadAsset("draw_es3.frag")), mapping);
#endif

}

void CinderProjectApp::mouseDown(MouseEvent event)
{
}

void CinderProjectApp::update()
{
	s.update();

	// Copy particle data onto the GPU.
	// Map the GPU memory and write over it.
	void *gpuMem = mParticleVbo->mapReplace();
	memcpy(gpuMem, s.particles.data(), s.particles.size() * sizeof(Particle));
	mParticleVbo->unmap();
}

void CinderProjectApp::draw()
{
	// clear out the window with black
	gl::clear(Color(0, 0, 0));
	gl::setMatricesWindowPersp(getWindowSize());
	gl::enableDepthRead();
	gl::enableDepthWrite();

	mParticleBatch->draw();
	gl::drawString("Framerate: " + to_string(getAverageFps()), vec2(200.0f, 200.0f));
}


CINDER_APP(CinderProjectApp, RendererGl, [](App::Settings *settings) {
	settings->setWindowSize(1600, 800);
	settings->setMultiTouchEnabled(false);
})