#version 450 core 

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba16f, binding = 0) uniform image2D Image;

in vec2 v_TexCoords;

uniform float u_Dt;

// Options
uniform float u_EvaporateSpeed;

void main() {

	ivec2 Pixel = ivec2(gl_GlobalInvocationID.xy);

	ivec2 Dimensions = ivec2(imageSize(Image).xy);

	if (Pixel.x > 0 && Pixel.x < Dimensions.x && Pixel.y > 0 && Pixel.y < Dimensions.y) {
		vec4 Value = imageLoad(Image, Pixel);
		Value -= u_Dt * u_EvaporateSpeed;
		Value = max(Value, 0.0f);

		imageStore(Image, Pixel, Value);
	}
}