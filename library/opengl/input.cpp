#include <library/opengl/input.hpp>

#include <library/log.hpp>
#include <library/math/toolbox.hpp>
#include <library/opengl/opengl.hpp>
#include <cmath>
#include <cstdio>
#include <set>

namespace library
{
  static Input* currentInput = nullptr;

	void Input::init(GLFWwindow* window, const bool kbd, const bool mouse)
	{
		currentInput = this;
		// set owning window
		this->m_window = window;
		this->m_speed  = 0.12;
		this->m_sensitivity = 8;
		// default rotation
		this->m_rot = glm::vec2(0.0f);

		if (kbd)
		{
      restore_keyboard();
		}
		if (mouse)
		{
      restore_mouse();
		}
	}
  void Input::restore_keyboard()
  {
    // hook keyboard events
    glfwSetKeyCallback (m_window, &Input::keyboard);
    // disable sticky keys
    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);
  }
  void Input::restore_mouse()
  {
    // hook mouse events
    glfwSetCursorPosCallback(m_window, &Input::mouseMove);
    // mouse button event
    glfwSetMouseButtonCallback(m_window, &Input::mouseButton);
    // mouse wheel event
    glfwSetScrollCallback(m_window, &Input::mouseWheel);
  }
	Input::~Input()
	{
		currentInput = nullptr;
	}

	void Input::mouse_options(double speed, double sensitivity)
	{
		this->m_speed       = speed;
		this->m_sensitivity = sensitivity;
	}
	void Input::mouse_grab(const bool grab)
	{
    this->m_mousegrab = grab;
    if (grab)
      glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
      glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	int Input::mouse_wheel() const
	{
		int wheel = this->m_wheel;
		this->m_wheel = 0;
		return wheel;
	}

	void Input::keyboard(GLFWwindow* window, int key, int, int action, int mods)
	{
		auto* input = Input::inputFromWindow(window);
		if (input)
		{
			// action: GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE
			if (action == GLFW_PRESS || action == GLFW_REPEAT)
			{
				// as long as the key isnt force-held, set to pressed
				if (input->m_keys[key].action != Input::KEY_LOCKED)
					input->m_keys[key].action = Input::KEY_PRESSED;
			}   // key is no longer pressed
			else input->m_keys[key].action = Input::KEY_RELEASED;
			// keypress modifiers: SHIFT, CTRL, ALT
			input->m_keys[key].mods = mods;
		}
	}

	void Input::rotate_degrees(glm::vec2 degrees)
	{
		static const float PI = 4 * atan(1);
		static const float degToRad = PI / 180;
		static const float maxX = 89 * degToRad;
		static const float maxY = PI * 2.0;

		this->m_rot += degrees * degToRad;
		// clamping
		if (this->m_rot.x >  maxX) this->m_rot.x =  maxX;
		if (this->m_rot.x < -maxX) this->m_rot.x = -maxX;
		while (this->m_rot.y <     0) this->m_rot.y += PI * 2;
		while (this->m_rot.y >= maxY) this->m_rot.y -= PI * 2;
	}

	void Input::mouseMove(GLFWwindow* window, double x, double y)
	{
    Input* input = Input::inputFromWindow(window);
    assert (input != nullptr);
		if (input->m_mousegrab)
		{
			// in-game
			double dx = (x - input->m_last_mouse_xy.x) * input->m_speed;
			double dy = (y - input->m_last_mouse_xy.y) * input->m_speed;

			if (fabs(dx) > input->m_sensitivity)
				dx = signum(dx) * input->m_sensitivity + dx / input->m_sensitivity;
			if (fabs(dy) > input->m_sensitivity)
				dy = signum(dy) * input->m_sensitivity + dy / input->m_sensitivity;

			// rotation on axes
			input->rotate_degrees(glm::vec2(dy, dx));

			// remember the last pos >:|
			input->m_last_mouse_xy = {x, y};
		}
		else
		{
			input->m_last_mouse_xy.x = input->m_mouse_xy.x;
			input->m_last_mouse_xy.y = input->m_mouse_xy.y;
		}
		input->m_mouse_xy = {x, y};
	}

	void Input::mouseButton(GLFWwindow* window, int button, int action, int mods)
	{
		#define MACTION() (action == GLFW_PRESS) ? Input::KEY_PRESSED : Input::KEY_RELEASED

		auto* input = Input::inputFromWindow(window);
		assert (input != nullptr);
    input->m_mouse.at(button).action = MACTION();
    input->m_mouse.at(button).mods = mods;
	}

	void Input::mouseWheel(GLFWwindow* window, double, double y)
	{
		Input* input = Input::inputFromWindow(window);
		if (input)
		{
			if (y < 0)      input->m_wheel -= 1;
			else if (y > 0) input->m_wheel += 1;
			else            input->m_wheel  = 0;
		}
	}

	Input* Input::inputFromWindow(GLFWwindow*)
	{
		return currentInput;
	}
}
