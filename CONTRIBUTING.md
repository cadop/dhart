**Documentation**
Documentation is vital and must be done throughout the project, not the end. This includes examples for the use of each function

*Non-unity users*

Python
* Sphinx
* Numpydocs formatting
* Must comply with doctest
* Must include examples
C++ 
* Doxygen
* Must have a getting started page for each module
* links to C# and Python interfaces for each function, module


Unity users:

* Natural Docs by Greg Valure
* Installing Natural Docs on Windows
* Download the Windows version of Natural Docs from it’s website
*  Natural Docs for Unity plugin (https://unitysquid.com/create-c-documentation-for-unity-like-a-pro/)
	
There are three users of the codebase. People that will develop the foundation of the code (includes people like you). People that will use the interfaces and our codebase as an API. People that will use final products that are built with the codebase.  Each user group must be considered at each stage. 

Coding standards are important, sometimes there is the ‘industry standard’ way. This is not always in alignment with the needs of the project or with research.  It’s likely some things will be different, it should always be discussed and clarified. It should also be in the documentation. 

**Python**

* pep8 convention in most cases (ask otherwise)
* Do not make everything a class. A class should only be used when it is very clearly necessary. Algorithms should be done in a functional way. 
* Do not add dependencies without making an issue on why you need it (this goes for libraries that are not already listed as dependencies for the code)

**C#**

* Naming Conventions and Coding Standards have to be well followed (https://sites.google.com/site/wcfpandu/student-of-the-month/c-coding-standards-and-naming-conventions)
* Code general architecture should be discussed and well maintained.
