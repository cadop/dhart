import cProfile
import pstats


cProfile.run("import GenerateRandomPathsExample", "out_file.txt")

p = pstats.Stats("out_file.txt")
p.strip_dirs().sort_stats(pstats.SortKey.TIME).print_stats(10)