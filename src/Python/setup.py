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
            # ('Lib/site-packages/humanfactorspy/bin', glob("humanfactorspy/bin/*")),
            ('Lib/site-packages/humanfactorspy/Example Models', glob("humanfactorspy/Example Models/*")),
            ('Lib/site-packages/humanfactorspy/Examples/Rhino', glob("humanfactorspy/Examples/*/*")),
            ('Lib/site-packages/humanfactorspy/Examples', [x for x in glob("humanfactorspy/Examples/*") if 'Rhino' not in x])
      ],
      install_requires=[
            'numpy',
            'scipy',
            'matplotlib',
      ],
      zip_safe=True,
      include_package_data=True
)