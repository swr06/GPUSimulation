#pragma once 

#include <iostream>
#include "Utils/Vertex.h"
#include "GLClasses/ComputeShader.h"
#include "GLClasses/Texture.h"
#include "Application/Application.h"
#include "GLClasses/VertexArray.h"
#include "GLClasses/VertexBuffer.h"
#include "GLClasses/IndexBuffer.h"
#include "GLClasses/Framebuffer.h"
#include "GLClasses/Shader.h"
#include "ShaderManager.h"
#include "GLClasses/Fps.h"
#include "Orthographic.h"

namespace Simulation {
	namespace Pipeline {
		void StartPipeline();
	}
}