import os
from os.path import dirname as up

# parent_dir = up(up(__file__))

directory = os.path.dirname(os.path.realpath(__file__)) + "\\bin"
os.add_dll_directory(directory)
