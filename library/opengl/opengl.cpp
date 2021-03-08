#include <library/opengl/opengl.hpp>

#include <library/log.hpp>
#include <library/opengl/fbo.hpp>
#include <stdexcept>
#include <string>

namespace library
{
	// set opengl defaults, read function entry points
	OpenGL::OpenGL(bool core_context)
	{
		//-== resolve openGL extensions ==-//
		if (core_context) glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			logger << Log::ERR << "OpenGL::init(): Failed to initialize GLEW" << Log::ENDL;
			throw std::runtime_error("OpenGL::init(): GLEW initialization error");
		}

		/// cheap tests for some basic driver support
		this->supportsVBO = glGenBuffers != nullptr;
		this->supportsVAO = glGenVertexArrays != nullptr;

		this->supportsFramebuffers = glGenFramebuffers != nullptr;
		this->supportsShaders = glCreateProgram != nullptr;
		this->supportsAttribs = glVertexAttribPointer != nullptr;

		this->supportsGenMipmap = glGenerateMipmap != nullptr;

		// slightly more complicated texture arrays test
		GLuint test;
		glGenTextures(1, &test);
		glBindTexture(GL_TEXTURE_2D_ARRAY, test);
		glDeleteTextures(1, &test);
		this->supportsTextureArrays = (glGetError() == 0);

		// cleared colorbuffer value
		glClearColor(0.0, 0.0, 0.0, 0.0);
		// cleared depth values
		glClearDepth(1.0);
		// full depth range
		glDepthRange(0.0, 1.0);

		// default backface culling & filled shading
		glCullFace(GL_BACK);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// default blend function
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// check if any bad things happened during initialization
		if (checkError()) {
			throw std::runtime_error("OpenGL::init(): Initialization error");
		}
	}

	bool OpenGL::checkError()
	{
		GLenum error = glGetError();
		if (error)
		{
			std::string errorString;
			switch (error)
			{
			case GL_INVALID_ENUM:
				errorString = "Invalid enumeration parameter (GL_INVALID_ENUM)";
				break;
			case GL_INVALID_VALUE:
				errorString = "Invalid value for parameter (GL_INVALID_VALUE)";
				break;
			case GL_INVALID_OPERATION:
				errorString = "Invalid operation for current state (GL_INVALID_OPERATION)";
				break;

			case GL_INVALID_FRAMEBUFFER_OPERATION:
				errorString = "Invalid operation on incomplete framebuffer (GL_INVALID_FRAMEBUFFER_OPERATION)\n"
							"Framebuffer error: " + FBO::errorString();
				break;

			default:
				errorString = "Unknown error";
			}

			logger << Log::ERR << "OpenGL error(" << error << "): " << errorString << Log::ENDL;
			return true;
		}
		return false;
	}

}
