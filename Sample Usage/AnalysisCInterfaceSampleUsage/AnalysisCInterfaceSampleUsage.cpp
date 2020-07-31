/*!
    \file		AnalysisCInterfaceSampleUsage.cpp
    \brief		Source file for testing functionality of (purely) the C Interface

    \author		Gem Aludino
    \date		31 Jul 2020
*/
#include <iostream>

#include <Windows.h>

#ifndef C_INTERFACE
#define C_INTERFACE extern "C" __declspec(dllexport) int
#endif /* C_INTERFACE */

#define DLL_RELATIVE_PATH  ".\\x64-Release\\bin\\HumanFactors.dll"

/*
using HRESULT = (CALLBACK * LPFNDLLFUNC1)(DWORD, UINT*);

HRESULT LoadAndCallSomeFunction(DWORD dwParam1, UINT* puParam2);
*/

/*!
    \brief  Namespace for sample usage function prototypes

    \details
    All implementation for these sample usage functions
    will be defined in separate .cpp source files
    (one source file per function prototype/example)
*/
namespace CInterfaceTests {
    void raycasting(HINSTANCE dll_hf);
    /*
    // These prototypes be uncommented 
    // as source files for these examples are added.

    void generate_graph(void);
    void visualize_graph(void);
    void basic_search_graph(void);
    void visualize_path(void);
    void path_plan_costs(void);
    void calculating_spatial_view(void);
    void create_visibility_graph(void);
    */
}

int main(int argc, const char *argv[]) {
    /*
        The following DLLs must be loaded in this order:
            - tbb.dll
            - embree3.dll
            - HumanFactors.dll

        If the DLLs are not loaded in this order,
        HumanFactors.dll will fail to load!
    */

    HINSTANCE dll_tbb = LoadLibrary(L"..\\x64-Release\\bin\\tbb.dll");
    
    if (dll_tbb == nullptr) {
        std::cerr << "Unable to load " << "..\\x64-Release\\bin\\tbb.dll" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "Loaded successfully: " << "..\\x64-Release\\bin\\tbb.dll" << std::endl;
    }

    HINSTANCE dll_embree3 = LoadLibrary(L"..\\x64-Release\\bin\\embree3.dll");

    if (dll_embree3 == nullptr) {
        std::cerr << "Unable to load " << "..\\x64-Release\\bin\\embree3.dll" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "Loaded successfully: " << "..\\x64-Release\\bin\\embree3.dll" << std::endl;
    }

    HINSTANCE dll_humanfactors = LoadLibrary(L"..\\x64-Release\\bin\\HumanFactors.dll");

    if (dll_humanfactors == nullptr) {
        std::cerr << "Unable to load " << "..\\x64-Release\\bin\\HumanFactors.dll" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "Loaded successfully: " << "..\\x64-Release\\bin\\HumanFactors.dll" << std::endl;

        /*
            Run all tests here.
            Pass HINSTANCE dll_humanfactors to the test function.
            Each test function will have its own .cpp source file.
        */
        CInterfaceTests::raycasting(dll_humanfactors);
        /*
            // These function calls will be commented out 
            // as source files for these examples are added.
        
        CInterfaceTests::generate_graph(dll_humanfactors);
        CInterfaceTests::visualize_graph(dll_humanfactors);
        CInterfaceTests::basic_search_graph(dll_humanfactors);
        CInterfaceTests::visualize_path(dll_humanfactors);
        CInterfaceTests::path_plan_costs(dll_humanfactors);
        CInterfaceTests::calculating_spatial_view(dll_humanfactors);
        CInterfaceTests::create_visibility_graph(dll_humanfactors);
        */
    }


    /*
        Free all libraries in reverse order of creation
    */
    FreeLibrary(dll_humanfactors);
    FreeLibrary(dll_embree3);
    FreeLibrary(dll_tbb);
    
    return EXIT_SUCCESS;
}
