Building Python docs on commandline only

Navigate to dhart\src, then run: 


cmake ./src/ -G"Visual Studio 16 2019" -DCMAKE_GENERATOR_PLATFORM="x64"  -DCMAKE_CONFIGURATION_TYPES="Debug" -DCMAKE_INSTALL_PREFIX=".\dhart\src\..\build\Python" -DHumanFactors_Config="All" -DHumanFactors_EnableTests="False" -DCMAKE_BUILD_TYPE="Debug" -DHumanFactors_EnablePython="True" -DHumanFactors_EnableCSharp="False" -DINSTALL_GTEST="False"  "E:\ProjectsRepo\dhart\src" 2>&1

cmake --build ../src

cmake --install .


Then cd to dhart\build\Python

pip install .

Then cd to dhart\docs\Python Docs

.\make html