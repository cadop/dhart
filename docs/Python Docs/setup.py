from setuptools import setup, find_packages

setup(name='humanfactors',
      version='0.2.2',
      description='Utilities to perform tasks',
      author='',
      author_email='',
      packages=find_packages(),
      data_files = [
            ('bin', ["humanfactorspy/bin/HumanFactors.dll", "humanfactorspy/bin/tbb.dll", "humanfactorspy/bin/embree3.dll"])
      ],
      install_requires=[
            'numpy',
            'scipy'
      ],
      zip_safe=True,
      include_package_data=True
)