/*!
    \file		AnalysisCInterfaceSampleUsage.cpp
    \brief		Source file for testing functionality of (purely) the C Interface

    \author		Gem Aludino
    \date		31 Jul 2020
*/
#include <iostream>
#include <thread>
#include <Windows.h>

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

/*!
    \brief  Program execution begins and ends here.

    \param  argc    Argument count
    \param  argv    Command line arguments

    \return         0 on success, else failure.
*/
int main(int argc, const char *argv[]) {
    /*
        The following DLLs must be loaded in this order:
            - tbb.dll
            - embree3.dll
            - HumanFactors.dll

        If the DLLs are not loaded in this order,
        HumanFactors.dll will fail to load!
    */

    /*
        Load tbb.dll first.
    */
    HINSTANCE dll_tbb = LoadLibrary(L"..\\x64-Release\\bin\\tbb.dll");
    
    if (dll_tbb == nullptr) {
        std::cerr << "Unable to load " << "..\\x64-Release\\bin\\tbb.dll" << std::endl;
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "Loaded successfully: " << "..\\x64-Release\\bin\\tbb.dll" << std::endl;
    }

    /*
        embree3.dll depends on tbb.dll.
    */
    HINSTANCE dll_embree3 = LoadLibrary(L"..\\x64-Release\\bin\\embree3.dll");

    if (dll_embree3 == nullptr) {
        std::cerr << "Unable to load " << "..\\x64-Release\\bin\\embree3.dll" << std::endl;
        
        FreeLibrary(dll_tbb);
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "Loaded successfully: " << "..\\x64-Release\\bin\\embree3.dll" << std::endl;
    }

    /*
        HumanFactors.dll depends on both tbb.dll and embree3.dll.
    */
    HINSTANCE dll_humanfactors = LoadLibrary(L"..\\x64-Release\\bin\\HumanFactors.dll");

    if (dll_humanfactors == nullptr) {
        std::cerr << "Unable to load " << "..\\x64-Release\\bin\\HumanFactors.dll" << std::endl;

        FreeLibrary(dll_embree3);
        FreeLibrary(dll_tbb);

        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "Loaded successfully: " << "..\\x64-Release\\bin\\HumanFactors.dll" << std::endl;

        /*
            Run all tests here.
            Pass HINSTANCE dll_humanfactors to the test function.
            Each test function will have its own .cpp source file.

            End status of 1 means OK.
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

        ///
        /// Free libraries in order of creation
        ///

        /*
            When stepping through the debugger, the statement below is not required --
            but when running the executable, FreeLibrary(dll_humanfactors) throws an exception.
            By putting the current thread to sleep for 250 ms, dll_humanfactors can be freed.

            Solution was described here:
            https://forums.ni.com/t5/Instrument-Control-GPIB-Serial/Why-does-FreeLibrary-sometimes-crash/m-p/128079/highlight/true?profile.language=en#M7393
        */
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
    }

    return EXIT_SUCCESS;
}
