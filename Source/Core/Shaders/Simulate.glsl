#version 450 core 

#define PI 3.14159265359 

layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
layout(rgba16f, binding = 0) uniform image2D o_OutputData;

struct Agent {
	vec2 Position; 
	vec2 Direction;
};

layout (std430, binding = 0) buffer AgentSSBO {
	Agent SimulationAgents[];
};


uniform mat4 u_OrthographicProjection;
uniform float u_Dt;
uniform float u_Time;
uniform int u_AgentCount;

// Implementation.
// RNG
float HASH2SEED = 0.0f;
vec2 hash2() 
{
	return fract(sin(vec2(HASH2SEED += 0.1, HASH2SEED += 0.1)) * vec2(43758.5453123, 22578.1459123));
}

void main() {

	int Index = int(gl_GlobalInvocationID.x);

	if (Index > u_AgentCount) {
		return;
	}

	HASH2SEED = float(Index) / 512.0f * 100.0f * u_Time;
	Agent CurrentAgent = SimulationAgents[Index];

	vec2 Position = vec2(CurrentAgent.Position.xy);

	vec4 Projected = u_OrthographicProjection * vec4(Position, 1.0f, 1.0f);
	Projected = Projected * 0.5f + 0.5f;

	ivec2 Pixel = ivec2(Projected.xy * imageSize(o_OutputData).xy);


	if (Projected.xy != clamp(Projected.xy, 0.0f, 1.0f)) {
		float Angle = hash2().x * 1.0f * PI;
		vec2 Vector = vec2(cos(Angle), sin(Angle));

		SimulationAgents[Index].Direction = normalize(-SimulationAgents[Index].Direction + Vector);

	}

	SimulationAgents[Index].Position += SimulationAgents[Index].Direction * u_Dt * 10.0f;

	float Value = 1.0f;
	imageStore(o_OutputData, Pixel, vec4(Value.xxx, 1.0f));
}
