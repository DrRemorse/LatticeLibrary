#include <library/opengl/texture.hpp>

#include <library/log.hpp>
#include <library/bitmap/bitmap.hpp>
#include <library/opengl/opengl.hpp>
#include <cmath>
#include <sstream>

namespace library
{
	GLuint Texture::lastid[TEXTURE_UNITS] = {0};
	GLenum Texture::lastUnit = -1;

	Texture::Texture(GLenum target)
		: Texture(target, GL_RGBA8) {}

  Texture::~Texture()
  {
    glDeleteTextures(1, &this->id);
  }

	Texture::Texture(GLenum target, GLint format)
	{
		glGenTextures(1, &this->id);
		this->type   = target;
		this->format = format;
		this->boundUnit = -1;
		this->isMipmapped = false;
	}

	void Texture::create(const Bitmap& bmp, bool mipmap = true, GLint wrapmode = GL_CLAMP_TO_EDGE, GLint magfilter = GL_NEAREST, GLint minfilter = GL_LINEAR_MIPMAP_LINEAR)
	{
		// Implementation is a C library, so no const& :)
		this->isMipmapped = mipmap;
		this->width  = bmp.getWidth();
		this->height = bmp.getHeight();
		const GLuint* pixel = bmp.data();

		bind(0);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_S, wrapmode);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_T, wrapmode);
		glTexParameteri(this->type, GL_TEXTURE_MAG_FILTER, magfilter);
		glTexParameteri(this->type, GL_TEXTURE_MIN_FILTER, minfilter);

		if (this->isMipmapped)
		{
			glTexParameteri(this->type, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(this->type, GL_TEXTURE_MAX_LEVEL, (int)(log(width) / log(2.0)));
		}

		GLenum byteFormat = getByteFormat();

		if (this->type == GL_TEXTURE_1D)
		{
			glTexImage1D(this->type, 0, format, width, 0, bmp.getFormat(), byteFormat, pixel);
		}
		else if (this->type == GL_TEXTURE_2D)
		{
			glTexImage2D(this->type, 0, format, width, height, 0, bmp.getFormat(), byteFormat, pixel);
		}
		else if (this->type == GL_TEXTURE_CUBE_MAP)
		{
			/* ====================================== */

			// create temporary bitmap consisting of 1 side of a cubemap
			int cmsize = width / 4;
			Bitmap blitdump(cmsize, cmsize, 32);

			// iterate the 6 sides of a cubemap
			for (int i = 0; i < 6; i++)
			{
				switch (i)
				{
				// blit each side of the cube to our temporary bitmap at (0, 0)
				case 0: // +x
					bmp.blit(blitdump, cmsize * 2, cmsize * 1, cmsize, cmsize, 0, 0);
					break;
				case 1: // -x
					bmp.blit(blitdump, cmsize * 0, cmsize * 1, cmsize, cmsize, 0, 0);
					break;
				case 2: // +y
					bmp.blit(blitdump, cmsize * 1, cmsize * 0, cmsize, cmsize, 0, 0);
					break;
				case 3: // -y
					bmp.blit(blitdump, cmsize * 1, cmsize * 2, cmsize, cmsize, 0, 0);
					break;
				case 4: // +z
					bmp.blit(blitdump, cmsize * 1, cmsize * 1, cmsize, cmsize, 0, 0);
					break;
				case 5: // -z
					bmp.blit(blitdump, cmsize * 3, cmsize * 1, cmsize, cmsize, 0, 0);
					break;
				}
				// upload each side, from temporary bitmap
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, cmsize, cmsize, 0, bmp.getFormat(), byteFormat, blitdump.data());
			}
			/* ====================================== */
		}
		else if (this->type == GL_TEXTURE_2D_ARRAY)
		{
			int numTiles = bmp.getTilesX() * bmp.getTilesY();

			glTexImage3D(this->type, 0, format, width, height, numTiles, 0, bmp.getFormat(), byteFormat, pixel);
		}
		else
		{
			logger << Log::ERR << "@Texture::create(Bitmap&): Unknown texture target (" << (int)this->type << ")" << Log::ENDL;
			throw std::string("Texture::create(Bitmap&): Unknown texture target (" + std::to_string(this->type) + ")");
		}

		if (this->isMipmapped)
		{
			glGenerateMipmap(this->type);
		}

		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Texture::create(Bitmap&): OpenGL state error" << Log::ENDL;
			logger << Log::ERR << toString() << Log::ENDL;
			throw std::string("Texture::create(Bitmap&): OpenGL state error");
		}
	}

	void Texture::create(int levels, int width, int height)
	{
		this->isMipmapped = (levels > 0);
		this->width  = width;
		this->height = height;

		bind(0);
		const GLint minfilter = (levels) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST;
		const GLint maxfilter = GL_NEAREST;

		glTexParameteri(this->type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->type, GL_TEXTURE_MAG_FILTER, maxfilter);
		glTexParameteri(this->type, GL_TEXTURE_MIN_FILTER, minfilter);

		if (this->isMipmapped)
		{
			glTexParameteri(this->type, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(this->type, GL_TEXTURE_MAX_LEVEL, levels);
		}

		const GLenum sformat = getStorageFormat();

		switch (this->type)
		{
		case GL_TEXTURE_1D:
			glTexImage1D(this->type, 0, format, width, 0, GL_BGRA, sformat, nullptr);
			break;
		case GL_TEXTURE_2D:
			glTexImage2D(this->type, 0, format, width, height, 0, GL_BGRA, sformat, nullptr);
			break;
		default:
			logger << Log::ERR << "Texture::create(): Unknown texture target (" << this->type << ")" << Log::ENDL;
			logger << Log::ERR << toString();
			throw std::string("Failed to create texture");
		}
		if (this->isMipmapped)
			glGenerateMipmap(this->type);

		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Texture::create(): OpenGL state error" << Log::ENDL;
			logger << Log::ERR << toString() << Log::ENDL;
			throw std::string("Texture::create(): OpenGL state error");
		}
	}

	void Texture::create3d(int levels, int x, int y, int z)
	{
		this->isMipmapped = (levels > 0);
		this->width  = x;
		this->height = y;

		bind(0);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		const GLint minfilter = (levels) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST;
		const GLint maxfilter = GL_NEAREST;

		glTexParameteri(this->type, GL_TEXTURE_MAG_FILTER, maxfilter);
		glTexParameteri(this->type, GL_TEXTURE_MIN_FILTER, minfilter);

		if (this->isMipmapped)
		{
			glTexParameteri(this->type, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(this->type, GL_TEXTURE_MAX_LEVEL, levels);
		}

		const GLenum sformat = getStorageFormat();

		switch (this->type)
		{
		case GL_TEXTURE_3D:
			glTexImage3D(this->type, 0, format, x, y, z, 0, GL_BGRA, sformat, nullptr);
			break;
		default:
			logger << Log::ERR << "Texture::create3d(): Unknown texture target (" << this->type << ")" << Log::ENDL;
			logger << Log::ERR << toString();
			throw std::string("Failed to create 3d texture");
		}
		if (this->isMipmapped)
			glGenerateMipmap(this->type);

		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Texture::create(): OpenGL state error" << Log::ENDL;
			logger << Log::ERR << toString() << Log::ENDL;
			throw std::string("Texture::create(): OpenGL state error");
		}
	}

	void Texture::setInterpolation(bool linear)
	{
		GLint maxfilter = (linear) ? GL_LINEAR : GL_NEAREST;
		GLint minfilter = maxfilter;
		if (isMipmapped) minfilter = (linear) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;

		glTexParameteri(this->type, GL_TEXTURE_MAG_FILTER, maxfilter);
		glTexParameteri(this->type, GL_TEXTURE_MIN_FILTER, minfilter);

		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Texture::setInterpolation(): OpenGL state error" << Log::ENDL;
			logger << Log::ERR << toString() << Log::ENDL;
			throw std::string("Texture::setInterpolation(): OpenGL state error");
		}
	}
	void Texture::setWrapMode(GLint wrapmode)
	{
		glTexParameteri(this->type, GL_TEXTURE_WRAP_S, wrapmode);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_T, wrapmode);
	}
	void Texture::setAnisotropy(float samples)
	{
		glTexParameterf(this->type, GL_TEXTURE_MAX_ANISOTROPY_EXT, samples);
	}

	void Texture::createDepth(int width, int height)
	{
		// create empty depth buffer texture
		this->width  = width;
		this->height = height;

		bind(0);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(this->type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(this->type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		if (format == GL_DEPTH24_STENCIL8 ||
			format == GL_DEPTH32F_STENCIL8)
		{
			// let opengl give the highest supported for this context
			glTexImage2D(this->type, 0, format, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		}
		else
		{
			glTexImage2D(this->type, 0, format, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Texture::createDepth(): OpenGL state error" << Log::ENDL;
			logger << Log::ERR << toString() << Log::ENDL;
			throw std::string("Texture::createDepth(): OpenGL state error");
		}
	}

	void Texture::createMultisample(int width, int height, int samples)
	{
		this->width  = width;
		this->height = height;
		this->isMipmapped = false;

		bind(0);
		glTexImage2DMultisample(this->type, samples, this->format, width, height, true);

		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Texture::createMultisample(): OpenGL state error" << Log::ENDL;
			logger << Log::ERR << toString() << Log::ENDL;
			throw std::string("Texture::createMultisample(): OpenGL state error");
		}
	}
	void Texture::createDepthMultisampled(int width, int height, int samples)
	{
		// create empty multisampled depth buffer texture
		this->width  = width;
		this->height = height;
		this->isMipmapped = false;

		bind(0);
		glTexImage2DMultisample(this->type, samples, this->format, width, height, true);

		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Texture::createDepthMultisampled(): OpenGL state error" << Log::ENDL;
			logger << Log::ERR << toString() << Log::ENDL;
			throw std::string("Texture::createDepthMultisampled(): OpenGL state error");
		}
	}

  void Texture::raw_bind(GLenum unit, GLenum type, GLuint tex_id)
  {
    // avoid binding same texture twice on the same texture unit
		if (lastid[unit] == tex_id) return;
		lastid[unit] = tex_id;

		if (lastUnit != unit)
		{
			lastUnit = unit;
			glActiveTexture(GL_TEXTURE0 + unit);
		}

		glBindTexture(type, tex_id);

#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Texture::bind(): OpenGL state error" << Log::ENDL;
			logger << Log::ERR << toString() << Log::ENDL;
			throw std::string("Texture::bind(): OpenGL state error");
		}
#endif
  }
	void Texture::bind(GLenum unit)
	{
    raw_bind(unit, this->type, this->id);
    this->boundUnit = unit;
	}

	void Texture::unbind(GLenum unit) noexcept
	{
		if (lastUnit != unit)
		{
			lastUnit = unit;
			glActiveTexture(GL_TEXTURE0 + unit);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		lastid[unit] = 0;
	}

	void Texture::copyScreen()
	{
		copyScreen(this->width, this->height);
	}
	void Texture::copyScreen(int w, int h)
	{
		// explicitly select textures last texture unit
		if (lastUnit != getBoundUnit())
		{
			glActiveTexture(GL_TEXTURE0 + getBoundUnit());
			lastUnit = getBoundUnit();
		}
		// copy screen
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);

		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Texture::copyScreen(): OpenGL state error" << Log::ENDL;
			logger << Log::ERR << toString() << Log::ENDL;
			throw std::string("Texture::copyScreen(): OpenGL state error");
		}
		#endif
	}

	void Texture::upload(const Bitmap& bmp)
	{
		this->width  = bmp.getWidth();
		this->height = bmp.getHeight();

		// upload pixel data
		if (this->type == GL_TEXTURE_1D)
		{
			glTexImage1D(this->type, 0, format, width, 0, bmp.getFormat(), getByteFormat(), bmp.data());
		}
		else if (this->type == GL_TEXTURE_2D)
		{
			glTexImage2D(this->type, 0, format, width, height, 0, bmp.getFormat(), getByteFormat(), bmp.data());
		}

		// auto-generate new mipmap levels
		if (this->isMipmapped)
		{
			glGenerateMipmap(this->type);
		}

		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Texture::uploadBGRA8(): OpenGL state error" << Log::ENDL;
			logger << Log::ERR << toString() << Log::ENDL;
			throw std::string("Texture::uploadBGRA8(): OpenGL state error");
		}
		#endif
	}

	void Texture::upload3D(int x, int y, int z, void* data)
	{
		// upload pixel data
		glTexImage3D(GL_TEXTURE_3D, 0, format, x, y, z, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

		// auto-generate new mipmap levels
		if (this->isMipmapped)
		{
			glGenerateMipmap(this->type);
		}

		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Texture::upload3D(): OpenGL state error" << Log::ENDL;
			logger << Log::ERR << toString() << Log::ENDL;
			throw std::string("Texture::upload3D(): OpenGL state error");
		}
		#endif
	}

	GLenum Texture::getStorageFormat()
	{
		if (format == GL_RGBA16F || format == GL_R16F || format == GL_RGBA32F || format == GL_R32F)
			return GL_FLOAT;
		else if (format == GL_RGBA32UI || format == GL_RGBA32I)
			return GL_RGBA_INTEGER;
		else if (format == GL_RGB32UI || format == GL_RGB32I)
			return GL_RGBA_INTEGER;
		else if (format == GL_RG32UI || format == GL_RG32I)
			return GL_RGBA_INTEGER;
		else if (format == GL_R32UI || format == GL_R32I)
			return GL_RGBA_INTEGER;
		else if (format == GL_RGBA16UI || format == GL_RGB16UI || format == GL_RG16UI || format == GL_R16UI)
			return GL_RG_INTEGER;
		else if (format == GL_RGBA16I || format == GL_RGB16I || format == GL_RG16I || format == GL_R16I)
			return GL_INT;
		else
			return GL_UNSIGNED_BYTE;
	}
	GLenum Texture::getByteFormat()
	{
		if (format == GL_RGBA16F || format == GL_R16F || format == GL_RGBA32F || format == GL_R32F)
			return GL_FLOAT;
		else if (format == GL_RGBA32UI || format == GL_RGB32UI || format == GL_RG32UI || format == GL_R32UI)
			return GL_UNSIGNED_INT;
		else if (format == GL_RGBA32I || format == GL_RGB32I || format == GL_RG32I || format == GL_R32I)
			return GL_INT;
		else if (format == GL_RGBA16UI || format == GL_RGB16UI || format == GL_RG16UI || format == GL_R16UI)
			return GL_UNSIGNED_SHORT;
		else if (format == GL_RGBA16I || format == GL_RGB16I || format == GL_RG16I || format == GL_R16I)
			return GL_SHORT;
		else
			return GL_UNSIGNED_BYTE;
	}

	std::string Texture::toString() const
	{
		std::string typeString;
		std::string formatString;

		switch (type)
		{
		case GL_TEXTURE_1D:
			typeString = "GL_TEXTURE_1D"; break;
		case GL_TEXTURE_2D:
			typeString = "GL_TEXTURE_2D"; break;
		case GL_TEXTURE_2D_MULTISAMPLE:
			typeString = "GL_TEXTURE_2D_MULTISAMPLE"; break;
		case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
			typeString = "GL_TEXTURE_2D_MULTISAMPLE_ARRAY"; break;
		case GL_TEXTURE_2D_ARRAY:
			typeString = "GL_TEXTURE_2D_ARRAY"; break;
		case GL_TEXTURE_3D:
			typeString = "GL_TEXTURE_3D"; break;
		case GL_TEXTURE_CUBE_MAP:
			typeString = "GL_TEXTURE_CUBE_MAP"; break;
		default:
			typeString = "(Unknown type)"; break;
		}

		switch (format)
		{
		case GL_RGBA:
			formatString = "GL_RGBA"; break;
		case GL_RGBA16F_ARB:
			formatString = "GL_RGBA16F"; break;
		case GL_RGBA32F_ARB:
			formatString = "GL_RGBA32F"; break;
		case GL_COMPRESSED_RGBA:
			formatString = "GL_COMPRESSED_RGBA"; break;
		case GL_DEPTH_COMPONENT:
			formatString = "GL_DEPTH_COMPONENT"; break;
		case GL_DEPTH_COMPONENT16:
			formatString = "GL_DEPTH_COMPONENT16"; break;
		case GL_DEPTH_COMPONENT24:
			formatString = "GL_DEPTH_COMPONENT24"; break;
		case GL_DEPTH_COMPONENT32:
			formatString = "GL_DEPTH_COMPONENT32"; break;
		case GL_DEPTH24_STENCIL8:
			formatString = "GL_DEPTH24_STENCIL8"; break;
		default:
			formatString = "(Unknown format)"; break;
		}

		std::stringstream ss;
		ss << "Texture ID: " << id << " Size: (" << width << ", " << height << ") Type: " + typeString + " Format: " + formatString;
		return ss.str();
	}

}
