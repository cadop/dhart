**Documentation**
Documentation is vital and must be done throughout the project, not the end. This includes examples for the use of each function

*Non-unity users*

<h1>Python</h1>
* Sphinx
* Numpydocs formatting
* Must comply with doctest
* Must include examples

<h1>C++</h1>
* Doxygen
* Must have a getting started page for each module
* links to C# and Python interfaces for each function, module

<h1>Unity users</h1>
* Natural Docs by Greg Valure
* Installing Natural Docs on Windows
* Download the Windows version of Natural Docs from it’s website
* Natural Docs for Unity plugin (https://unitysquid.com/create-c-documentation-for-unity-like-a-pro/)
	
There are three users of the codebase. 
- People that will develop the foundation of the code (includes people like you). 
- People that will use the interfaces and our codebase as an API. 
- People that will use final products that are built with the codebase.  
Each user group must be considered at each stage. 

Coding standards are important, sometimes there is the ‘industry standard’ way.<br>
This is not always in alignment with the needs of the project or with research.<br>
It’s likely some things will be different, it should always be discussed and clarified.<br>
It should also be in the documentation. 

<h1>Python</h1>
* pep8 convention in most cases (ask otherwise)
* Do not make everything a class. A class should only be used when it is very clearly necessary. Algorithms should be done in a functional way. 
* Do not add dependencies without making an issue on why you need it (this goes for libraries that are not already listed as dependencies for the code)

<h1>C#</h1>
* Naming Conventions and Coding Standards have to be well followed (https://sites.google.com/site/wcfpandu/student-of-the-month/c-coding-standards-and-naming-conventions)
* Code general architecture should be discussed and well maintained.

<h1>C++ Doxygen Guidelines</h1>
<b>Overview</b><br>
* Methods and xml format to use in methods
* Algorithms should be documented inline with source rather than headers
* Interface details should remain with headers

<b>Table of contents</b><br>
- Using TODO
- Documenting the heads of each file
- Documenting member fields
- Snippets and Code Samples
- Why the use of <code>///</code> and <code>//</code>?

<h2>Using <code>TODO</code></h2>
To have a TODO automatically pull to documentation use:<br>
<code>/// TODO:</code>

<h2>Documenting the heads of each file (headers and sources)</h2>

<b>The head of each file should contain the following tags:</b><br>

<code>\file</code><br>
Name of a file<br>
e.g. <b><code>/// \file     node.h</code></b><br>
https://www.doxygen.nl/manual/commands.html#cmdfile

<code>\brief</code><br>
A one-line description describing the contents of the file<br>
e.g. <b><code>/// \brief    Header file for a node data structure, used in a graph ADT</code></b><br>
https://www.doxygen.nl/manual/commands.html#cmdbrief

<code>\author</code><br>
The author of the file<br>
If there are multiple authors,<br>
you may use one <code>\author</code> tag for all of the authors,<br>
or one <code>\author</code> tag per person/entity.<br>
e.g. <b><code>/// \author   John Doe</code></b><br>
https://www.doxygen.nl/manual/commands.html#cmdauthor

<code>\date</code><br>
This may be the date of the file's last update,<br>
or the date the file was created -- whichever is relevant.<br>
e.g. <b><code>/// \date   06 Jun 2020</code></b><br>
https://www.doxygen.nl/manual/commands.html#cmddate

An example of a file head would look like this:

<code>///
/// \file       node.h
/// \brief      Header file for a node data structure, used in a graph ADT
///
/// \author     John Doe
/// \date       06 Jun 2020
///</code>

Other notes:<b>
- Follow each <code>///</code> with a <b>single</b> space, before using a tag.
- Follow each tag with <b>two</b> tabs, then provide the description.

<h2>Snippets and code samples</h2>

<h3>The <code>\snippet</code> tag should be used for long examples that are over 10 lines.</h3>
https://www.doxygen.nl/manual/commands.html#cmdsnippet

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

<h3>The <code>\code</code> tag should be used for code block examples under 10 lines.</h3>
https://www.doxygen.nl/manual/commands.html#cmdcode

To use the <code>\code</code> tag:

<b>Step 0:</b><br>
In a header file, at the line where you want the code sample to appear,
start with the <code>\code{.cpp}</code> tag.

<code>/// \code{.cpp}</code>

<b>Step 1:</b><br>
Create the section where the code sample will appear.
We will be using the <code>///</code> comment style.

<code>/// std::cout << "Your sample code goes here." << std::endl;
/// std::cout << "Notice the whitespace after the '///'. << std::endl;
/// // Use '//' for comments within the sample.</code><br>

To be clear:
- <b>All</b> lines in the sample must begin with <code>///</code>, followed by a whitespace
- Any comments that occur within the sample must begin with <code>//</code>, followed by a whitespace.

Our convention dictates that a single character of whitespace ('<code> </code>')
<br> <b>must</b> follow a <code>///</code> or <code>//</code>, whenever:
- a <code>\code</code> tag is initiated
- a sample line of code begins
- a comment begins within the sample code
- a <code>\endcode</code> tag is used to end a sample 

<br>This ensures ease of viewing<br>
for anyone reading the source code -- <br>
and also assists Doxygen with the<br>
formatting process for the HTML export.<br>

For more information, see<br> 
'Why the use of <code>///</code> and <code>//</code>?'<br>
at the bottom of this page.<br>

<b>Step 2:</b><br>
End your code sample with the tag <code>\endcode</code>

<code>/// \endcode</code>

A properly-formatted sample will look like this:

<code>/// \code{.cpp}
/// std::cout << "Your sample code goes here." << std::endl;
/// std::cout << "Notice the whitespace after the '///'. << std::endl;
/// // Use '//' for comments within the sample.
/// \endcode</code><br>

In the final HTML, your sample would look like this:

<code>std::cout << "Your sample code goes here." << std::endl;
std::cout << "Notice the whitespace after the '///'. << std::endl;
// Use '//' for comments within the sample.</code>

You may now use <b>'Run Doxygen'</b> to export your documentation.
(There are no extra steps, unlike with the snippet section)

<h2>Why the use of <code>///</code> and <code>//</code>?</h2>

It may seem pedantic to insist on using <code>///</code> for the entire code sample,
<br> and <code>//</code> for comments within the sample -- but there is a reason
for this.<br>

As you may know, there are many ways to invoke a comment in C++,<br>
particularly for use with Doxygen -- such as:<br><br>
<code>/* A C-style comment */</code><br>
<code>/*! Qt variant of a C-style comment */</code><br>
<code>// C++ style double-slash comment</code><br>
<code>/// C++ style triple-slash comment</code><br>

You can read more about the specifics of Doxygen-legal comment styling here:
https://www.doxygen.nl/manual/docblocks.html

<b>The use of <code>///</code> for the entirety of the code sample,<br>
because it permits the use of <code>//</code> within the code sample whenever
necessary.</b>This is important for including comments within your code sample.<b>

If, for example, your code sample were to look like this:<br>

<code>/// \code{.cpp}
/*!
std::cout << "Here is my sample" << std::endl;
// Here is a comment
*/</code>

...the final Doxygen HTML export would look like this:<br>

<code>
@verbatim 
        std::cout << "Here is my sample" << std::endl;
@endverbatim
Here is a comment</code>

It seems that Doxygen will have issues with differentiating comments<br>
within the code sample, with a comment that would end the code sample.<br>

It is unclear as to why the <code>@verbatim</code> and <code>@endverbatim</code>
tags appear in place of <code>/*!</code> and <code>/*</code> respectively --<br>
but if the convention described above is used, particularly if comments are to<br>
be included in the code sample, there will be no issues.<br>
