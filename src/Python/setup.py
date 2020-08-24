from setuptools import setup, find_packages
from glob import glob

setup(name='humanfactors',
      version='0.2.2',
      description='Utilities to perform tasks',
      author='',
      author_email='',
      packages=find_packages(),
      data_files = [
            ('bin', glob("humanfactorspy/bin/*")),
            ('Lib/site-packages/humanfactorspy/Example Models', glob("humanfactorspy/Example Models/*"))
      ],
      install_requires=[
            'numpy',
            'scipy',
            'matplotlib',
      ],
      zip_safe=True,
      include_package_data=True
)