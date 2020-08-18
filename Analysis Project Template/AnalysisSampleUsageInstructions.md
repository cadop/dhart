<h2> Introduction </h2>

This folder contains a sample HumanFactors project template,
with a preconfigured Visual Studio solution (.sln) file and a Visual Studio project (.vcxproj) file. This project template will be used by every video tutorial/use case example, so that folder/project setup need not be endured by the end user.

The Visual Studio project contains a source file with logic that will load the HumanFactors DLL (and its required dependencies). This is required before any functions can be called from the HumanFactors DLL.<br>

The DLLs are included with this directory (in x64-Release/bin), along with sample .obj files that will be used by the examples to be demonstrated.

If the end user chooses to create their own Human Factors solution/project,<br>
without the use of this project template, the steps have been outlined below<br> 
as to how this project folder structure was created.

<h2> Adding the project template to Visual Studio </h2>

Begin by opening <code>HumanFactorsSamples.sln</code> in<br> 
<code>Analysis\Analysis Project Template\HumanFactorsSamples\ </code>

After opening the solution -- at the <b>top</b> of Visual Studio, navigate to<br>
<b>Project > Export Template</b>

- Ensure that the '<b>Project Template</b>' radio button is selected.<br>
- Ensure that '<b>HumanFactorsSamples</b>' is selected for<br>
- '<b>From which project would you like to create a template?</b>'

Provide a <b>description</b> for '<b>Template description</b>'.<br>
Then, click '<b>Finish</b>'.

The template will be saved to:<br>
<code>[user folder]\Documents\Visual Studio 2019\My Exported Templates</code>

Whenever you want to create a <b>new Visual Studio project</b> with the HumanFactors DLL,<br> the code provided will <b>already be defined to load the required DLLs</b>.<br>
Note that the project will assume that the DLLs are in<br>
<code>Analysis Project Template\x64-Release\bin</code>.<br>

The <b>same is true for the <code>.obj</code> files</b> --<br>
the project assumes that the <code>.obj</code> files are in the directory<br>
<code>Analysis Project Template</code>.

<h2> DIRECTORY SETUP/INSTALL HUMAN FACTORS </h2>

<b> Note that the steps below are provided for reference only.<br>
<br>
If you choose to use the project template,<br>
the steps below have already been completed for you,<br>
-- you may skip the rest of this document if that is the case.</b>

Create a folder named '<b>Analysis Project Template</b>' in your development directory of choice.

Launch <b>Visual Studio 2019</b> and open the <b>Analysis</b> repository.
(<code>Analysis/src</code>). This will use the <code>CMakeLists.txt</code> file to work with the codebase within Visual Studio. (as opposed to a <code>.sln</code>).

At the top of Visual Studio, click <b>Build > Install Human Factors</b>.<br>
This will install Human Factors in <code>Analysis\src\out\install</code>.

After installing Human Factors, navigate to <code>Analysis\src\out\install</code> using the <b>File Explorer</b>.

Copy the <b>x64-Release</b> folder into the '<b>Analysis Project Template</b>' folder you made earlier.

In <code>Analysis Project Template\x64-Release</code>, keep only the following files,

```
bin\embree3.dll
bin\HumanFactors.dll
bin\tbb.dll
```

<b>Delete everything else.</b> They will not be used.

In <code>Analysis\src\Cpp\tests\Example Models</code>, copy the following<br>
into <code>Analysis Sample Usage</code>:
```
plane.obj
energy_blob_zup.obj
```

<h2> VISUAL STUDIO SOLUTION/PROJECT SETUP </h2>

At the top of Visual Studio, go to <b>File > Close Folder</b>.<br>
Then, under '<b>Get Started</b>', click '<b>Create a new project</b>'

On the left side, click '<b>Console App</b>'.<br>
On the right side, scroll until you find '<b>Empty Project</b>'.
Double click '<b>Empty Project</b>'

For project name, type '<b>HumanFactorsSamples</b>'.<br>
For location, navigate to <code>Analysis Sample Usage</code>.<br>
Make sure that '<b>Place solution and project in the same directory</b>' is checked.
Click '<b>Create</b>'.

- Right click '<b>HumanFactorsSamples</b>'.
- Select '<b>Properties</b>'.
- Under '<b>General Properties</b>', select '<b>ISO C++17 Standard (std:c++17)</b>' for C++ Language Standard. 
- Click '<b>Apply</b>', then '<b>OK</b>'.

Next, at the top of Visual Studio -- there is dropdown that says '<b>Debug</b>';<br> click it and go to <b>Configuration Manager</b>.

Edit the <b>Active solution configuration</b> so we only have <b>Release</b> mode.
Edit the <b>Active solution platform</b> so we only have <b>x64</b>.

Right click <b>'Source Files' > Add > New Item.</b><br>
Click '<b>Select C++ File (.cpp)</b>'. For the name, type '<b>HumanFactorsSamples.cpp</b>'.

In <code>HumanFactorsSamples.cpp</code>, enter the following:

```
#include <iostream>
int main(int argc, const char *argv[]) {
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
```
And hit <b>F5</b> to verify that the project builds.

<h2> SOURCE FILE SETUP </h2>

Now that the Visual Studio <code>.sln</code> and <code>.vcxproj</code><br>
are set up, we can work on <code>HumanFactorsSamples.cpp</code>.<br>

The goal is to add the logic necessary<br>
to load the HumanFactors DLL and its dependencies.<br>

At the top of <code>HumanFactorsSamples.cpp</code>,
be sure the following headers are included:

```
#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <Windows.h>
```

In the <b>global namespace</b>, add the following:

```
// Paths to DLLs
const wchar_t path_tbb[27] = L"..\\x64-Release\\bin\\tbb.dll";
const wchar_t path_embree3[31] = L"..\\x64-Release\\bin\\embree3.dll";
const wchar_t path_humanfactors[36] = L"..\\x64-Release\\bin\\HumanFactors.dll";

// Paths to .obj files
const std::string plane_path_str = "..\\plane.obj";
const std::string energy_blob_path_str = "..\\energy_blob_zup.obj";

// Subroutine that will use the HumanFactors DLL.
void HF_routine(HINSTANCE dll_hf) { }

You may rename HF_routine to a name that best suits your purpose/example.
```

Now, clear out the <code>main()</code> function.

In <code>main()</code>, we must <b>load the following DLLs in this order</b>:

```
tbb.dll
embree3.dll
HumanFactors.dll
```

<b>If the DLLs are not loaded in this order, HumanFactors.dll will not load.</b>

Load <code>tbb.dll</code> first.

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

Then load <code>embree3.dll</code>.
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

Then load <code>HumanFactors.dll</code>.

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
```

Your '<code>HF_routine</code>' function (or whatever you had named this function)<br>
is now ready to be called.

```
//
// Ready to use dll_humanfactors here.
//
HF_routine(dll_humanfactors);
```

After using <code>dll_humanfactors</code>,<br>
<b>we must free each library in reverse order of initialization.</b>

Before we do that, we must sleep the current thread for 250ms.

```
std::this_thread::sleep_for(std::chrono::milliseconds(250));
```

Now we can <b>free each library</b>:
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

return EXIT_SUCCESS;
```

Here is what <b>HumanFactorsSamples.cpp</b> should look like:<br>
(comments have been abridged for brevity)<br>

```
#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <Windows.h>

// Paths to DLLs
const wchar_t path_tbb[27] = L"..\\x64-Release\\bin\\tbb.dll";
const wchar_t path_embree3[31] = L"..\\x64-Release\\bin\\embree3.dll";
const wchar_t path_humanfactors[36] = L"..\\x64-Release\\bin\\HumanFactors.dll";

// Paths to .obj files used by examples
const std::string plane_path_str = "..\\plane.obj";
const std::string energy_blob_path_str = "..\\energy_blob_zup.obj";

/*!
	\brief	Use case example code that uses HumanFactors DLL. All examples should begin here.

	\param	dll_hf	Loaded HumanFactors DLL from which all function pointers will be loaded
*/
void HF_routine(HINSTANCE dll_hf) {
	//
	// HumanFactors example code goes here.
	//
}

/*!
	\brief	Program execution begins and ends here.

	\param	argc	Command line argument count
	\param	argv	Command line arguments

	\returns		0 on success, else failure (see error code)
*/
int main(int argc, const char* argv[]) {
	/*
		The following DLLs must be loaded in this order:
			- tbb.dll
			- embree3.dll
			- HumanFactors.dll

		If the DLLs are not loaded in this order,
		HumanFactors.dll will fail to load!
	*/

	// Load tbb.dll first.
	HINSTANCE dll_tbb = LoadLibrary(path_tbb);

	if (dll_tbb == nullptr) {
		std::cerr << "Unable to load " << "tbb.dll" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "Loaded successfully: " << "tbb.dll" << std::endl;
	}

	// embree3.dll depends on tbb.dll.
	HINSTANCE dll_embree3 = LoadLibrary(path_embree3);

	if (dll_embree3 == nullptr) {
		std::cerr << "Unable to load " << "embree3.dll" << std::endl;

		FreeLibrary(dll_tbb);
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "Loaded successfully: " << "embree3.dll" << std::endl;
	}
    
	// HumanFactors.dll depends on both tbb.dll and embree3.dll.
	HINSTANCE dll_humanfactors = LoadLibrary(path_humanfactors);

	if (dll_humanfactors == nullptr) {
		std::cerr << "Unable to load " << "HumanFactors.dll" << std::endl;

		FreeLibrary(dll_embree3);
		FreeLibrary(dll_tbb);

		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "Loaded successfully: " << "HumanFactors.dll" << std::endl;
	}

	//
	// Ready to use dll_humanfactors here.
	//
	HF_routine(dll_humanfactors);

	//
	// Free libraries in reverse order of creation.
	//
	std::this_thread::sleep_for(std::chrono::milliseconds(250));

	if (FreeLibrary(dll_humanfactors)) {
		std::cout << "Freed successfully: " << "HumanFactors.dll" << std::endl;
	}

	if (FreeLibrary(dll_embree3)) {
		std::cout << "Freed successfully: " << "embree3.dll" << std::endl;
	}

	if (FreeLibrary(dll_tbb)) {
		std::cout << "Freed successfully: " << "tbb.dll" << std::endl;
	}

	return EXIT_SUCCESS;
}
```

<b>You now have a Visual Studio solution/project that is set up to use the HumanFactors DLL.</b><br>

Each use-case example will demonstrate how to use <code>dll_humanfactors</code> to load
the functions required to do each example, but every use-case example will begin with the sample project template created here.

<h2> Loading functions from the HumanFactors DLL </h2>

Within <code>HumanFactorsSamples.cpp</code>, the function <code>HF_routine</code> is where you will use the loaded Human Factors DLL. You will load the functions you require from the DLL.<br>

In order to use the HumanFactors DLL,<br>
you must take stock of the following:
- All data types required for an example (<code>struct</code>/<code>enum</code>/<code>class</code>)
- Must the types be fully defined, or will a forward-declaration suffice?
- Required C Interface functions, and their prototypes

If you want access to the members of an <code>enum</code> or <code>struct</code>,<br>
you must define the required types before using the Human Factors DLL.<br>
Otherwise, a forward-declaration will work.

To load functions from the HumanFactors DLL,<br>
you will use the function <code>GetProcAddress</code>, defined in <code>Windows.h</code>.

Follow these steps for every example:<br>
- Determine what functions/types are required for your example
- Forward declare or completely define all required types for your example
- Create <code>typedef</code> statements for all functions that are required.
- Retrieve the function pointers using <code>GetProcAddress</code>
- Call the functions you have loaded

Here is a sample, loading <code>LoadOBJ</code> and <code>CreateRaytracer</code>:

```
// ... globals of DLL path strings, .obj path strings

namespace HF::Geometry {
    class MeshInfo;
}

namespace HF::Raytracer {
    class EmbreeRayTracer;
}

void HF_routine(HINSTANCE dll_hf) {
	// Create typedefs
	typedef int (*p_LoadOBJ)(const char *, int, float, float, float, std::vector<HF::Geometry::MeshInfo> **);

	typedef int (*p_CreateRaytracer)(std::vector<HF::Geometry::MeshInfo>*, HF::RayTracer::EmbreeRayTracer**);

	// Obtain functions
	auto LoadOBJ = reinterpret_cast<p_LoadOBJ>(GetProcAddress(dll_hf, "LoadOBJ"));
	auto CreateRaytracer = reinterpret_cast<p_CreateRaytracer>(GetProcAddress(dll_hf, "CreateRaytracer"));

	// Now you can call LoadOBJ and CreateRaytracer.
}

int main(int argc, const char *argv[]) {
    // Load DLLs

    HF_routine(dll_humanfactors);

    // Free DLL
}

```