#version 330

uniform sampler2D texture0;
uniform float skyLight;
uniform vec4 fogColor;
uniform float fogDistance;
uniform float fogDropoff;

in vec2 fragTexcoord;
in vec4 fragColor;
in vec3 fragNormal;
in float fragDepth;

out vec4 finalColor;

void main()
{
	vec4 texColor = texture(texture0, fragTexcoord);

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

	float shade = fragNormal.y * 0.25 + abs(fragNormal.x) * 0.05 - abs(fragNormal.z) * 0.15 + 0.75;

	float fog = 1.0 - (fragDepth - fogDropoff) / fogDistance;
	fog = clamp(fog, 0.0, 1.0);

	float lighting = fragColor.r + fragColor.g * skyLight;
	shade *= lighting;
	finalColor = texColor * vec4(shade, shade, shade, 1.0);


	//finalColor = mix(fogColor, finalColor, fog);

	/// Crazy colors
	// finalColor = vec4(fragDepth / 1000, 0.0, 1.0, 1.0);
}

