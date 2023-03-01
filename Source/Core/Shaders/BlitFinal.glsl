#version 330 core 

layout (location = 0) out vec4 o_Color;

in vec2 v_TexCoords;

uniform sampler2D u_Texture;

float saturate (float x)
{
    return min(1.0, max(0.0,x));
}
vec3 saturate (vec3 x)
{
    return min(vec3(1.,1.,1.), max(vec3(0.,0.,0.),x));
}


vec3 bump3 (vec3 x)
{
	vec3 y = vec3(1.,1.,1.) - x * x;
	y = max(y, vec3(0.,0.,0.));
	return y;
}

vec3 spectral_gems (float w)
{
    // w: [400, 700]
	// x: [0,   1]
	float x = saturate((w - 400.0)/ 300.0);

	return bump3
	(	vec3
		(
			4. * (x - 0.75),	// Red
			4. * (x - 0.5),	// Green
			4. * (x - 0.25)	// Blue
		)
	);
}

float remap(float x, float a, float b, float c, float d)
{
    return (((x - a) / (b - a)) * (d - c)) + c;
}


vec3 RomBinDaHouseToneMapping(vec3 color)
{
    color = exp( -1.0 / ( 2.72*color + 0.15 ) );
	color = pow(color, vec3(1. / 2.2));
	return color;
}

void main() {

	vec4 Sample = texture(u_Texture, v_TexCoords);
	float Value = Sample.x * 2.5f;
	vec4 Color = vec4(pow(sin(Value*vec3(1,1.2,1.5)), vec3(0.7f)) * vec3(1., 1., 1.6f), 1.);
	o_Color = vec4(Sample.xxx,1.); vec4((Color.xyz), 1.0f);
}