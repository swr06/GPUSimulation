#version 330 core 

layout (location = 0) out vec4 o_Data;

in vec2 v_TexCoords;

uniform sampler2D u_Input;

// Options 
uniform float u_DiffuseSpeed;

void main() {

	const float[3] Weights = float[3] (1.0f, 2.0f / 3.0f, 1.0f / 6.0f);

	vec4 Total = vec4(0.0f);
	vec2 TexelSize = 1.0f / textureSize(u_Input, 0);
	float TotalWeight = 0.0f;

	vec4 Center = vec4(0.);
	
	for (int x = -1 ; x <= 1 ; x++) {

		for (int y = -1 ; y <= 1 ; y++) {

			float Weight = Weights[abs(x)] * Weights[abs(y)];
			TotalWeight += Weight;

			vec4 Sample = texture(u_Input, v_TexCoords + vec2(x,y) * TexelSize);

			if (x == 0 && y == 0) {
				Center = Sample;
			}

			Total += Sample * Weight;
		}

	}

	Total /= TotalWeight;

	o_Data = mix(Total, Center, 1.-u_DiffuseSpeed);

}