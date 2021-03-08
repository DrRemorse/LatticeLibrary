#include <library/math/baseconv.hpp>

namespace library
{
	// base32 lookup table
	static const std::string convTable32 = "abcdefghijklmnopqrstuvwxyz012345";
	// base64 lookup table
	static const std::string convTable64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789[]";
	
	std::string BaseConv::base32(unsigned int coordinate, int length)
	{
		const unsigned int B32_ANDMASK = 0x1F;
		
		//char coord[length];
		char* coord;
		coord = new char[length];

		for (int i = 0; i < length; i++)
		{
			coord[length-1 - i] = 
				convTable32[ (coordinate >> (i * 5)) & B32_ANDMASK ];
		}
		delete[] coord;

		return std::string(coord, length);
	}
	
	std::string BaseConv::base64(unsigned int coordinate, int length)
	{
		const unsigned int B64_ANDMASK = 0x3F;
		
		//char coord[length];
		char* coord;
		coord = new char[length];

		for (int i = 0; i < length; i++)
		{
			coord[length-1 - i] = 
				convTable64[ (coordinate >> (i * 6)) & B64_ANDMASK ];
		}
		delete[] coord;

		return std::string(coord, length);
	}
	
}
