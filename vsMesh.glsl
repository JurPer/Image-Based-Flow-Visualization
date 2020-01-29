layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 texcoord;

smooth out vec2 vtexcoord;

void main(void)
{
    vtexcoord = texcoord;
    gl_Position = vec4(pos, 1.0f);
}
