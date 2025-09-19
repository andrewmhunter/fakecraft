#version 330

in vec3 vertexPosition;
in vec2 vertextTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform vec3 camPos;
uniform mat4 model;

out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out float fragDepth;

void main()
{
    fragTexCoord = vertextTexCoord;
    fragColor = vertexColor;
    fragNormal = vertexNormal;

    vec4 diff = model * vec4(vertexPosition, 1.0) - vec4(camPos, 1.0);
    diff = diff * diff;
    fragDepth = diff.x + diff.y + diff.z;

    fragColor = vertexColor;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}

