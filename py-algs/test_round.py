import math
import numpy as np
from decimal import Decimal, ROUND_HALF_UP

def normal_round(n,d=0):
    n= n* (10**d)
    if n - math.floor(n) < 0.5:
        return math.floor(n) / (10**d)
    return math.ceil(n) / (10**d)

def trailing_round(x,d=0):
    s = str(x)
    if '.' not in s: return x
    s_int,s_float = s.split('.')
    i_len = len(s_int) # length of integer
    f_len = len(s_float) # length of float 
    precision = f_len - d # number of digits to remove from end
    for i in range(precision):
        x = normal_round(x,f_len-(i+1))
    return x

def trunc(x, p=7):
    s = str(x)
    s_len = len(s)-1 # length without decimal
    if s_len <=p: return x # if the number is within the allowed precision
    if '.' in s: x = float(s[0:p+1])
    else: x = int(s[0:p])
    return x

set1 = [737.0089 ,69.533935546875,667.474964453125,667.4789000000001,667.47]
set2 = [737.009 ,69.53399658203125,667.4750034179688,667.479,667.48]
set3 = [737.0091 ,69.53411865234375, 667.4749813476562, 667.4791, 667.47]
set4 = [737.0092, 69.53424072265625, 667.4749592773437, 667.4792, 667.47]

print( round(set1[2], 2) )
print( round(set2[2], 2) )

n1 = 0.474964453125
n2 = 0.4750034179688

print( np.around(n1,2) )
print( np.around(n2,2) )
print( round(n1,2) )
print( round(n2,2) )
print( float(0.474964453125) )
print( len(str(474964453125)) )
print( trailing_round(n1,2) )

print(trunc(set1[1]))
print(trunc(set2[1]))

print(trailing_round(625.5001,2))