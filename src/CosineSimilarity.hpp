#ifndef COSINE_SIMILARITY_H_
#define COSINE_SIMILARITY_H_
#include <vector>

class CosineSimilarity {

public:
    CosineSimilarity(std::vector<std::vector<double> > v1, std::vector<std::vector<double> > v2);
    std::vector<std::vector<double> > run();
private:
    std::vector<std::vector<double> > v1, v2;
    std::vector<double> v1_norms, v2_norms;
    void calculateNorms();
    double dotProduct(std::vector<double> v1, std::vector<double> v2);
};

#endif // COSINE_SIMILARITY_H_
