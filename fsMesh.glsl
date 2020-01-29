uniform sampler2D tex;
uniform float alpha;

smooth in vec2 vtexcoord;

layout(location = 0) out vec4 fcolor;

void main(void)
{
	// output the texture color (RGBA)
    fcolor = vec4(texture(tex, vtexcoord).rgb, alpha);
}
