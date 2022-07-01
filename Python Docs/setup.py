from setuptools import setup, find_packages

setup(name='dhart',
      version='0.2.2',
      description='Utilities to perform tasks',
      author='',
      author_email='',
      packages=find_packages(),
      data_files = [
            ('bin', ["dhart/bin/HumanFactors.dll", "dhart/bin/tbb.dll", "dhart/bin/embree3.dll"])
      ],
      install_requires=[
            'numpy',
            'scipy',
            'matplotlib',
      ],
      zip_safe=True,
      include_package_data=True
)