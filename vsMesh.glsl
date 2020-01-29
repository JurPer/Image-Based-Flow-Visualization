uniform mat4 projection_matrix;

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 texcoord;

smooth out vec2 vtexcoord;

void main(void)
{
    vtexcoord = texcoord;
    gl_Position = projection_matrix * vec4(pos, 1.0f);
}
