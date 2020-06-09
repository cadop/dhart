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



C++ Doxygen Guidelines

Overview

Methods and xml format to use in methods
Algorithms should be documented inline with source rather than headers
Interface details should remain with headers

Implementation

To have a TODO automatically pull to documentation use: <code>///TODO:</code>

<h1>Snippets and code samples</h1>

<b>Snippets should be used for long examples that are over 10 lines.</b>

<b>Step 0:</b><br>
In a header file, at the line where you want the snippet to appear:

<code>/// \snippet PATH_TO_FILE\filename.cpp SnippetName</code>

If you would like to include line numbers for the snippet, you may use the tag
<code>\snippet{lineno}</code>
This is best used for directly referencing source files from within this repo,
since the line numbers that appear are the actual line numbers of the code used
for the snippet.

<b>Step 1:</b><br>
In <code>PATH_TO_FILE\filename.cpp</code> (where the snippet sample is),

<code>/// [SnippetName]
std::cout << "My sample snippet" << std::endl;
/// [SnippetName]
</code>

Notice that the code is not a comment.

<b>Step 2:</b><br>
The following steps utilize the Doxygen GUI Frontend.

Load the Doxyfile from this repo into Doxygen, using the File menu.
The Doxyfile is in <code>Analysis\src\Doxyfile</code>

<b>Step 3:</b><br>
- Navigate to the Expert tab on the left hand side.
- Under topics, click 'Input'.
- On the right side, scroll down until you see EXAMPLE_PATH.
- In the EXAMPLE_PATH section, click the folder button to reveal a file prompt.
- Within the file prompt, select the file path where the snippet resides. (<code>PATH_TO_FILE\filename.cpp</code>)
- Add any more directories/paths where your snippets will come from.

Snippets will now appear when you navigate to the Run tab, and click 'Run Doxygen'.

<h2>For code block examples under 10 lines, use the <code>\code</code> tag.</h2>

To use the <code>\code</code> tag:

<b>Step 0:</b><br>
In a header file, at the line where you want the code sample to appear,
start with the <code>\code{.cpp}</code> tag.

<code>/// \code{.cpp}</code>

<b>Step 1:</b><br>
Create the comment block where the code example will appear.
We will put the code block within this comment block.

<code>/*!
std::cout << "Your sample code goes here." << std::endl;
*/
</code>

<b>Step 2:</b><br>
End your code sample with the tag <code>\endcode</code>

<code>/// \endcode
</code>

In total, a code sample would look like this:

<code>/// \code{.cpp}
/*!
std::cout << "Your sample code goes here." << std::endl;
*/
/// endcode
</code>

You may now use 'Run Doxygen' to export your documentation.
(There are no extra steps, unlike with the snippet section)
