**Doxygen Guidelines**

This document explains the formatting and style for documenting code. To build the documentation,
 see [Building Documentation](DoxygenInstructions.md) for information on documenting and building documentation.

<b>Overview</b><br>
* Methods and xml format to use in methods
* Algorithms should be documented inline with source rather than headers
* Interface details should remain with headers

<b>Table of contents</b><br>
- Using TODO
- Documenting the heads of each file
- Documenting member fields
- Snippets and Code Samples
- Adding images for headers/sources, and markdown files (`.md`)

<h2>Using `TODO`</h2>
To have a TODO automatically pull to documentation use:<br>
`/// TODO:`

<h2>Documenting the heads of each file (headers and sources)</h2>

<b>The head of each file should contain the following tags:</b><br>

`\file`<br>
Name of a file<br>
e.g. <b>`/// \file     node.h`</b><br>
https://www.doxygen.nl/manual/commands.html#cmdfile

`\brief`<br>
A one-line description describing the contents of the file<br>
e.g. <b>`/// \brief    Header file for a node data structure, used in a graph ADT`</b><br>
https://www.doxygen.nl/manual/commands.html#cmdbrief

`\author`<br>
The author of the file<br>
If there are multiple authors,<br>
you may use one `\author` tag for all of the authors,<br>
or one `\author` tag per person/entity.<br>
e.g. <b>`/// \author   John Doe`</b><br>
https://www.doxygen.nl/manual/commands.html#cmdauthor

`\date`<br>
This may be the date of the file's last update,<br>
or the date the file was created -- whichever is relevant.<br>
e.g. <b>`/// \date   06 Jun 2020`</b><br>
https://www.doxygen.nl/manual/commands.html#cmddate

An example of a file head would look like this:

    ///
    /// \file       node.h
    /// \brief      Header file for a node data structure, used in a graph ADT
    ///
    /// \author     John Doe
    /// \date       06 Jun 2020
    ///

Other notes:<br>
- Follow each `///` with a <b>single</b> space, before using a tag.
- Follow each tag with <b>two</b> tabs, then provide the description.

<h2>Documenting member fields</h2>

Here is a simple example:<br>
    struct position {
        float x, y, z;  ///< Cartesian coordinates x, y, and z
        int id;         ///< Unique identifier
    };

The general format is `///< Description goes here`,<br>
notice that a <b>single whitespace character</b> follows `///<`<br>
and the field descriptions are in alignment with each other, using tabs.<br>
Sometimes, this is not always possible,<br>
but do your best to keep the formatting <b>neat</b> and <b>consistent</b>.

<h2>Snippets and code samples</h2>

<h3>The `\snippet` tag should be used for long examples that are over 10 lines.</h3>
https://www.doxygen.nl/manual/commands.html#cmdsnippet

<b>Step 0:</b><br>
In a header file, at the line where you want the snippet to appear:

    /// \snippet PATH_TO_FILE\filename.cpp SnippetName

If you would like to include line numbers for the snippet, you may use the tag
`\snippet{lineno}`
This is best used for directly referencing source files from within this repo,
since the line numbers that appear are the actual line numbers of the code used
for the snippet.

<b>Step 1:</b><br>
In `PATH_TO_FILE\filename.cpp` (where the snippet sample is),

    /// [SnippetName]
    std::cout << "My sample snippet" << std::endl;
    /// [SnippetName]


Notice that the code is not a comment.

