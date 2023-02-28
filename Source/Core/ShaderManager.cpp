#include "ShaderManager.h"
#include <sstream>

static std::unordered_map<std::string, GLClasses::Shader> ShaderManager_ShaderMap;
static std::unordered_map<std::string, GLClasses::ComputeShader> ShaderManager_ShaderMapC;

void Simulation::ShaderManager::CreateShaders()
{
	AddShader("BLIT", "Core/Shaders/FBOVert.glsl", "Core/Shaders/Blit.glsl");
	AddComputeShader("SIMULATE", "Core/Shaders/Simulate.glsl");
}

void Simulation::ShaderManager::AddShader(const std::string& name, const std::string& vert, const std::string& frag, const std::string& geo)
{
	auto exists = ShaderManager_ShaderMap.find(name);

	if (exists == ShaderManager_ShaderMap.end())
	{
		ShaderManager_ShaderMap.emplace(name, GLClasses::Shader());
		ShaderManager_ShaderMap.at(name).CreateShaderProgramFromFile(vert, frag);
		ShaderManager_ShaderMap.at(name).CompileShaders();
	}

	else
	{
		std::string err = "A shader with the name : " + name + "  already exists!";
		throw err;
	}
}

void Simulation::ShaderManager::AddComputeShader(const std::string& name, const std::string& comp)
{
	auto exists = ShaderManager_ShaderMapC.find(name);

	if (exists == ShaderManager_ShaderMapC.end())
	{
		ShaderManager_ShaderMapC.emplace(name, GLClasses::ComputeShader());
		ShaderManager_ShaderMapC.at(name).CreateComputeShader(comp);
		ShaderManager_ShaderMapC.at(name).Compile();
	}

	else
	{
		std::string err = "A shader with the name : " + name + "  already exists!";
		throw err;
	}
}

GLClasses::Shader& Simulation::ShaderManager::GetShader(const std::string& name)
{
	auto exists = ShaderManager_ShaderMap.find(name);

	if (exists != ShaderManager_ShaderMap.end())
	{
		return ShaderManager_ShaderMap.at(name);
	}

	else
	{
		throw "Shader that doesn't exist trying to be accessed!";
	}
}

GLClasses::ComputeShader& Simulation::ShaderManager::GetComputeShader(const std::string& name)
{
	auto exists = ShaderManager_ShaderMapC.find(name);

	if (exists != ShaderManager_ShaderMapC.end())
	{
		return ShaderManager_ShaderMapC.at(name);
	}

	else
	{
		throw "Shader that doesn't exist trying to be accessed!";
	}
}

GLuint Simulation::ShaderManager::GetShaderID(const std::string& name)
{
	auto exists = ShaderManager_ShaderMap.find(name);

	if (exists != ShaderManager_ShaderMap.end())
	{
		return ShaderManager_ShaderMap.at(name).GetProgramID();
	}

	else
	{
		throw "Shader that doesn't exist trying to be accessed!";
	}
}

void Simulation::ShaderManager::RecompileShaders()
{
	int ShadersRecompiled = 0;
	int ComputeShadersRecompiled = 0;

	for (auto& e : ShaderManager_ShaderMap)
	{
		ShadersRecompiled += (int)e.second.Recompile();
	}

	for (auto& e : ShaderManager_ShaderMapC)
	{
		ComputeShadersRecompiled += (int)e.second.Recompile();
	}
	
	std::cout << "\nShaders Recompiled : " << ShadersRecompiled << "   |   Compute Shaders Recompiled : " << ComputeShadersRecompiled;
}

void Simulation::ShaderManager::ForceRecompileShaders()
{
	for (auto& e : ShaderManager_ShaderMap)
	{
		e.second.ForceRecompile();
	}

	for (auto& e : ShaderManager_ShaderMapC)
	{
		e.second.ForceRecompile();
	}

	std::cout << "\nShaders Recompiled : " << ShaderManager_ShaderMap.size() << "   |   Compute Shaders Recompiled : " << ShaderManager_ShaderMapC.size();
}
