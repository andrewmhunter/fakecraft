#version 330 core

//uniform sampler2D texture0;
uniform vec4 color;

in vec4 fragColor;
in vec2 fragTexcoord;

out vec4 FragColor;

void main()
{
    FragColor = fragColor * color; // * texture(texture0, fragTexcoord);
};

