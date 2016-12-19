#version 400 core

#define VERT_POSITION	0
#define VERT_NORMAL     1
#define VERT_COLOR		2
#define VERT_TEXTURE    3
#define MAX_LIGHTS      10

uniform mat4x4 matModel;
uniform mat3x3 matNormal;
uniform mat4x4 matView;
uniform mat4x4 matProjection;
uniform mat4x4 matLightView[MAX_LIGHTS];

layout(location = VERT_POSITION) in vec4 Position;
layout(location = VERT_NORMAL)   in vec4 Normal;
layout(location = VERT_COLOR)    in vec4 Color;
layout(location = VERT_TEXTURE)  in vec4 Texture;

out vec4 VertPosition;
out vec4 VertNormal;
out vec4 VertColor;
out vec4 VertTexture;
out vec4 VertShadowCoord;

void main()
{	   
    VertPosition = matModel * Position; 
    VertNormal.rgb = normalize(matNormal * Normal.xyz);
	VertColor    = Color; 
	VertTexture  = Texture;

	VertShadowCoord = matLightView[0] * matModel * vec4(Position.xyz, 1);
	
    gl_Position = matProjection * matView * matModel * vec4(Position.xyz, 1);
}
