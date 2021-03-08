#include <library/script/script.hpp>

#include <libtcc.h>

#define TCC_ERROR -1

namespace library
{
	Script::Script()
	{
		this->state = tcc_new();
		
		if (this->state == nullptr)
		{
			throw std::string("Failed to initialize libtcc");
		}
	}
	
	Script::~Script()
	{
		if (this->state)
			tcc_delete(this->state);
			
		if (this->memory)
			free(this->memory);
	}
	
	void Script::setErrorFunc(error_func errfunc)
	{
		tcc_set_error_func(this->state, nullptr, errfunc);
	}
	
	void Script::addsymb(const std::string& symb, const void* func)
	{
		tcc_add_symbol(this->state, symb.c_str(), func);
	}
	
	void Script::compile(const std::string& program)
	{
		// memory output
		tcc_set_output_type(this->state, TCC_OUTPUT_MEMORY);
		
		// compile
		if (tcc_compile_string(this->state, program.c_str()) == TCC_ERROR)
		{
			throw std::string("Compile error");
		}
		
		// allocate
		this->memory = malloc(tcc_relocate(this->state, nullptr));
		
		// relocate to executable memory
		if (tcc_relocate(this->state, this->memory) == TCC_ERROR)
		{
			throw std::string("Link error");
		}
	}
	
	// all execute() variants returns -1 upon failure of execution (for now)
	
	int Script::execute(const std::string& function)
	{
		typedef int (*intfunc) (void);
		intfunc func = (intfunc) tcc_get_symbol(this->state, function.c_str());
		
		if (func)
			return func();
		else
			return -1;
	}
	
	int Script::execute(const std::string& function, void* data)
	{
		typedef int (*datafunc) (void* data);
		datafunc func = (datafunc) tcc_get_symbol(this->state, function.c_str());
		
		if (func)
			return func(data);
		else
			return -1;
	}
	
}
