<h2> DIRECTORY SETUP/INSTALL HUMAN FACTORS </h2>

Create a folder named 'Analysis Sample Usage' in your development directory of choice.

Open Visual Studio 2019 and open the Analysis repository.
(Analysis/src). This will use the CMakeLists.txt file to work with the codebase within VS.

Click Build > Install Human Factors in Visual Studio.
This will install Human Factors in
Analysis\src\out\install.

After installing, navigate to
Analysis\src\out\install.

Copy the x64-Release folder into the 'Analysis Sample Usage' folder you made.

In Analysis Sample Usage\x64-Release, keep only the following files,

bin\embree3.dll
bin\HumanFactors.dll
bin\tbb.dll

Delete everything else.

In Analysis\src\Cpp\tests\Example Models\, copy the following:
    plane.obj
    energy_blob.zup.obj
into Analysis Sample Usage.

<h2> VISUAL STUDIO SOLUTION/PROJECT SETUP </h2>

In Visual Studio, go to File > Close Folder.
Then, under 'Get Started', click 'Create a new project'

On the left side, click 'Console App'.
On the right side, scroll until you find 'Empty Project'.
Double click 'Empty Project'

For project name, type 'HumanFactorsSamples'.
For location, navigate to Analysis Sample Usage.
Make sure that 'Place solution and project in the same directory' is checked.
Click 'Create'.

Right click 'HumanFactorsSamples'.
Select 'Properties'.
Under 'General Properties', select 'ISO C++17 Standard (std:c++17)'
for C++ Language Standard. Click 'Apply', then 'OK'.

In the dropdown where it says 'Debug', click it and go to
Configuration Manager.

Edit the Active solution configuration so we only have Release mode.
Edit the Active solution platform so we only have x64.

Right click 'Source Files' > Add > New Item.
'Select C++ File (.cpp)'.
For the name, type 'ProjectSetup.cpp'.

Enter the following:
```
#include <iostream>
int main(int argc, const char *argv[]) {
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
```
And hit F5 to verify that the project builds.

<h2> DRIVER CODE SETUP </h2>

At the top of HumanFactorsSamples.cpp,
be sure the following headers are included:

```
#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <Windows.h>
```

In the global namespace, add the following:

```
// Paths to DLLs
const wchar_t path_tbb[27] = L"..\\x64-Release\\bin\\tbb.dll";
const wchar_t path_embree3[31] = L"..\\x64-Release\\bin\\embree3.dll";
const wchar_t path_humanfactors[36] = L"..\\x64-Release\\bin\\HumanFactors.dll";

// Paths to .obj files
const std::string plane_path_str = "..\\plane.obj";
const std::string energy_blob_path_str = "..\\energy_blob_zup.obj";

// Subroutine that will use the HumanFactors DLL.
void HF_routine(HINSTANCE dll_hf).

You may rename HF_routine to a name that best suits your purpose.
```

In main(), we must load the following DLLs in this order:
```
tbb.dll
embree3.dll
HumanFactors.dll
```
If the DLLs are not loaded in this order, HumanFactors.dll will not load.

Load tbb.dll first.
```
HINSTANCE dll_tbb = LoadLibrary(path_tbb);

if (dll_tbb == nullptr) {
    std::cerr << "Unable to load " << "tbb.dll" << std::endl;
    exit(EXIT_FAILURE);
}
else {
    std::cout << "Loaded successfully: " << "tbb.dll" << std::endl;
}
```

Then load embree3.dll.
```
HINSTANCE dll_embree3 = LoadLibrary(CInterfaceTests::path_embree3);

if (dll_embree3 == nullptr) {
    std::cerr << "Unable to load " << "embree3.dll" << std::endl;

    FreeLibrary(dll_tbb);
    exit(EXIT_FAILURE);
}
else {
    std::cout << "Loaded successfully: " << "embree3.dll" << std::endl;
}
```

Then load HumanFactors.dll.
```
HINSTANCE dll_humanfactors = LoadLibrary(CInterfaceTests::path_humanfactors);

if (dll_humanfactors == nullptr) {
    std::cerr << "Unable to load " << "HumanFactors.dll" << std::endl;

    FreeLibrary(dll_embree3);
    FreeLibrary(dll_tbb);

    exit(EXIT_FAILURE);
}
else {
    std::cout << "Loaded successfully: " << "HumanFactors.dll" << std::endl;
}

Add a comment block that denotes where you will use the HumanFactors.dll.

```
//
// Ready to use dll_humanfactors here.
//
HF_routine(dll_humanfactors);
```

After using dll_humanfactors, we must free each library in reverse order
of initialization.

Before we do that, we must sleep the current thread for 250ms.
```
std::this_thread::sleep_for(std::chrono::milliseconds(250));
```

Now we can free each library:
```
if (FreeLibrary(dll_humanfactors)) {
    std::cout << "Freed successfully: " << "HumanFactors.dll" << std::endl;
}

if (FreeLibrary(dll_embree3)) {
    std::cout << "Freed successfully: " << "embree3.dll" << std::endl;
}

if (FreeLibrary(dll_tbb)) {
    std::cout << "Freed successfully: " << "tbb.dll" << std::endl;
}
```

You now have a visual studio project that is set up to use the HumanFactors DLL.
Each use-case example will demonstrate how to use dll_humanfactors to load
the functions required to do each example, but every use-case example will
use this sample project created here.
