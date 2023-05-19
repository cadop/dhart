.. _user-install:

============
Installation
============

This guide will explain how to install the dhart package. Most of the explanation in Setup is how to 
create a virtual environment.  If you know how to do this, you can skip this and just use ``pip install .`` 
as the regular python package installation method. 

Check Basic Requirements
------------------------

Currently tested on ``Python 3.8+``.


If the dhart package is in the general repository, it will be located in `dhart/build/Python`. 
Navigate to this folder and the directory should look like: 

| Python
| ├── bin
| ├── dhart        
| ├── lib
| ├── README.md             
| ├── setup.py


Setup
-----

To install, open the windows `Command Prompt` by typing ``cmd`` into the windows search bar. 

It is recommended to use virtual environments, at least to start, to ensure the code is working. 
For this, you will navigate to some folder that you can consistently find and is not part of the 
dhart package. For example, make a folder called ``py_envs`` in your ``Documents`` folder. 

To setup a new virtual environment, go to the permanent folder location of your environments 
in the command prompt. For example:


``cd C:\Documents\py_envs``


Assuming you have python 3 installed with the python selector, you should be able to use ``py -3.8``. 
Otherwise, if python 3.8 is the only version installed, it should also be possible to create the environment 
with ``python``. Depending on your case, you will type:


``py -3.8 -m venv [preferred name]``


In the above line ``[preferred name]`` is really what you would like it to say. For this explanation, 
we will call the environment ``dhart_venv``, which means you type:


``py -3.8 -m venv dhart_venv``


It should take a second or two to finish. You will not see anything happen.  If you now type ``dir``, you should
see a folder called ``dhart_venv``.  Now you need to 'activate' the virtual environment by typing:


``dhart_venv\Scripts\activate``


Now you should see ``(dhart_venv)`` before the directory in the command prompt. This means you are successfully
in the virtual environment.  Anytime you want to reload the installed environment, you will navigate to the 
folder you made and type the above line for activating the environment.  If you want to end your virtual environment, 
simply type ``deactivate`` in the command prompt. 

Now that the virtual environment is activated, navigate to the dhart folder e.g., ``build/Python``. Make sure
you are in the correct location by typing ``dir`` to see the ``setup.py`` file. To install, type:

``pip install .``

If you are going to run examples and/or docs, use ``pip install .[dev]``.

To do a basic check that the package installed in the correct location, navigate to a different directory. 
For example:

``cd ../..``

Start a Python console with:

``python`` 

or if you have multiple versions:

``python3``

then enter:

``>>> import dhart``

``>>> dhart.get_sample_model()``

You should see the output being a path to your virtual environment folder:

``[your directory]\\lib\\site-packages\\dhart\\Example Models\\``

Where [your directory] would be ``C:\\Documents\\py_envs\\dhart_venv`` if following the directions in this document. 