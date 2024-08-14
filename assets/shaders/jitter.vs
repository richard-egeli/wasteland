#version 330

layout(location = 0) in vec3 vertPos;   // Vertex position
layout(location = 1) in vec2 texPos;    // Texture coordinates

uniform mat4 modelViewProjection;

out vec2 fragTexCoord;

void main()
{
    fragTexCoord = texPos;
    gl_Position = modelViewProjection * vec4(vertPos, 1.0);
}
