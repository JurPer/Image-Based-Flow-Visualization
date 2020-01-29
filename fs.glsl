uniform sampler2D tex;
uniform float max_length;

smooth in vec2 vtexcoord;

layout(location = 0) out vec4 fcolor;

void main(void)
{
	/*
    float len = texture(tex, vtexcoord).r;
    len /= max_length;
    vec3 color = vec3(len, len, len);
    fcolor = vec4(color, 1.0);
	*/

	// output the texture color (RGBA)
    fcolor = vec4(texture(tex, vtexcoord));
}
