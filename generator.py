import random
import numpy as np
import sys
import time
from math import sqrt

vector_size = 5
v1_file = "vectors_1.txt"
v2_file = "vectors_2.txt"

v1_size = random.randint(1, 10000)
v2_size = random.randint(1, 10000)

print("Sizes: V1: {} V2: {}".format(v1_size, v2_size))

v1_vectors = []
v2_vectors = []

for i in range(v1_size):
    v1_vectors.append(np.random.rand(1, vector_size)[0])

for i in range(v2_size):
    v2_vectors.append(np.random.rand(1, vector_size)[0])

print("Writing V1 vectors")

with open(v1_file, "w") as f:
    for i in range(v1_size):
        for j in range(vector_size):
            f.write(str(v1_vectors[i][j]))
            if j != vector_size-1:
                f.write(",")
            else:
                f.write("\n")

print("Writing V2 vectors")

with open(v2_file, "w") as f:
    for i in range(v2_size):
        for j in range(vector_size):
            f.write(str(v2_vectors[i][j]))
            if j != vector_size-1:
                f.write(",")
            else:
                f.write("\n")

print("Done")
