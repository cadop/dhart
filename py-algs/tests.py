import numpy as np
import matplotlib.pyplot as plt

def scale_range (input, min, max):
    input += -(np.min(input))
    input /= np.max(input) / (max - min)
    input += min
    return input

def min_test():

    scores = np.empty((3,4))
    for i in range(3):
        scores[i] = np.append(np.random.rand(3),(np.inf))

    print(scores)
    print(np.isfinite(scores))
    print(scores>0)
    bit_and = np.bitwise_and( np.isfinite(scores) , scores>0 )
    print( bit_and )
    scores = np.amin(scores, axis=0, where=bit_and, initial=9999)
    print(scores)


def run():

    # make set of values
    scores = list(range(1, 100))

    # make a curve
    curve = np.asarray(range(1,100))**2
    plt.plot(curve)
    plt.show()

    # natural log 
    curve = np.log(scores)
    plt.plot(curve)
    plt.show()

    # natural log scaled 1 to 10
    curve = scale_range(curve,1, 10)
    plt.plot(curve)
    plt.show()
    # weight values based on curve

    score2 = []
    for i in range(len(scores)):
        score2.append(scores[i]*curve[i])

    plt.plot(score2)
    plt.show()

    # Inverse scores and scale to max so there are no negatives
    scores = (-1*np.asarray(scores,dtype=np.float64)+(max(scores)))
    plt.plot(scores)
    plt.show()

    return 

# run()
min_test()