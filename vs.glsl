uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;

layout(location = 0) in vec4 pos;
layout(location = 2) in vec2 texcoord;

smooth out vec2 vtexcoord;

void main(void)
{
    vtexcoord = texcoord;
    gl_Position = projection_matrix * modelview_matrix * pos;
}
