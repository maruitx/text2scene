#version 400 core

#define VERT_POSITION	0
#define VERT_NORMAL     1
#define VERT_COLOR		2
#define VERT_TEXTURE    3

#define MAX_LIGHTS      10

uniform mat4x4 matModel;
uniform mat4x4 matView;
uniform mat4x4 matProjection;
uniform mat3x3 matNormal;

uniform mat4x4 matLightView[MAX_LIGHTS];
uniform vec4 clipPlane;
uniform int numLights;
uniform vec3 centerOffset;

layout(location = VERT_POSITION) in vec4 Position;
layout(location = VERT_NORMAL)   in vec4 Normal;
layout(location = VERT_COLOR)    in vec4 Color;
layout(location = VERT_TEXTURE)  in vec4 Texture;

out vec4 VertPosition;
out vec4 VertNormal;
out vec4 VertColor;
out vec4 VertTexture;
out vec4 VertShadowCoord[MAX_LIGHTS];

void main()
{	   
	VertPosition = matModel * (Position + vec4(centerOffset, 0));
	VertNormal = vec4(Normal.xyz, 0);
	//VertNormal.xyz = matNormal * Normal.xzy;
	VertColor    = Color;
	VertTexture  = Texture;
    
    for(int i=0; i<numLights; i++)
    {
       VertShadowCoord[i] = matLightView[i] * matModel * vec4(Position.xyz, 1);
    }

    vec4 tmpClip = clipPlane;
    tmpClip.y = 1.0;
    tmpClip.w = -0.1;

    //gl_ClipDistance[0] = dot(matModel * Position-vec4(0, -0.25, 0 ,0), tmpClip);	
	gl_Position = matProjection * matView * matModel * vec4(Position.xyz, 1);
}
