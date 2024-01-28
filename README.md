# 'Variable: value' map

VVM is an easy way of reading and writing values from the disk. It has zero dependencies and can compile with a compiler that has C++17.

## Groups
VVM stores all variables into groups, which work like a namespaces for the variables. A group can contain variables and / or other groups. The variable ```MyGroup1.MyGroup2.myLocalVar1``` is represented like this in a file:

```
MyGroup1:
	MyGroup2:
		myLocalVar1: "my_value"
```

## Writing
Every group has its own 'scope': each variable in this scope must have an unique name, but it does not care about variable names outside the scope. A new group can be added and removed with these functions:

```
void VVM::PushGroup(const std::string& name);
void VVM::PopGroup();
```

```PushGroup``` adds a new group inside the current group, ```PopGroup``` moves one group back. The variables ```MyGroup1.MyGroup2.myLocalVar1```  and ```MyGroup1.myVar1``` can be created with this code:

```
VVM::PushGroup("MyGroup1");
VVM::AddVariable("myVar1", 1);

VVM::PushGroup("MyGroup2");
VVM::AddVariable("myLocalVar1", "my_value");
VVM::PopGroup();

VVM::PopGroup();
```

This sample also uses the function ```VVM::AddVariable```, which is (of course) used to add a variable to the current group. This function will accept the value as anything as long as it can be converted to a string:

```
template<typename T> void AddVariable(const std::string& name, T value);
```

There is also a macro avaible to simplify adding existing variables, to remove every macro simply define ```VVM_NO_MACROS```.

```
#define VVM_ADD_VARIABLE(variable) VVM::AddVariable(#variable, variable##)
```

This macro is useful for adding already existing variables to VVM. This macro will add a variable with the name and value of the parameter ```variable```, making a call to ```VVM::AddVariable``` look less complex. The macro can be used to simplify code like this:

```
float myLocalVar1 = 1.0f;
VVM::AddVariable("myLocalVar1", myLocalVar1);
```

To this:

```
float myLocalVar1 = 1.0f;
VVM_ADD_VARIABLE(myLocalVar1);
```

After creating all the groups and variables, it can be written to a file with this function:

```
VVMResult VVM::WriteToFile(const std::string& path);
```

This function will write all current current groups to the file given with ```path```. If the file does not it exist, it will create it first. It is recommended to end files with ".vvm", in order to recognize VVM files.

## Reading

Reading from a VVM file is done by one function:
```
VVMResult VVM::ReadFromFile(const std::string& path, std::vector<VVM::Group>& groups);
```

This will read from the file at ```path``` and write the result to the parameter ```groups```. Every group in the file will be seperated with an absolute path: the group ```MyGroup2```, which was pushed inside ```MyGroup1```, will be seperated and named ```MyGroup1.MyGroup2``` to comply with its path. If you want start over, for example to write to another file, then the ```VVM::Reset``` function should be called, this function completely removes all of the previous groups and variable that have been submitted:

```
void VVM::Reset();
```

The application does not have to search for a variable manually, luckily. This function can be used for that:

```
VVM::Group::Variable& VVM::FindVariable(std::string name, std::vector<VVM::Group>& groups);
```

The parameter ```name``` is expected to contain the absolute path to a variable: the variable ```myLocalVar1``` should be written as ```MyGroup1.MyGroup2.myLocalVar1``` in order to find it, for example. The function will look through the parameter  ```groups```  in order to find the variable. It will return a reference to a variable inside the ```groups``` vector.

 If the variable from ```name``` cannot be found it will return a reference to the first variable inside the group that ```name``` points to. If ```myLocalVar1``` does not exist, a reference to the first variable in the group ```MyGroup1.MyGroup2```  will be returned. If the given group does not exist, it will return a reference to the first variable inside the first (global) group. This function causes undefined behavior if the fallback groups do not have any variables to return a reference to.

The ```VVM::Group::Variable``` struct contains the name and value of a variable, both as a string. The value as a string is stored in ```VVM::Group::Variable::value```, but can be retrieved as a float, int etc. by calling the member function ```Variable::As```:

```
template<typename T> T As();
```

This function will return the value as the specified type. If the value can not be converted into the type it will return 0. Pairing this with ```VVM::FindVariable```, it becomes trivial to get any value from the VVM file:

```
float myCppVar = VVM::FindVariable("MyGroup1.myVvmVar", groups).As<float>();
```

Check out [Example.cpp](src/Example.cpp) for a sample on writing and reading from a file.
