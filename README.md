# CudaCosineSimilarity

Parallel implementation of Cosine Similarity using Nvidia CUDA. Also contains a serial implementation to check against.

Compilation:
```
nvcc src\CSVParser.cpp src\CosineSimilarity.cpp src\CudaCosineSimilarity.cu src\ArgumentParser.cpp src\main.cpp
```

The program accepts two files, containing vectors in a CSV format. It then executes both implementations and reports their running time. The program also outputs the results of each implementation in two different files. The results contain the cosine similarity of each vector v1 (from file 1) with each vector v2 (from file 2). For example, for two files file1 and file2 with 5 and 10 lines each, the first 10 lines of the results would be the similarities of the first vector of file1 with all 10 vectors of file2.

You can use generator.py to create randomized input files.
