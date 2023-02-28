#include "Pipeline.h"

#include "Agent.h"

#include "Utils/Random.h"

namespace Simulation {

	float CurrentTime = glfwGetTime();
	float Frametime = 0.0f;
	float DeltaTime = 0.0f;

	//Settings

	float EvaporateSpeed = 0.01f;
	float DiffuseSpeed = 0.2f;
	float RotationSpeed = 2.0f;
	float SensoryDistance = 4.0f;
	int SensorySampleRadius = 3;

	bool Paused = true;

	class RayTracerApp : public Simulation::Application
	{
	public:

		bool vsync;

		RayTracerApp()
		{
			m_Width = 800;
			m_Height = 600;
		}

		void OnUserCreate(double ts) override
		{

		}

		void OnUserUpdate(double ts) override
		{
			glfwSwapInterval((int)vsync);

			GLFWwindow* window = GetWindow();

		}

		void OnImguiRender(double ts) override
		{
			ImGuiIO& io = ImGui::GetIO();
			if (ImGui::Begin("Debug/Edit Mode"))

			{
				ImGui::SliderFloat("Evaporation Speed", &EvaporateSpeed, 0.0001f, 0.15f);
				ImGui::SliderFloat("Diffuse Speed", &DiffuseSpeed, 0.01f, 0.999f);
				ImGui::SliderFloat("Rotation Speed", &RotationSpeed, 0.01f, 48.0f);
				ImGui::SliderFloat("Sensory Distance", &SensoryDistance, 0.01f, 64.0f);
				ImGui::SliderInt("Sensory Sample Radius", &SensorySampleRadius, 1, 8);
			
				std::string Text = Paused ? "Unpause" : "Pause";

				if (ImGui::Button(Text.c_str())) {
					Paused = !Paused;
				}
			
			} ImGui::End();
		}

		void OnEvent(Simulation::Event e) override
		{
			ImGuiIO& io = ImGui::GetIO();

			if (e.type == Simulation::EventTypes::MousePress && !ImGui::GetIO().WantCaptureMouse)
			{
				if (!this->GetCursorLocked()) {

				}
			}

			if (e.type == Simulation::EventTypes::MouseMove && GetCursorLocked())
			{
			}


			if (e.type == Simulation::EventTypes::MouseScroll && !ImGui::GetIO().WantCaptureMouse)
			{
			}

			if (e.type == Simulation::EventTypes::WindowResize)
			{
			}

			if (e.type == Simulation::EventTypes::KeyPress && e.key == GLFW_KEY_ESCAPE) {
				exit(0);
			}

			if (e.type == Simulation::EventTypes::KeyPress && e.key == GLFW_KEY_F1)
			{
				this->SetCursorLocked(!this->GetCursorLocked());
			}

			if (e.type == Simulation::EventTypes::KeyPress && e.key == GLFW_KEY_F2 && this->GetCurrentFrame() > 5)
			{
				Simulation::ShaderManager::RecompileShaders();
			}

			if (e.type == Simulation::EventTypes::KeyPress && e.key == GLFW_KEY_F3 && this->GetCurrentFrame() > 5)
			{
				Simulation::ShaderManager::ForceRecompileShaders();
			}

			if (e.type == Simulation::EventTypes::KeyPress && e.key == GLFW_KEY_F5 && this->GetCurrentFrame() > 5)
			{
				Paused = !Paused;
			}

		}


	};

	GLClasses::Framebuffer Map(16, 16, { {GL_RGBA16F, GL_RGBA, GL_FLOAT, true, true}, {GL_R16F, GL_RED, GL_FLOAT, true, true} }, false, false);


	void Pipeline::StartPipeline()
	{
		// ZOrient
		const glm::mat4 ZOrientMatrix = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(1.0f));
		const glm::mat4 ZOrientMatrixNegative = glm::mat4(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f), glm::vec4(0.0f, -1.0f, 0.0f, 0.0f), glm::vec4(1.0f));

		// Application
		RayTracerApp app;
		app.Initialize();
		app.SetCursorLocked(false);

		// Create VBO and VAO for drawing the screen-sized quad.
		GLClasses::VertexBuffer ScreenQuadVBO;
		GLClasses::VertexArray ScreenQuadVAO;

		// Setup screensized quad for rendering
		{
			unsigned long long CurrentFrame = 0;
			float QuadVertices_NDC[] =
			{
				-1.0f,  1.0f,  0.0f, 1.0f, -1.0f, -1.0f,  0.0f, 0.0f,
				 1.0f, -1.0f,  1.0f, 0.0f, -1.0f,  1.0f,  0.0f, 1.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,  1.0f,  1.0f,  1.0f, 1.0f
			};

			ScreenQuadVAO.Bind();
			ScreenQuadVBO.Bind();
			ScreenQuadVBO.BufferData(sizeof(QuadVertices_NDC), QuadVertices_NDC, GL_STATIC_DRAW);
			ScreenQuadVBO.VertexAttribPointer(0, 2, GL_FLOAT, 0, 4 * sizeof(GLfloat), 0);
			ScreenQuadVBO.VertexAttribPointer(1, 2, GL_FLOAT, 0, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
			ScreenQuadVAO.Unbind();
		}

		ShaderManager::CreateShaders();

		// Shaders
		GLClasses::Shader& BlitShader = ShaderManager::GetShader("BLIT");
		GLClasses::ComputeShader& SimulateShader = ShaderManager::GetComputeShader("SIMULATE");
		GLClasses::ComputeShader& EvaporateShader = ShaderManager::GetComputeShader("EVAPORATE");
		GLClasses::Shader& DiffuseShader = ShaderManager::GetShader("DIFFUSE");

		// Matrices
		float OrthographicRange = 400.0f;
		OrthographicCamera Orthographic(-400.0f, 400.0f, -400.0f, 400.0f);

		// Framebuffer Output 
		GLClasses::Framebuffer SimulationMap(16, 16, { {GL_RGBA16F, GL_RGBA, GL_FLOAT, true, true} }, true, false);
		GLClasses::Framebuffer DiffuseMap(16, 16, { {GL_RGBA16F, GL_RGBA, GL_FLOAT, true, true} }, true, false);

		// Create Agents 

		int AgentCount = 200000;
		std::vector<Agent> Agents;

		bool SpawnInCircle = true;
		Random RNG;

		if (SpawnInCircle) {
			Agents.resize(AgentCount);

			const float Pi = 3.14159265359;
			float Radius = glm::sqrt((float(AgentCount) / Pi));

			std::cout << Radius << "\n\n";

			float HalfR = Radius ;
			int iHalfR = int(HalfR);

			int IndexAgent = 0;

			for (int x = -iHalfR; x < iHalfR; x++) {
				for (int y = -iHalfR; y < iHalfR; y++) {
					float p = x * x + y * y;

					if (p < Radius * Radius) {
						glm::vec2& Pos = Agents[IndexAgent].Position;
						Pos = glm::vec2(x, y);
						Agents[IndexAgent].Direction = glm::normalize(-Pos);

						IndexAgent++;
					}
				}
			}

		}

		else {
			Agents.resize(AgentCount);

			for (int i = 0; i < AgentCount; i++) {
				glm::vec2& Pos = Agents[i].Position;

				Pos.x = (RNG.Float() * 2.0f - 1.0f) * (OrthographicRange - 1.0f);
				Pos.y = (RNG.Float() * 2.0f - 1.0f) * (OrthographicRange - 1.0f);

				Agents[i].Direction = glm::vec2(RNG.Float() * 2.0f - 1.0f, RNG.Float() * 2.0f - 1.0f);
				//Agents[i].Direction = glm::vec2(0.0f, -1.0f);
				Agents[i].Direction = glm::normalize(Agents[i].Direction);
			}
		}

		// Agent SSBO
		GLuint AgentSSBO = 0;
		glGenBuffers(1, &AgentSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, AgentSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Agent) * AgentCount, (void*)Agents.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// Clear simulation map
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		SimulationMap.Bind();
		DiffuseMap.Bind();
		glClear(GL_COLOR_BUFFER_BIT);

		while (!glfwWindowShouldClose(app.GetWindow())) {

			app.OnUpdate();

			// Resize Framebuffers 
			SimulationMap.SetSize(app.GetWidth(), app.GetHeight());
			DiffuseMap.SetSize(app.GetWidth(), app.GetHeight());

			if (!Paused || app.GetCurrentFrame() < 3) {
				// Simulate Agents 

				SimulateShader.Use();
				SimulateShader.SetMatrix4("u_OrthographicProjection", Orthographic.GetProjectionMatrix());
				SimulateShader.SetFloat("u_Dt", DeltaTime);
				SimulateShader.SetFloat("u_Time", glfwGetTime());
				SimulateShader.SetInteger("u_AgentCount", AgentCount);

				SimulateShader.SetFloat("u_RotationSpeed", RotationSpeed);
				SimulateShader.SetFloat("u_SensoryDistanceOffset", SensoryDistance);
				SimulateShader.SetInteger("u_SensorySampleRadius", SensorySampleRadius);


				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, AgentSSBO);
				glBindImageTexture(0, SimulationMap.GetTexture(), 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
				glDispatchCompute((AgentCount / 16) + 2, 1, 1);


				// Perform Diffusion

				glFinish();
				DiffuseMap.Bind();
				DiffuseShader.Use();
				DiffuseShader.SetInteger("u_Input", 0);
				DiffuseShader.SetFloat("u_DiffuseSpeed", DiffuseSpeed);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, SimulationMap.GetTexture());

				ScreenQuadVAO.Bind();
				glDrawArrays(GL_TRIANGLES, 0, 6);
				ScreenQuadVAO.Unbind();

				DiffuseMap.Unbind();

				// Copy Framebuffers
				SimulationMap.Bind();
				BlitShader.Use();
				BlitShader.SetInteger("u_Texture", 0);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, DiffuseMap.GetTexture());

				ScreenQuadVAO.Bind();
				glDrawArrays(GL_TRIANGLES, 0, 6);
				ScreenQuadVAO.Unbind();

				// Evaporate 
				EvaporateShader.Use();
				EvaporateShader.SetFloat("u_Dt", DeltaTime);
				EvaporateShader.SetFloat("u_Time", glfwGetTime());
				EvaporateShader.SetFloat("u_EvaporateSpeed", EvaporateSpeed);

				glBindImageTexture(0, SimulationMap.GetTexture(), 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
				glDispatchCompute((int)floor(float(SimulationMap.GetWidth()) / 16.0f) + 1, (int)(floor(float(SimulationMap.GetHeight())) / 16.0f) + 1, 1);

			}

			// Blit Final Result 
			glBindFramebuffer(GL_FRAMEBUFFER,0);
			BlitShader.Use();
			BlitShader.SetInteger("u_Texture", 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, SimulationMap.GetTexture());

			ScreenQuadVAO.Bind();
			glDrawArrays(GL_TRIANGLES, 0, 6);
			ScreenQuadVAO.Unbind();

			glUseProgram(0);

			glFinish();
			app.FinishFrame();

			CurrentTime = glfwGetTime();
			DeltaTime = CurrentTime - Frametime;
			Frametime = glfwGetTime();

			GLClasses::DisplayFrameRate(app.GetWindow(), "Candela ");
		}
	}
}