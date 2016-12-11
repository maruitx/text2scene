#ifndef TRANSFORMFEEDBACK_H
#define TRANSFORMFEEDBACK_H

#include "Headers.h"

class Shader;
class VertexBufferObject;

class TransformFeedback
{
public:
	TransformFeedback();
	~TransformFeedback();

	void saveAsObj(QString path, int nr);
	void printBuffer(int nr);
	int renderToBuffer(VertexBufferObject *vbo, const Transform &trans);

private:

	struct Face
	{
		uint a, b, c;
	};

	void initBuffer();
	void initShader();	
	
	VertexBufferObject *m_vboFeedBack;
	Shader *m_shaderFeedback;

};

#endif

