#include <library/opengl/shader.hpp>

#include <library/log.hpp>
#include <library/opengl/opengl.hpp>
#include <GL/glext.h>
#include <stdexcept>
#include <fstream>

namespace library
{
	// static members
	GLuint Shader::lastShader = 0;

	std::string trim(std::string text)
	{
		// trim trailing spaces
		size_t endpos = text.find_last_not_of(" \t");
		if (std::string::npos != endpos)
			text = text.substr(0, endpos + 1);

		// trim leading spaces
		size_t startpos = text.find_first_not_of(" \t");
		if (std::string::npos != startpos)
			text = text.substr(startpos);

		return text;
	}

	// as written by user763305
	// http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
	std::istream& safeGetline(std::istream& is, std::string& t)
	{
    	t.clear();

	    // The characters in the stream are read one-by-one using a std::streambuf.
	    // That is faster than reading them one-by-one using the std::istream.
	    // Code that uses streambuf this way must be guarded by a sentry object.
	    // The sentry object performs various tasks,
	    // such as thread synchronization and updating the stream state.

	    std::istream::sentry se(is, true);
	    std::streambuf* sb = is.rdbuf();

	    while(true)
		{
    	    int c = sb->sbumpc();
	        switch (c) {
		        case '\n':
		            return is;
        		case '\r':
		            if(sb->sgetc() == '\n')
		                sb->sbumpc();
		            return is;
		        case EOF:
		            // Also handle the case when the last line has no line ending
		            if(t.empty())
		                is.setstate(std::ios::eofbit);
		            return is;
		        default:
		            t += (char)c;
	        }
	    }
	} // safeGetline

	void Shader::printShaderStatus(GLuint shader, bool prog)
	{
		logger << Log::ERR << "*** Shader program compile error:" << Log::ENDL;
		GLint logsize;
		GLchar* infolog;
		if (prog)
		{
			glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &logsize);
			infolog = new char[logsize+1];
			glGetProgramInfoLog(shader, logsize, NULL, infolog);
		}
		else
		{
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
			infolog = new char[logsize+1];
			glGetShaderInfoLog(shader, logsize, NULL, infolog);
		}
		logger << Log::INFO << std::string(infolog) << Log::ENDL;
		delete[] infolog;
	}

	std::string Shader::shaderProcessor(const std::string& filename, Shader::processFunc tokenizer, bool isVertex)
	{
		std::ifstream file(filename.c_str());

		if (!file)
		{
			throw std::runtime_error("Shader::shaderProcessor(): Could not open file " + filename);
		}

		std::string shaderText = "";

		// read file line by line
		const std::string DEF_VERT_PROG = "#define VERTEX_PROGRAM";
		const std::string DEF_FRAG_PROG = "#define FRAGMENT_PROGRAM";
		std::string line;

		while (safeGetline(file, line))
		{
			// strip spaces & tabs
			line = trim(line);

			if (line.find("#include ") == 0)
			{
				// preprocessor statement
				// remove #include "
				line = line.substr(10, line.length());
				// get substring between first " and next "
				line = line.substr(0, line.find(34));

				// reconstruct path
				std::string path = ".";
				size_t findPath = filename.find_last_of("/");

				if (findPath != filename.length())
					line = filename.substr(0, findPath) + "/" + line;

				// process included file
				shaderText += shaderProcessor(line, tokenizer, isVertex);
			}
			// preserve line numbers by commenting out when compiling vertex/fragment program
			else if (line.compare(DEF_VERT_PROG) == 0)
			{
				if (isVertex == true) shaderText += line + "\n";
			}
			else if (line.compare(DEF_FRAG_PROG) == 0)
			{
				if (isVertex == false) shaderText += line + "\n";
			}
			else
			{
				// let user-provided tokenizer function process text further
				if (tokenizer) shaderText += tokenizer(line) + "\n";
				// unless it's not provided, then just add line
				else shaderText += line + "\n";
			}
		}
		file.close();

		return shaderText;
	}

	Shader::Shader(const std::string& filename, const std::vector<std::string>& linkstage) :
		Shader(filename, nullptr, linkstage) { }

	// shader from external file
	Shader::Shader(const std::string& filename, processFunc tokenizer, 
                 const std::vector<std::string>& attributes)
	{
		// recursively process text from files and #includes
		std::string vertshader = shaderProcessor(filename, tokenizer, true );
		std::string fragshader = shaderProcessor(filename, tokenizer, false);

		// use the separated vertex and fragment code to create the shader
		createShader(vertshader, fragshader, filename, attributes);
	}

	// shader from source code
	Shader::Shader(const std::string& vertex, const std::string& frag,
                 const std::string& title, const std::vector<std::string>& attributes)
	{
		createShader(vertex, frag, title, attributes);
	}

	// internal function for uploading shader code, creating and compiling the shader program
	void Shader::createShader(
		const std::string& vertshader,
		const std::string& fragshader,
		const std::string& source,
		const std::vector<std::string>& attributes)
	{
		// char arrays for GL call
		const GLchar* source_v[1] = { (GLchar*) vertshader.c_str() };
		const GLchar* source_f[1] = { (GLchar*) fragshader.c_str() };

		// create shaders
		GLuint shader_v = glCreateShader(GL_VERTEX_SHADER_ARB);
		GLuint shader_f = glCreateShader(GL_FRAGMENT_SHADER_ARB);

		// upload source
		glShaderSource(shader_v, 1, source_v, NULL);
		glShaderSource(shader_f, 1, source_f, NULL);

		// compile shaders
		glCompileShader(shader_v);
		glCompileShader(shader_f);

		GLint status;
		// vertex status
		glGetShaderiv(shader_v, GL_COMPILE_STATUS, &status);
		if (!status)
		{
			printShaderStatus(shader_v, false);
			logger << Log::ERR << "*** Shader source: " << source << Log::ENDL;
			throw std::runtime_error("Shader::load(): Shader compilation error");
		}
		// fragment status
		glGetShaderiv(shader_f, GL_COMPILE_STATUS, &status);
		if (!status)
		{
			printShaderStatus(shader_f, false);
			logger << Log::ERR << "*** Shader source: " << source << Log::ENDL;
			throw std::runtime_error("Shader::load(): Shader compilation error");
		}

		// create shader program
		this->shader = glCreateProgram();
		glAttachShader(this->shader, shader_v);
		glAttachShader(this->shader, shader_f);

		// common attributes
		for (size_t i = 0; i < attributes.size(); i++)
		{
			glBindAttribLocation(this->shader, i, (GLchar*) attributes[i].c_str());
		}

		// link program
		glLinkProgram(this->shader);
		glGetProgramiv(this->shader, GL_LINK_STATUS, &status);
		if (!status)
		{
			logger << Log::INFO << "Offending source: " << source << Log::ENDL;
			printShaderStatus(this->shader, true);
			throw std::runtime_error("Shader::load() failed to link shader " + source);
		}

		// check for errors
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Shader::load(): OpenGL error for: " << source << Log::ENDL;
			throw std::runtime_error("Shader::load(): OpenGL error for: " + source);
		}

		// use program
		bind();
	}

	void Shader::bind()
	{	// avoid costly rebind
		if (lastShader == shader) return;
		lastShader = shader;

		glUseProgram(shader);

		#ifdef DEBUG
		if (OpenGL::checkError())
		{
			logger << Log::ERR << "Shader::bind(): OpenGL error for shader ID = " << shader << Log::ENDL;
			throw std::runtime_error("Shader::bind() OpenGL state error");
		}
		#endif
	}

}
