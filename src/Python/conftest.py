# This file is found by pytest and stops matplotlib from showing plots when running 

# conftest.py
import matplotlib
matplotlib.use('Agg')  # Force headless backend for all tests
