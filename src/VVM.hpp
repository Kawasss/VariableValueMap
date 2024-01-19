#pragma once // requires C++17
#include <string>
#include <vector>
#include <type_traits>

enum VVMResult
{
	VVM_SUCCESS,
	VVM_FILE_NOT_FOUND,
};

namespace VVM
{
	struct Group
	{
		struct Variable
		{
			std::string name;
			std::string value;

			template<typename T> T As()
			{
				if constexpr (std::is_same<std::string, T>::value || std::is_same<const char*, T>::value) return value;
				else if constexpr (std::is_same<float,  T>::value) return std::stof(value);
				else if constexpr (std::is_same<int,    T>::value) return std::stoi(value);
				else if constexpr (std::is_same<double, T>::value) return std::stod(value);
				else if constexpr (std::is_same<char,   T>::value) return value[0];
				else if constexpr (std::is_same<bool,   T>::value) return value[0] == '1';
				else return 0;
			}
		};
		std::string name;
		std::vector<Variable> variables;
	};

	inline extern void PushGroup(const std::string& name);
	inline extern void PopGroup();

	inline extern void AddVariable(const std::string& name, const std::string& value);
	template<typename T> void AddVariable(const std::string& name, T value)
	{
		std::string result;
		if constexpr (std::is_same<std::string, T>::value || std::is_same<const char*, T>::value)
			result = '"' + (std::string)value + '"';
		else
			result = std::to_string(value); // this does not really warn for variables that do not have a convertible type
		AddVariable(name, result);
	}
	inline extern Group::Variable& FindVariable(std::string name, std::vector<Group>& groups);

	inline extern VVMResult WriteToFile(const std::string& path);
	inline extern VVMResult ReadFromFile(const std::string& path, std::vector<Group>& groups);
}