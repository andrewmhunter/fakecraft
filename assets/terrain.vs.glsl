#version 330

layout (std140) uniform Globals
{
    mat4 projectionView;
    vec3 cameraPosition;
};

uniform mat4 model;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec2 vertexTexcoord;
layout (location = 2) in vec3 vertexNormal;
layout (location = 3) in vec4 vertexColor;
layout (location = 4) in int boneId;

out vec2 fragTexcoord;
out vec4 fragColor;
out vec3 fragNormal;
out float fragDepth;

void main()
{
    fragTexcoord = vertexTexcoord;
    fragColor = vertexColor;
    fragNormal = vertexNormal;

    vec4 diff = model * vec4(vertexPosition, 1.0) - vec4(cameraPosition, 1.0);
    diff = diff * diff;

    fragDepth = diff.x + diff.z;
    // fragDepth = diff.x + diff.z + diff.y;

    fragColor = vertexColor;

    gl_Position = projectionView * model * vec4(vertexPosition, 1.0);
}

