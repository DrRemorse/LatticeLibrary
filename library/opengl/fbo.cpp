#include <library/opengl/fbo.hpp>

#include <library/log.hpp>
//#include <library/opengl/opengl.hpp>
#include <library/opengl/texture.hpp>
#include <cassert>

namespace library
{
	GLuint FBO::lastFBO = 0;

  FBO::~FBO()
  {
    if (this->fbo != 0) {
      glDeleteFramebuffers(1, &this->fbo);
    }
  }

	void FBO::create()
	{
    // remove old, if any
    if (this->fbo != 0)
    {
      glDeleteFramebuffers(1, &this->fbo);
    }
		// check for framebuffers support
    assert(glGenFramebuffers != nullptr && "OpenGL was not initialized, or does not support framebuffers");
		glGenFramebuffers(1, &this->fbo);
		assert(this->fbo != 0 && "FBO handle cannot be zero");
	}

	void FBO::attachColor(GLenum index, Texture& texture)
	{
		attachColor(index, texture.getHandle());
	}
	void FBO::attachColor(GLenum index, GLuint texture)
	{
    assert(fbo != 0 && "FBO was not yet created");
		if (lastFBO != fbo) throw std::runtime_error("FBO was not currently bound, cannot attach color target");

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, texture, 0);
	}

	void FBO::removeColor(GLenum index)
	{
    assert(fbo != 0 && "FBO was not yet created");
		if (lastFBO != fbo) throw std::runtime_error("FBO was not currently bound, cannot attach color target");

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, 0, 0);
	}

	void FBO::attachDepth(Texture& texture)
	{
		attachDepth(texture.getHandle());
	}
	void FBO::attachDepth(GLuint texture)
	{
    assert(fbo != 0 && "FBO was not yet created");
		if (lastFBO != fbo) throw std::runtime_error("FBO was not currently bound, cannot attach color target");

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
	}
	void FBO::removeDepth()
	{
    assert(fbo != 0 && "FBO was not yet created");
		if (lastFBO != fbo) throw std::runtime_error("FBO was not currently bound, cannot attach color target");

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
	}

	void FBO::createDepthRBO(int width, int height, GLenum internalFormat)
	{
		// depth renderbuffer
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		// attach the rbo to this fbo
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (OpenGL::checkError())
		{
			logger << Log::ERR << "FBO::createDepthRBO(): Error creating depth renderbuffer" << Log::ENDL;
			throw std::runtime_error("FBO::createDepthRBO(): Error creating depth renderbuffer");
		}
	}
	void FBO::attachDepthRBO(const FBO& fboWithRBO)
	{
		rbo = fboWithRBO.rbo;
		// attach the rbo to this fbo
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (OpenGL::checkError())
		{
			logger << Log::ERR << "FBO::createDepthRBO(): Error creating depth renderbuffer" << Log::ENDL;
			throw std::runtime_error("FBO::createDepthRBO(): Error creating depth renderbuffer");
		}
	}

	void FBO::createMsaaRBO(int width, int height, int samples)
	{
		// depth renderbuffer
		GLuint rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		// attach the rbo to this fbo
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

		if (isComplete() == false)
		{
			throw std::runtime_error(errorString());
		}

		if (OpenGL::checkError())
		{
			logger << Log::ERR << "FBO::createMsaaRBO(): Error creating msaa renderbuffer" << Log::ENDL;
			throw std::runtime_error("FBO::createMsaaRBO(): Error creating msaa renderbuffer");
		}
	}

	void FBO::createDepthMsaaRBO(int width, int height, int samples)
	{
		// depth renderbuffer
		GLuint rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		// attach the rbo to this fbo
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (isComplete() == false)
		{
			throw std::runtime_error(errorString());
		}

		if (OpenGL::checkError())
		{
			logger << Log::ERR << "FBO::createDepthMsaaRBO(): Error creating msaa depth renderbuffer" << Log::ENDL;
			throw std::runtime_error("FBO::createDepthMsaaRBO(): Error creating msaa depth renderbuffer");
		}
	}

	void FBO::drawBuffers(std::vector<int> buffers)
	{
		glDrawBuffers(buffers.size(), (GLenum*) buffers.data());

		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "FBO::drawBuffers(vector): Error setting draw buffers" << Log::ENDL;
			throw std::runtime_error("FBO::drawBuffers(vector): Error setting draw buffers");
		}
		#endif
	}
	void FBO::drawBuffer(GLenum buffer)
	{
		glDrawBuffers(1, &buffer);

		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "FBO::drawBuffer(GLenum): Error setting draw buffers" << Log::ENDL;
			throw std::runtime_error("FBO::drawBuffer(GLenum): Error setting draw buffers");
		}
		#endif
	}
	void FBO::drawBuffer()
	{
		GLenum one = GL_COLOR_ATTACHMENT0;
		glDrawBuffers(1, &one);

		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "FBO::drawBuffer(): Error setting draw buffer" << Log::ENDL;
			throw std::runtime_error("FBO::drawBuffer(): Error setting draw buffer");
		}
		#endif
	}

	void FBO::blitTo(FBO& dest_fbo, int w, int h, int mask, int filter)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, this->fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_fbo.getHandle());

		glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, mask, filter);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	void FBO::blitTo(FBO& dest_fbo, int w, int h, int w2, int h2, int mask, int filter)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, this->fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_fbo.getHandle());

		glBlitFramebuffer(0, 0, w, h, 0, 0, w2, h2, mask, filter);

		//glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	bool FBO::isComplete()
	{
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		return status == GL_FRAMEBUFFER_COMPLETE;
	}

	std::string FBO::errorString()
	{
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			switch (status)
			{
				case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
					return std::string("Incomplete attachment");

				case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
					return std::string("(Incomplete) Missing attachment");

				case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
					return std::string("(Incomplete) Layer targets");

				case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
					return std::string("Incomplete draw buffer");

				case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
					return std::string("Incomplete multisample format");

				case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
					return std::string("Incomplete read buffer");

				case GL_FRAMEBUFFER_UNSUPPORTED:
					return std::string("Unsupported format");

			}
			return std::string("Framebuffer was incomplete, but unknown error");
		}
		return std::string("Framebuffer was complete (no error)");
	}

}
