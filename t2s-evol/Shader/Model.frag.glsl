#version 400 core

#define FRAG_COLOR 0
layout(location = FRAG_COLOR) out vec4 FragColor;

in vec4 VertPosition;
in vec4 VertNormal;
in vec4 VertColor;
in vec4 VertTexture;
in vec4 VertShadowCoord;

uniform sampler2D tex;
uniform sampler2D shadowMap;
uniform vec3 Kd;
uniform vec3 lightPos;
uniform float shadowIntensity = 0.5;
uniform int applyShadow = 1;

float VSM(vec4 smcoord, sampler2D smb)
{
	vec3 coords = smcoord.xyz / smcoord.w;

	// avoid artifacts
	float eps = 0.1;
	if (coords.y > 1 - eps || coords.y < eps || coords.x > 1 - eps || coords.x < eps)
		return 1;

	if (smcoord.z < 1)
		return 1;

	float depth = coords.z;

	vec4 depthBlurrred = texture(smb, coords.xy);

	float depthSM = depthBlurrred.x;
	float sigma2 = depthBlurrred.y;

	sigma2 -= depthSM * depthSM;

	float bias = 0.00001;

	float dist = depth - depthSM;
	float P = sigma2 / (sigma2 + dist * dist);
	float lit = max(P, (depth - bias) <= depthSM ? 1.0 : 0.0);
	lit = min(1.0, lit);

	return mix(shadowIntensity, 1, lit);
}

void main()
{
	float shadow = 1.0;
	if (applyShadow == 1)
	{
		shadow *= VSM(VertShadowCoord, shadowMap);
	}

   vec4 color = VertColor;
   vec4 texColor = texture(tex, VertTexture.st);

   vec3 N = VertNormal.xyz;
   vec3 P = VertPosition.xyz;
   vec3 L = lightPos;

   float d = abs(dot(N, normalize(L-P)));
   color.rgb = (texColor.rgb * 0.25 + Kd * d * 0.75) * shadow;

   FragColor = vec4(color.rgb, 1);
}
