#pragma once

#include <vector>
#include <sstream>

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

static std::vector<int> StringToIntegerList(const std::string &s, const std::string &prefix)
{
	std::string subString = PartitionString(s, prefix)[0];
	std::vector<std::string> parts = PartitionString(subString, " ");

	std::vector<int> result(parts.size());
	for (unsigned int resultIndex = 0; resultIndex < result.size(); resultIndex++)
	{
		result[resultIndex] = StringToInt(parts[resultIndex]);
	}
	return result;
}

static std::vector<float> StringToFloatList(const std::string &s, const std::string &prefix)
{
	std::string subString = PartitionString(s, prefix)[0];
	std::vector<std::string> parts = PartitionString(subString, " ");

	std::vector<float> result(parts.size());
	for (unsigned int resultIndex = 0; resultIndex < result.size(); resultIndex++)
	{
		result[resultIndex] = StringToFloat(parts[resultIndex]);
	}
	return result;
}
