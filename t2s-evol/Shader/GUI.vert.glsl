#version 400 core

#define VERT_POSITION	0
#define VERT_NORMAL     1
#define VERT_COLOR		2
#define VERT_TEXTURE    3

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

layout(location = VERT_POSITION) in vec4 Position;
layout(location = VERT_NORMAL)   in vec4 Normal;
layout(location = VERT_COLOR)    in vec4 Color;
layout(location = VERT_TEXTURE)  in vec4 Texture;

out vec4 VertPosition;
out vec3 VertNormal;
out vec4 VertColor;
out vec4 VertTexture;

void main()
{	   
    VertPosition = Position; 
    VertNormal   = Normal.xyz;
	VertColor    = Color;
	VertTexture  = Texture;
	
    gl_Position = matProjection * matView * matModel * vec4(Position.xyz, 1);
}
