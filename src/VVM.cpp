#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>

#include "VVM.hpp"

struct Node
{
	std::string name;
	std::string value;
	bool isGroup;
	bool isPop;

	operator VVM::Group::Variable()
	{
		return { name, value };
	}
};

inline std::string NodeToString(const Node& node)
{
	return node.name + ": " + (node.isGroup ? "" : node.value) + "\n";
}

uint64_t indentationLevel = 0;
std::vector<Node> nodes;

void VVM::PushGroup(const std::string& name)
{
	nodes.push_back({ name, "", true });
}

void VVM::PopGroup()
{
	nodes.push_back({ "", "", false, true });
}

void VVM::AddVariable(const std::string& name, const std::string& value)
{
	nodes.push_back({ name, value, false });
}

inline VVM::Group::Variable& GetVar(std::string name, std::vector<VVM::Group::Variable>& vars)
{
	for (int i = 0; i < vars.size(); i++)
		if (vars[i].name == name)
			return vars[i];
	return vars[0]; // not safe if the vector is empty
}

VVM::Group::Variable& VVM::FindVariable(std::string name, std::vector<Group>& groups)
{
	size_t lastDot = name.find('.');
	if (lastDot == std::string::npos)
		return GetVar(name, groups[0].variables);

	std::vector<std::string> dissectedName;
	while (name.find('.', lastDot + 1) != std::string::npos)
		lastDot = name.find('.', lastDot + 1);

	std::string var = name.substr(lastDot + 1);
	std::string group = name.substr(0, lastDot);

	for (int i = 0; i < groups.size(); i++)
		if (groups[i].name == group)
			return GetVar(var, groups[i].variables);
	return groups[0].variables[0]; // not safe if vector is empty
}

VVMResult VVM::WriteToFile(const std::string& path)
{
	std::ofstream stream(path, std::ios::out);
	if (!stream.good())
		return VVM_FILE_NOT_FOUND;

	for (size_t i = 0; i < nodes.size(); i++)
	{
		if (nodes[i].isPop)
		{
			indentationLevel--;
			continue;
		}

		std::string write = "";
		for (size_t j = 0; j < indentationLevel; j++)
			write += '\t';
		write += NodeToString(nodes[i]);

		stream << write;

		if (nodes[i].isGroup)
			indentationLevel++;
	}

	stream.close();
	return VVM_SUCCESS;
}

inline std::vector<std::string> LexFile(const std::string& input)
{
	std::vector<std::string> ret;
	std::string processingString = "";
	for (size_t i = 0; i < input.size(); i++)
	{
		if (input[i] == ' ' || input[i] == '\r')
			continue;

		switch (input[i])
		{
		case ':':
		case ',':
		case '\t':
		case '\n':
			if (!processingString.empty())
				ret.push_back(processingString);
			ret.push_back({ input[i] });
			processingString = "";
			continue;
		}

		processingString += input[i];
		if (i == input.size() - 1)
			ret.push_back(processingString);
	}
	return ret;
}

inline std::vector<VVM::Group> ParseTokens(const std::vector<std::string>& tokens)
{
	std::vector<VVM::Group> ret = { { "global" }}; // the first group is the global one
	std::unordered_map<size_t, std::string> groupHierarchy;
	
	VVM::Group::Variable currentVar;
	bool readingLValue = true;
	size_t indentLevel = 0;

	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].size() == 1)
			switch (tokens[i][0])
			{
			case ':':
			{
				if (tokens[i + 1] != "\n")
				{
					readingLValue = false;
					continue;
				}
				std::string prefix = indentLevel == 0 ? "" : groupHierarchy[indentLevel - 1] + '.';
				groupHierarchy[indentLevel] = prefix + tokens[i - 1];
				ret.push_back({ prefix + tokens[i - 1] });
				currentVar = {};
				indentLevel = 0;
				i++; // skip over the \n character so that the other case doesnt get invoked next cycle
				continue;
			}
			case '\n':
				ret[indentLevel].variables.push_back(currentVar);
				indentLevel = 0;
				currentVar = {};
				readingLValue = true;
				continue;
			case '\t':
				indentLevel++;
				continue;
			}
		if (readingLValue)
		{
			currentVar.name = tokens[i];
			if (indentLevel == 0)
				continue;
		}
		else
		{
			currentVar.value = tokens[i];
			if (currentVar.value[0] == '"' && currentVar.value.back() == '"') // remove the string literal signs
			{
				currentVar.value.erase(currentVar.value.begin());
				currentVar.value.erase(currentVar.value.end() - 1);
			}
		}
	}
	return ret;
}

VVMResult VVM::ReadFromFile(const std::string& path, std::vector<Group>& groups)
{
	std::ifstream stream(path, std::ios::ate);
	if (!stream.good())
		return VVM_FILE_NOT_FOUND;
	
	size_t fileSize = (size_t)stream.tellg();
	
	std::vector<char> input(fileSize + 1);
	input.back() = '\0';

	stream.seekg(0);
	stream.read(input.data(), fileSize);

	stream.close();

	std::string source = input.data();

	std::vector<std::string> tokens = LexFile(source);
	groups = ParseTokens(tokens);
	return VVM_SUCCESS;
}