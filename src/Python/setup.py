from setuptools import setup, find_packages
from glob import glob

with open("README.md", "r") as fh:
    long_description = fh.read()

setup(name='dhart',
      version='0.2.6',
      description='Design Humans Analysis RoboTics',
      long_description=long_description,
      long_description_content_type="text/markdown",
      url="https://github.com/cadop/dhart",
      author='',
      author_email='',
      packages=find_packages(),
      classifiers=[
            "Programming Language :: Python :: 3",
            "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
            "Operating System :: Microsoft :: Windows :: Windows 10",
      ],
      python_requires='>=3.6',
      data_files = [
            ('bin', glob("dhart/bin/*")),
            ('Lib/site-packages/dhart/Example Models', glob("dhart/Example Models/*")),
            ('Lib/site-packages/dhart/Examples/Rhino', glob("dhart/Examples/*/*")),
            ('Lib/site-packages/dhart/Examples', [x for x in glob("dhart/Examples/*") if 'Rhino' not in x])
      ],
      install_requires=[
            'numpy',
            'scipy',
            'matplotlib',
      ],
      zip_safe=True,
      include_package_data=True
)
