#ifdef TEST_BONERIG
// opengl window
#include <library/opengl/window.hpp>
// matrix and vector
#include <library/math/vector.hpp>
#include <library/math/matrix.hpp>
// bones
#include <library/math/kine/bone.hpp>
// stuff
#include <library/log.hpp>
#include <library/sleep.hpp>

#include <GL/glfw3.h>
#include <iostream>
#include <cmath>

using namespace library;

Bone motherBone;
const double PI = 4 * atan(1);

void renderPivot(const mat4& comp)
{
	vec4 O(0.0, 0.0, 0.0, 1.0);
	vec4 X(1.0, 0.0, 0.0, 1.0);
	vec4 Y(0.0, 1.0, 0.0, 1.0);
	vec4 Z(0.0, 0.0, 1.0, 1.0);
	
	glBegin(GL_LINES);
		// X-axis
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(O.x, O.y, O.z);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(X.x, X.y, X.z);
		// Y-axis
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(O.x, O.y, O.z);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(Y.x, Y.y, Y.z);
		// Z-axis
		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(O.x, O.y, O.z);
		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(Z.x, Z.y, Z.z);
	glEnd();
	
	/*
	vec4 tx = comp * X;
	vec4 ty = comp * Y;
	vec4 tz = comp * Z;
	
	logger << Log::INFO << cross(tx.xyz(), ty.xyz()) << " --> " << tz.xyz() << Log::ENDL;
	*/
}

void renderTail(float rad)
{
	glBegin(GL_QUADS);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(-rad, -rad, 0.0);
		
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f( rad, -rad, 0.0);
		
		glColor3f(0.0, 0.0, 1.0);
		glVertex3f( rad,  rad, 0.0);
		
		glColor3f(0.0, 0.0, 0.0);
		glVertex3f(-rad,  rad, 0.0);
	glEnd();
}

// render this bone
void renderBone(Bone& bone, mat4& matview, double time)
{
	// modulate bone
	
	// send mv matrix
	mat4 mv = matview * bone.getMatrix();
	glLoadMatrixf(mv.data());
	
	// pivot axes
	renderPivot(bone.getMatrix());
	// graphics
	renderTail(0.5);
	// children
	for (int i = 0; i < bone.childCount(); i++)
	{
		renderBone(bone[i], matview, time);
	}
	
	// rotate back and forth over time
	bone.rotate( Quaternion(vec3(0, 1, 0), 0.002 * sin(time * 4.0)) );
}

bool boneRenderer(WindowClass& wnd, double dtime, double timeElapsed)
{
	glLineWidth(2.0f);
	
	// depth testing
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	
	glClearColor(0.2, 0.4, 0.8, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	// set up perspective projection matrix
	mat4 matproj = perspectiveMatrix(61.0, wnd.getAspect(), 0.1, 40.0);
	
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(matproj.data());
	
	mat4 matview = rotationMatrix(0.0, 0.0, 0.0);
	matview.translate(4.0, -2.0, -32.0);
	
	glMatrixMode(GL_MODELVIEW);
	
	renderBone(motherBone, matview, timeElapsed);
	
	// swap frontbuffer with backbuffer (frame change)
	glfwSwapBuffers(wnd.window());
	
	// close window after N seconds
	return (timeElapsed < 6);
}

void test_opengl_bonerig()
{
	std::cout << "Opening OpenGL window" << std::endl;
	
	WindowConfig wndconf;
	wndconf.fullscreen = false;
	wndconf.SW = 800;
	wndconf.SH = 600;
	wndconf.multisample = 2;
	wndconf.vsync = true;
	
	// open up OpenGL window
	WindowClass renderer;
	// throws error if something bad happens...
	renderer.open(wndconf);
	
	// create bone rig
	motherBone = Bone(vec3(0.0, 0.0, 0.0), Quaternion());
	
	Bone* current = &motherBone;
	for (int i = 0; i < 64; i++)
	{
		current->addChild(Bone(vec3(0.0, 0.0, 1.0), vec3(0.2, 0.2, 0.2)));
		current = &current[0][0];
	}
	
	//////////////////////////////////////////////////////////////////
	std::cout << "Starting rendering loop: Bone rig" << std::endl;
	std::cout << "Window closes after 5 seconds" << std::endl;
	
	// what we want delta-time to be measured in:
	// 1ms granularity, meaning 1.0 dtime is 1ms, 100.0 is 100ms etc.
	// Examples: 1/1000 = 1ms/frame, 1/60 = 16.7ms/frame (60fps) etc.
	// And finally, 1.0 means dtime is measured in (whole) seconds
	const double timing_granularity = 1.0;
	
	renderer.startRenderingLoop(boneRenderer, timing_granularity);
	renderer.waitClose();
	renderer.close();
	
}
#endif
