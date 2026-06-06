#version 330

uniform mat4 projectionView;
uniform mat4 model;
uniform vec3 camPos;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexTexcoord;
layout (location = 2) in vec3 vertexNormal;
layout (location = 3) in vec4 vertexColor;

out vec2 fragTexcoord;
out vec4 fragColor;
out vec3 fragNormal;
out float fragDepth;

void main()
{
    fragTexcoord = vertexTexcoord;
    fragColor = vertexColor;
    fragNormal = vertexNormal;

    vec4 diff = model * vec4(vertexPosition, 1.0) - vec4(camPos, 1.0);
    diff = diff * diff;

    fragDepth = diff.x + diff.z;
    // fragDepth = diff.x + diff.z + diff.y;

    fragColor = vertexColor;

    gl_Position = projectionView * model * vec4(vertexPosition, 1.0);
}

