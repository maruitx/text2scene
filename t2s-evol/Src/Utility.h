#pragma once

#include <vector>
#include <sstream>
#include <QString>
#include "Math.h"
#include <QQuaternion>

static bool FileExists(const std::string &filename)
{
	std::ifstream file(filename);
	return (!file.fail());
}

static std::vector<std::string> GetFileLines(const std::string &filename, unsigned int minLineLength)
{
	std::vector<std::string> result;
	if (!FileExists(filename)) {
		std::cout << "Required file not found: " << filename <<"\n";
		return result;
	}
	std::ifstream file(filename);

	std::string curLine;
	while (!file.fail())
	{
		std::getline(file, curLine);
		if (!file.fail() && curLine.length() >= minLineLength)
		{
			if (curLine.at(curLine.length() - 1) == '\r')
				curLine = curLine.substr(0, curLine.size() - 1);
			result.push_back(curLine);
		}
	}
	return result;
}

static std::vector<std::string> PartitionString(const std::string &s, const std::string &separator)
{
	std::vector<std::string> result;
	std::string curEntry;
	for (unsigned int outerCharacterIndex = 0; outerCharacterIndex < s.length(); outerCharacterIndex++)
	{
		bool isSeperator = true;
		for (unsigned int innerCharacterIndex = 0; innerCharacterIndex < separator.length() && outerCharacterIndex + innerCharacterIndex < s.length() && isSeperator; innerCharacterIndex++)
		{
			if (s[outerCharacterIndex + innerCharacterIndex] != separator[innerCharacterIndex]) {
				isSeperator = false;
			}
		}
		if (isSeperator) {
			if (curEntry.length() > 0) {
				result.push_back(curEntry);
				curEntry.clear();
			}
			outerCharacterIndex += separator.length() - 1;
		}
		else {
			curEntry.push_back(s[outerCharacterIndex]);
		}
	}
	if (curEntry.length() > 0) {
		result.push_back(curEntry);
	}
	return result;
}

static int StringToInt(const std::string &s)
{
	std::stringstream stream(std::stringstream::in | std::stringstream::out);
	stream << s;

	int result;
	stream >> result;
	return result;
}

static float StringToFloat(const std::string &s)
{
	std::stringstream stream(std::stringstream::in | std::stringstream::out);
	stream << s;

	float result;
	stream >> result;
	return result;
}

static std::vector<int> StringToIntegerList(const std::string &s, const std::string &prefix, const std::string &separator = " ")
{
	std::string subString;
	if (prefix == "")
	{
		subString = s;
	}
	else
	{
		subString = PartitionString(s, prefix)[0];
	}

	std::vector<std::string> parts = PartitionString(subString, separator);

	std::vector<int> result(parts.size());
	for (unsigned int resultIndex = 0; resultIndex < result.size(); resultIndex++)
	{
		result[resultIndex] = StringToInt(parts[resultIndex]);
	}
	return result;
}

static std::vector<float> StringToFloatList(const std::string &s, const std::string &prefix, const std::string &separator = " ")
{
	std::string subString;
	if (prefix == "")
	{
		subString = s;
	}
	else
	{
		subString = PartitionString(s, prefix)[0];
	}
	std::vector<std::string> parts = PartitionString(subString, separator);

	std::vector<float> result(parts.size());
	for (unsigned int resultIndex = 0; resultIndex < result.size(); resultIndex++)
	{
		result[resultIndex] = StringToFloat(parts[resultIndex]);
	}
	return result;
}

static vec3 TransformPoint(const mat4 &transMat, const vec3 &p)
{
	vec3 transP = vec3(transMat.a11*p.x + transMat.a12*p.y + transMat.a13*p.z + transMat.a14,
		transMat.a21*p.x + transMat.a22*p.y + transMat.a23*p.z + transMat.a24,
		transMat.a31*p.x + transMat.a32*p.y + transMat.a33*p.z + transMat.a34);

	return transP;
}

static vec3 TransformVector(const mat4 &transMat, const vec3 &v)
{
	vec3 transV = vec3(transMat.a11*v.x + transMat.a12*v.y + transMat.a13*v.z,
		transMat.a21*v.x + transMat.a22*v.y + transMat.a23*v.z,
		transMat.a31*v.x + transMat.a32*v.y + transMat.a33*v.z);

	return transV;
}

static mat4 GetRotationMatrix(const vec3 &fromVec, const vec3 &toVec)
{
	QQuaternion rotQ = QQuaternion::rotationTo(QVector3D(fromVec.x, fromVec.y, fromVec.z), QVector3D(toVec.x, toVec.y, toVec.z));
	QMatrix3x3 qrotMat = rotQ.toRotationMatrix();

	mat4 rotMat;
	rotMat.setToIdentity();
	
	rotMat.a11 = qrotMat(0, 0); rotMat.a12 = qrotMat(0, 1); rotMat.a13 = qrotMat(0, 2);
	rotMat.a21 = qrotMat(1, 0); rotMat.a22 = qrotMat(1, 1); rotMat.a23 = qrotMat(1, 2);
	rotMat.a31 = qrotMat(2, 0); rotMat.a32 = qrotMat(2, 1); rotMat.a33 = qrotMat(2, 2);

	return rotMat;
}

static mat4 GetTransformationMat(const mat4 &rotMat, const vec3 &currPos, const vec3 &tarPos)
{
	mat4 transMat;
	transMat.setToIdentity();

	vec3 transVec = tarPos - TransformPoint(rotMat, currPos);
	transMat = rotMat*transMat;
	transMat.a14 = transVec.x; transMat.a24 = transVec.y; transMat.a34 = transVec.z;

	return transMat;
}
