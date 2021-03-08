#include <library/noise/cosnoise.hpp>
#include <cmath>

namespace library
{
	static const double PI = 4 * atan(1);
/*
	static inline double cosntri(double *p, double x, double y, double z)
	{
		return( p[0] * (1-x) * (1-y) * (1-z) +  // 0,0,0
				p[1] *   x   * (1-y) * (1-z) +  // 1,0,0
				p[2] * (1-x) *   y   * (1-z) +  // 0,1,0
				p[3] * (1-x) * (1-y) *   z   +  // 0,0,1
				p[4] *   x   * (1-y) *   z   +  // 1,0,1
				p[5] * (1-x) *   y   *   z   +  // 0,1,1
				p[6] *   x   *   y   * (1-z) +  // 1,1,0
				p[7] *   x   *   y   *   z   ); // 1,1,1
	}
*/
	static inline double cosntri_const1(double *p, double y)
	{
		return  ( ( p[0] * (1-y)  +  // 0,0,0
					p[1] * (1-y)  +  // 1,0,0
					p[2] *   y    +  // 0,1,0
					p[3] * (1-y)  +  // 0,0,1
					p[4] * (1-y)  +  // 1,0,1
					p[5] *   y    +  // 0,1,1
					p[6] *   y    +  // 1,1,0
					p[7] *   y   )   // 1,1,1
					* 0.25  );
	}

	static inline double cosntri_const2(double *p)
	{
		return  ( p[0] + p[1] + p[2] + p[3] + p[4] + p[5] + p[6] + p[7] ) * 0.125;
	}

	// curve creates roundness with 1.0, and sharp spires with higher values
	// curve is undefined < 1.0
	// cuts -- -1.0 to 1.0, and is otherwise undefined
	// fatness increases or decreases bulginess

	double cosnoise(glm::vec3& v, double density, double scale_xz, double scale_y,
						          double curve,   double fatness,  double cuts)
	{
		#define size_x  scale_xz
		#define size_z  scale_xz
		#define size_y  scale_y

		// cheap reusage of slope
		double slope = v.y * size_y + density;

		// 2d shearing:
		double x = v.x * size_x + slope;
		double z = v.z * size_z + slope;

		// r * r for sloping
		#define cosip(fx) (0.5 + cos((fx) * PI) * 0.5)

		// real slope
		slope = curve + density * density * fatness;
		// weights
		double weights[8] __attribute__((aligned(32)));
		size_x += size_x; // size_x == size_z

		// optimized formula
		weights[0] = std::pow( cosip(x - size_x - size_y), slope);
		weights[1] = std::pow( cosip(x - size_y)         , slope);
		weights[2] = std::pow( cosip(x - size_x + size_y), slope);
		weights[3] = std::pow( cosip(z - size_y)         , slope);
		weights[4] = std::pow( cosip(z + size_x - size_y), slope);
		weights[5] = std::pow( cosip(z + size_y)         , slope);
		weights[6] = std::pow( cosip(x + size_y)         , slope);
		weights[7] = std::pow( cosip(z + size_x + size_y), slope);

		// simplified solution for centered y
		if (cuts == 0)
			return cosntri_const2(&weights[0]) * 2.0 - 1.0;

		x = 0.5 + cuts * 0.5; // y-value
		return cosntri_const1(&weights[0], x) * 2.0 - 1.0;

	}

}
