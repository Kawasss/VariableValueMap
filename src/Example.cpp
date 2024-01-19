#include <iostream>
#include "VVM.hpp"

int main()
{
	VVM::AddVariable("myGlobalVar1", 1.0f);

	VVM::PushGroup("myGroup1");
	VVM::AddVariable("myGroupVar1", "myGroupValue1");

	VVM::PushGroup("myGroup2");
	VVM::AddVariable("myGroupVar1", "myGroupValue1");
	VVM::AddVariable("myGroupVar2", 3);
	VVM::PopGroup();

	VVM::AddVariable("myGroupVar2", "myGroupValue2");
	VVM::AddVariable("myGroupVar3", 1);
	VVM::PopGroup();

	VVM::AddVariable("myGlobalVar2", "myGlobalValue1");

	if (VVM::WriteToFile("test.vvm") != VVM_SUCCESS)
		return 1;

	std::vector<VVM::Group> groups;
	if (VVM::ReadFromFile("test.vvm", groups) != VVM_SUCCESS)
		return 1;

	std::string var = VVM::FindVariable("myGlobalVar2", groups).As<std::string>();
	std::cout << var << "\n";

	for (int i = 0; i < groups.size(); i++)
	{
		for (int k = 0; k < i; k++) // simulate the indentations
			std::cout << "  ";
		std::cout << groups[i].name << ":\n";
		for (int j = 0; j < groups[i].variables.size(); j++)
		{
			for (int k = 0; k < i; k++) // simulate the indentations
				std::cout << "  ";
			std::cout << groups[i].variables[j].name << ": " << groups[i].variables[j].value << "\n";
		}
	}
	return 0;
}