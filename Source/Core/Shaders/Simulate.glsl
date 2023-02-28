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

// Options 
uniform int u_SensorySampleRadius;
uniform float u_SensoryDistanceOffset;
uniform float u_RotationSpeed;


uniform mat4 u_OrthographicProjection;
uniform float u_Dt;
uniform float u_Time;
uniform int u_AgentCount;

float remap(float x, float a, float b, float c, float d)
{
    return (((x - a) / (b - a)) * (d - c)) + c;
}

// Implementation.
// RNG
float HASH2SEED = 0.0f;
vec2 hash2() 
{
	return fract(sin(vec2(HASH2SEED += 0.1, HASH2SEED += 0.1)) * vec2(43758.5453123, 22578.1459123));
}



// Position is in image space! (integer coordinates)
vec4 GetConcentration(vec2 Position) {
	
	vec2 TexelSize = 1.0f / imageSize(o_OutputData);

	vec4 TotalConcentration = 0.0f.xxxx;

	for (int x = -u_SensorySampleRadius ; x <= u_SensorySampleRadius ; x++) {

		for (int y = -u_SensorySampleRadius ; y <= u_SensorySampleRadius ; y++) {

			vec2 SamplePosition = Position + vec2(x,y) * 1.;

			TotalConcentration += imageLoad(o_OutputData, ivec2(SamplePosition));
		}

	}

	return TotalConcentration;

}

const float Cos45 = cos(0.785398f);
const float Sin45 = sin(0.785398f);
const mat2 Rot45Deg = mat2(vec2(Cos45, -Sin45), vec2(Sin45, Cos45));

const float CosNeg45 = cos(-0.785398f);
const float SinNeg45 = sin(-0.785398f);
const mat2 RotNeg45Deg = mat2(vec2(CosNeg45, -SinNeg45), vec2(SinNeg45, CosNeg45)); // 90 + 45 or -45 from +x axis

vec2 GetWeightedDirectionNorm(vec2 OriginalDirection, vec2 PixelP) {

	OriginalDirection = normalize(OriginalDirection);
	vec2 NewDirection = OriginalDirection;
	vec2 DirectionL = RotNeg45Deg * OriginalDirection;
	vec2 DirectionR = Rot45Deg * OriginalDirection;
	vec2 DirectionC = OriginalDirection;
	vec2 Hash = hash2();

	float ConcentrationL = GetConcentration(PixelP + DirectionL * u_SensoryDistanceOffset).x;
	float ConcentrationR = GetConcentration(PixelP + DirectionR * u_SensoryDistanceOffset).x;
	float ConcentrationC = GetConcentration(PixelP + DirectionC * u_SensoryDistanceOffset).x;

	if (ConcentrationC > ConcentrationL && ConcentrationC > ConcentrationR) {
		NewDirection = OriginalDirection;
	}

	else if (ConcentrationC < ConcentrationL && ConcentrationC < ConcentrationR) {
		float CosTheta = cos((Hash.x - 0.5f) * u_RotationSpeed * 2.0f * u_Dt);
		float SinTheta = sin((Hash.x - 0.5f) * u_RotationSpeed * 2.0f * u_Dt);
		mat2 RotationMatrix = mat2(vec2(CosTheta, -SinTheta), vec2(SinTheta, CosTheta));
		NewDirection = normalize(RotationMatrix * OriginalDirection);
	}

	else if (ConcentrationR > ConcentrationL) {
		float CosTheta = cos(Hash.x * u_RotationSpeed * u_Dt);
		float SinTheta = sin(Hash.x * u_RotationSpeed * u_Dt);
		mat2 RotationMatrix = mat2(vec2(CosTheta, -SinTheta), vec2(SinTheta, CosTheta));
		NewDirection = normalize(RotationMatrix * OriginalDirection);
	}

	else if (ConcentrationL > ConcentrationR) {
		float CosTheta = cos(-Hash.x * u_RotationSpeed * u_Dt);
		float SinTheta = sin(-Hash.x * u_RotationSpeed * u_Dt);
		mat2 RotationMatrix = mat2(vec2(CosTheta, -SinTheta), vec2(SinTheta, CosTheta));
		NewDirection = normalize(RotationMatrix * OriginalDirection);
	}

	return NewDirection;
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

	vec2 PixelF = Projected.xy * imageSize(o_OutputData).xy;

	SimulationAgents[Index].Direction = GetWeightedDirectionNorm(SimulationAgents[Index].Direction, PixelF);
	vec2 NewPosition = SimulationAgents[Index].Position + SimulationAgents[Index].Direction * u_Dt * 10.0f;

	if (Projected.xy != clamp(Projected.xy, 0.0f, 1.0f)) {
		float Angle = hash2().x * 1.0f * PI; // Cover only semicircle
		vec2 Vector = vec2(cos(Angle), sin(Angle));

		SimulationAgents[Index].Direction = normalize(-SimulationAgents[Index].Direction + Vector);
		NewPosition = clamp(NewPosition, -100.0f, 100.0f);
	}

	SimulationAgents[Index].Position = NewPosition;

	float Value = 1.0f;
	imageStore(o_OutputData, Pixel, vec4(Value.xxx, 1.0f));
}
