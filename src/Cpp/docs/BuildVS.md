<h1> Building with Microsoft Visual Studio 2019</h1>

<h3>0. Opening Visual Studio</h3>
Open <b>Microsoft Visual Studio 2019</b>. After seeing the splash/welcome screen,<br>
please click the button that reads <b>Open a local folder</b>.

<h3>1. Opening the dhart repository</h3>
When the <b>Browse</b> window appears, navigate to the <code><b>dhart</b></code> folder<br>
(the repository that you had cloned) on your local machine.<br>
Then, navigate to <code><b>src</b>.</code><br>
Click the <b>Select</b> folder button to confirm.<br>

<h3>2. Revealing the Solution Explorer</h3>
If the <b>Solution Explorer</b> view is not already open, you may open by<br>
navigating to <b>View > Solution Explorer</b>, or alternatively,<br>
you can also use the <b>Ctrl + Alt + L</b> shortcut to reveal Solution Explorer.<br>
Here, you can examine the sources imported by Visual Studio.<br>

<h3>3. Choosing a build version</h3>
There are <b>two</b> build versions for <b>dhart</b>:
- x64-Debug<br>
- x64-Release<br>

To change between build versions,<br>
navigate (move your cursor) over to the top-left portion of Visual Studio,<br>
and you will see a drop-down bar -- <b>x64-Debug</b> is selected by default.

If you want to build the <b>x64-Release</b> version of <b>dhart</b>,<br>
change the option in the drop-down bar to <b>x64-Release</b>.

<h3>4. Building the repository</h3>
<b>We are now ready to build dhart.</b><br>
<b>CMake</b> is used to aid in the compilation process.<br>

To build, navigate to <b>Build > Build All</b>,<br>
or you may use the <b>Ctrl + Shift + B</b> keyboard shortcut.<br>

<b>You have now built the sources for dhart.</b><br>
You can find the build files in <code><b>./dhart/src/out/build/x64-Debug</b></code>