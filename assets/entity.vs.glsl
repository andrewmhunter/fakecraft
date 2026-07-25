#version 330 core

layout (std140) uniform Globals
{
    mat4 projectionView;
    vec3 cameraPosition;
};

uniform mat4 model;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexTexcoord;
layout (location = 3) in vec4 vertexColor;

out vec4 fragColor;
out vec2 fragTexcoord;

void main()
{
    fragColor = vertexColor;
    fragTexcoord = vertexTexcoord;
    gl_Position = projectionView * model * vec4(vertexPosition, 1.0);
};

