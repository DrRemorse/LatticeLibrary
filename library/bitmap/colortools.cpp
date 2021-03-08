#include <library/bitmap/colortools.hpp>

using namespace glm;

namespace library
{
	Color::Color(int red, int grn, int blu)
	{
		this->r = red & 255; this->g = grn & 255; this->b = blu & 255;
	}
	Color::Color(int r, int g, int b, int a)
	{
		this->r = r & 255; this->g = g & 255;
		this->b = b & 255; this->a = a & 255;
	}
	
	uint32_t Color::toBGRA()
	{
		return r + (g << 8) + (b << 16) + (a << 24);
	}
	
	Color Color::mixColor(Color& a, Color& b, float mixlevel)
	{
		return Color(
			(int)( (float)a.r * (1.0 - mixlevel) + (float)b.r * mixlevel ),
			(int)( (float)a.g * (1.0 - mixlevel) + (float)b.g * mixlevel ),
			(int)( (float)a.b * (1.0 - mixlevel) + (float)b.b * mixlevel ),
			(int)( (float)a.a * (1.0 - mixlevel) + (float)b.a * mixlevel )
		);
	}
	
	void Color::addColorv(Color& a, Color& b, float level)
	{
		a.r += (int)( (float)b.r * level ) & 255;
		a.g += (int)( (float)b.g * level ) & 255;
		a.b += (int)( (float)b.b * level ) & 255;
	}
	
	Color Color::getGradientColor(float v, Color* array, int size)
	{
		int    vint = (int)v, vnxt;
		float  vfrac = v - vint;
		
		// find color gradient values
		if (vfrac < 0.5)
			vnxt = vint - 1;
		else
			vnxt = vint + 1;
		
		// clamp
		if (vnxt < 0) vnxt = 0;
		if (vnxt >= size) vnxt = size-1;
		
		// get gradient array colors
		Color& cl1 = array[vint];
		Color& cl2 = array[vnxt];
		
		// convert fractional to interpolator
		if (vfrac < 0.5) vfrac = (0.5 - vfrac) / 0.5;
		else vfrac = (vfrac - 0.5) / 0.5;
		// S-curve the interpolator
		vfrac = quintic(vfrac);
		
		return mixColor( cl1, cl2, vfrac * 0.5 );
	}
	
	glm::vec4 colorToVector(rgba8_t color)
	{
		return glm::vec4
		(
			 (color & 255) / 255.0f,
			((color >>  8) & 255) / 255.0f,
			((color >> 16) & 255) / 255.0f,
			 (color >> 24) / 255.0f
		);
	}
	
	rgba8_t vectorToColor(const glm::vec4& vector)
	{
		int r = int(vector.x * 255);
		int g = int(vector.y * 255) << 8;
		int b = int(vector.y * 255) << 16;
		int a = int(vector.y * 255) << 24;
		return BGRA8(r, g, b, a);
	}
	
}
