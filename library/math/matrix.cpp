#include <library/math/matrix.hpp>

#include <cmath>
using namespace glm;

namespace library
{
	mat4 biasMatrix()
	{
		glm::mat4 mat;
		float* m = (float*) &mat;
		m[0] = 0.5; m[4] = 0.0; m[8 ] = 0.0; m[12] = 0.5;
		m[1] = 0.0; m[5] = 0.5; m[9 ] = 0.0; m[13] = 0.5;
		m[2] = 0.0; m[6] = 0.0; m[10] = 0.5; m[14] = 0.5;
		m[3] = 0.0; m[7] = 0.0; m[11] = 0.0; m[15] = 1.0;
		return mat;
	}

	mat4 rotationMatrix(float ax, float ay, float az)
	{
		//////////////////////////////////////////////////////////////////////////////
		// convert Euler angles(x,y,z) to axes(left, up, forward)					//
		// Each column of the rotation matrix represents left, up and forward axis. //
		// The order of rotation is Roll->Yaw->Pitch (Rx*Ry*Rz)
		// Rx: rotation about X-axis, pitch
		// Ry: rotation about Y-axis, yaw(heading)
		// Rz: rotation about Z-axis, roll
		//    Rx           Ry          Rz
		// |1  0   0| | Cy  0 Sy| |Cz -Sz 0|   | CyCz        -CySz         Sy  |	//
		// |0 Cx -Sx|*|  0  1  0|*|Sz  Cz 0| = | SxSyCz+CxSz -SxSySz+CxCz -SxCy|	//
		// |0 Sx  Cx| |-Sy  0 Cy| | 0   0 1|   |-CxSyCz+SxSz  CxSySz+SxCz  CxCy|	//
		//////////////////////////////////////////////////////////////////////////////
		
		// rotation angle about X-axis (pitch)
		float sx = sin(ax);
		float cx = cos(ax);
		
		// rotation angle about Y-axis (yaw)
		float sy = sin(ay);
		float cy = cos(ay);
		
		glm::mat4 mat;
		float* m = (float*) &mat;
		
		if (az != 0.0)
		{
			// rotation angle about Z-axis (roll)
			float sz = sin(az);
			float cz = cos(az);
			
			// left vector
			m[0] =  cy * cz;
			m[1] =  sx * sy * cz + cx * sz;
			m[2] = -cx * sy * cz + sx * sz;
			m[3] = 0.0; // w1
			
			// up vector
			m[4] = -cy * sz;
			m[5] = -sx * sy * sz + cx * cz;
			m[6] =  cx * sy * sz + sx * cz;
			m[7] = 0.0; // w2
		}
		else
		{
			// sz = 0.0 (sin(0) = 0)
			// cz = 1.0 (cos(0) = 1)
			
			// left vector
			m[0] =  cy;
			m[1] =  sx * sy;
			m[2] = -cx * sy;
			m[3] = 0.0; // w1
			
			// up vector
			m[4] = 0.0;
			m[5] = cx;
			m[6] = sx;
			m[7] = 0.0; // w2
		}
		
		// forward vector
		m[ 8] =  sy;
		m[ 9] = -sx * cy;
		m[10] =  cx * cy;
		m[11] = 0.0; // w3
		
		// translation (tx, ty, tz, w)
		m[12] = 0.0; // tx
		m[13] = 0.0; // ty
		m[14] = 0.0; // tz
		m[15] = 1.0; // w4
		
		return mat;
	}
	
	// orthographic projection matrix (center top-left)
	mat4 ortho2dMatrix(float width, float height, float znear, float zfar)
	{
		float depth = zfar - znear;
		/*
			m[0] = 2/width;  m[4] = 0;         m[ 8] = 0;         m[12] = -1.0;
			m[1] = 0;        m[5] = 2/height;  m[ 9] = 0;         m[13] = -1.0;
			m[2] = 0;        m[6] = 0;         m[10] = -2/depth;  m[14] = -(zfar+znear) / depth;
			m[3] = 0;        m[7] = 0;         m[11] = 0;         m[15] =  1.0;
		*/
		glm::mat4 mat =
		{	2/width, 0, 0, 0,
			0, -2/height, 0, 0,
			0, 0, -2/depth, 0,
			-1.0, 1.0, -(zfar+znear) / depth, 1.0
		};
		return mat;
	}

	// orthographic projection matrix (center screen)
	mat4 orthoMatrix(float width, float height, float znear, float zfar)
	{
		float ndcw =  width;  // 
		float ndch = -height; // inverting Y-axis
		float depth = zfar - znear;
		/*
			m[0] = 2/ndcw;  m[4] = 0;       m[ 8] = 0;         m[12] = -1.0;
			m[1] = 0;       m[5] = 2/ndch;  m[ 9] = 0;         m[13] =  1.0;
			m[2] = 0;       m[6] = 0;       m[10] = -2/depth;  m[14] = -(zfar+znear) / depth;
			m[3] = 0;       m[7] = 0;       m[11] = 0;         m[15] =  1.0;
		*/
		glm::mat4 mat =
		{
			2/ndcw, 0, 0, 0,
			0, 2/ndch, 0, 0,
			0, 0, -2/depth, 0,
			-1.0, 1.0, -(zfar+znear) / depth, 1.0
		};
		return mat;
	}

	// FOV perspective matrix (frustum)
	glm::mat4 perspectiveMatrix(float fov, float aspect, float znear, float zfar)
	{
		const double pio180 = 4.0 * atan(1.0) / 180.0;
		
		float h = 1.0 / tan(fov * pio180 / 2.0);
		float negd = znear - zfar;
		glm::mat4 mat;
		float* m = (float*) &mat;
		
		m[ 0] = h / aspect; m[ 1] = 0; m[ 2] = 0;                       m[ 3] =  0;
		m[ 4] = 0;          m[ 5] = h; m[ 6] = 0;                       m[ 7] =  0;
		m[ 8] = 0;          m[ 9] = 0; m[10] = (zfar + znear) / negd;   m[11] = -1;
		m[12] = 0;          m[13] = 0; m[14] = 2 * znear * zfar / negd; m[15] =  0;
		
		return mat;
	}
}
