from setuptools import setup, find_packages
from glob import glob


setup(name='dhart',
      version='0.2.2',
      description='Utilities to perform tasks',
      author='',
      author_email='',
      packages=find_packages(),
      data_files = [
            ('bin', glob("dhart/bin/*")),
            # ('Lib/site-packages/dhart/bin', glob("dhart/bin/*")),
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