## Building Python docs in terminal

Navigate to dhart\src, then run: 


`cmake ./src/ -G"Visual Studio 16 2019" -DCMAKE_GENERATOR_PLATFORM="x64"  -DCMAKE_CONFIGURATION_TYPES="Debug" -DCMAKE_INSTALL_PREFIX=".\..\build\Python" -DDHARTAPI_Config="All" -DDHARTAPI_EnableTests="False" -DCMAKE_BUILD_TYPE="Debug" -DDHARTAPI_EnablePython="True" -DDHARTAPI_EnableCSharp="False" -DINSTALL_GTEST="False"  ".\" 2>&1 `

`cmake --build ../src`

`cmake --install .`


Then cd to dhart\build\Python

`pip install .`

Then cd to dhart\docs\Python Docs

`.\make clean`

`.\make html`