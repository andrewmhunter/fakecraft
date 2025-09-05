#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in float fragDepth;

uniform sampler2D texture0;

out vec4 finalColor;

void main()
{
	vec4 texColor = texture(texture0, fragTexCoord);

	if (texColor.a == 0.0) {
		discard;
	}

	/**
	 * Expected light values:
	 * Top = 1.0 :)
	 * LightSide = 0.8 :)
	 * DarkSide = 0.6
	 * Bottom = 0.5
	*/

	//float shade = fragNormal.y * 0.2 - abs(fragNormal.x) * 0.2 + 0.8;

	float shade = fragNormal.y * 0.25 + abs(fragNormal.x) * 0.05 - abs(fragNormal.z) * 0.15 + 0.75;

	//float fog = 1.0 - (fragDepth - 2500.0) / 2000.0;
	float fog = 1.0;

	finalColor = texColor * fragColor * vec4(shade, shade, shade, fog);
	//finalColor = vec4(fragDepth / 1000, 0.0, 1.0, 1.0);
}

