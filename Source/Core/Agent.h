#pragma once 

#include <iostream>
#include <glm/glm.hpp>

namespace Simulation {
	struct Agent {
		glm::vec2 Position;
		glm::vec2 Direction;
	};
}