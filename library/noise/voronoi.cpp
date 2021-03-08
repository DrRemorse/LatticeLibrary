#include <library/noise/voronoi.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/noise.hpp>
#include <stdint.h>
#include <cmath>

namespace library
{
	Voronoi::vor_t Voronoi::vor_euclidian(vor_t x1, vor_t y1, vor_t x2, vor_t y2)
	{
		vor_t dx = x2-x1, dy = y2-y1;
		return sqrtf(dx*dx + dy*dy);
	}
	
	Voronoi::vor_t Voronoi::vor_quadratic(vor_t x1, vor_t y1, vor_t x2, vor_t y2)
	{
		x1 -= x2; y1 -= y2;
		return x1*x1 + x1*y1 + y1*y1;
	}
	
	Voronoi::vor_t Voronoi::vor_linsquare(vor_t x1, vor_t y1, vor_t x2, vor_t y2)
	{
		x1 -= x2; y1 -= y2;
		return x1*x1 + y1*y1;
	}
	
	Voronoi::vor_t Voronoi::vor_minkowski(vor_t x1, vor_t y1, vor_t x2, vor_t y2)
	{
		const vor_t MINKOWSKI = 3.5;
		
		x1 -= x2; y1 -= y2;
		return std::pow(std::pow(fabs(x1), MINKOWSKI) + std::pow(fabs(y1), MINKOWSKI), 1.0 / MINKOWSKI);
	}
	
	Voronoi::vor_t Voronoi::vor_manhattan(vor_t x1, vor_t y1, vor_t x2, vor_t y2)
	{
		x1 -= x2; y1 -= y2;
		return fabs(x1) + fabs(y1);
	}
	
	Voronoi::vor_t Voronoi::vor_chebyshev(vor_t x1, vor_t y1, vor_t x2, vor_t y2)
	{
		x1 = fabs(x1 - x2);
		y1 = fabs(y1 - y2);
		
		if (x1 > y1) return x1;
		return y1;
	}
	
	inline unsigned int hash(unsigned int x)
	{
		x = (x ^ 61) ^ (x >> 16);
		x = x + (x << 3);
		x = x ^ (x >> 4);
		x = x * 0x27d4eb2d;
		x = x ^ (x >> 15);
		return x;
	}
	inline int ihash(int x)
	{
		return hash(x) & INT32_MAX;
	}

	inline float randf(int x)
	{
		return ihash(x) / (float)INT32_MAX;
	}
	inline float randf(int x, int y)
	{
		return randf(x ^ ihash(y));
	}
	
	Voronoi::vor_t Voronoi::getdist(vor_t px, vor_t py, distance_func distFunc)
	{
		vor_t mindist = 99;
		int x = floorf(px);
		int y = floorf(py);
		
		for (int dx = x-1; dx <= x+1; dx++)
		for (int dy = y-1; dy <= y+1; dy++)
		{
			// create a voronoi point
			vor_t getx = dx + randf(px, py);
			vor_t gety = dy + randf(px + 431, py + 293);
			
			vor_t dist = distFunc( px, py, getx, gety );
			if (dist < mindist) mindist = dist;  // mew closest value
		}
		return mindist;
	}
	
	int Voronoi::getid(vor_t px, vor_t py, distance_func distFunc)
	{
		vor_t mindist = 99;
		int x  = floorf(px);
		int y  = floorf(py);
		int id = 0;
		
		for (int dx = x-1; dx <= x+1; dx++)
		for (int dy = y-1; dy <= y+1; dy++)
		{
			// create a voronoi point
			vor_t getx = dx + randf(dx + 3299, dy + 5843);
			vor_t gety = dy + randf(dx + 431,  dy + 2437);
			
			vor_t dist = distFunc( px, py, getx, gety );
			
			if (dist < mindist)
			{
				// assign some integral id based on hash
				id      = ihash(dx + 5953) ^ ihash(dy + 7639);
				// save distance
				mindist = dist;  // mew closest value
			}
		}
		return id;
	}
	
}

