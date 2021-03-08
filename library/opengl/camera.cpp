#include <library/opengl/camera.hpp>

#include <library/log.hpp>
#include <library/opengl/window.hpp>
#include <cmath>
#include <glm/gtx/transform.hpp>

namespace library
{
	Camera::Camera(float FOV, float aspect, float znear, float zfar)
	{
		setProjection(FOV, aspect, znear, zfar);
		
		// initialize matrices
		setRotation(0, 0, 0);
		setTranslation(0, 0, 0);
	}
	
	void Camera::setProjection(float FOV, float aspect, float znear, float zfar)
	{
		if (znear > zfar) throw std::string("Camera::Camera(): Znear must be less than Zfar");
		if (znear <= 0.0) throw std::string("Camera::Camera(): Znear must be greater than zero");
		
		// unit blocks depth-range
		this->FOV   = FOV;
		this->znear = znear;
		this->zfar  = zfar;
		
		// perspective projection matrix
		matproj = perspectiveMatrix(this->FOV, aspect, this->znear, this->zfar);
		
		// calculate half near-plane size
		const double pio180 = 4.0 * atan(1.0) / 180.0;
		
		float halfTan = tan(this->FOV * pio180 / 2.0);
		nearPlaneHalfSize = glm::vec2(halfTan * aspect, halfTan);
	}
	
	void Camera::calculateFrustum()
	{
		frustum.calculate(matviewproj);
	}
	const Frustum& Camera::getFrustum() const
	{
		return this->frustum;
	}
	
	void Camera::setRotation(float rx, float ry, float rz)
	{
		matrot = rotationMatrix(rx, ry, rz);
	}
	
	void Camera::setTranslation(float tx, float ty, float tz)
	{
		// view matrix = rotation & translation
		matview = matrot;
		matview *= glm::translate(glm::vec3(tx, ty, tz));
		
		// modelview projection matrix
		matviewproj = matproj * matview;
	}
	
	void Camera::setRotationMatrix(const glm::mat4& rot)
	{
		matrot = rot;
	}
	void Camera::setViewMatrix(const glm::mat4& view)
	{
		matview = view;
		matviewproj = matproj * matview;
	}
	
}
