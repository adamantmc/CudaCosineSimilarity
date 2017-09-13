#include "CosineSimilarity.hpp"
#include <cmath>

CosineSimilarity::CosineSimilarity(std::vector<std::vector<double> > v1, std::vector<std::vector<double> > v2) {
    this->v1 = v1;
    this->v2 = v2;
}

std::vector<std::vector<double> > CosineSimilarity::run() {
    calculateNorms();

    std::vector<std::vector<double> > results;

    std::vector<std::vector<double> >::iterator v1_iterator;
    std::vector<std::vector<double> >::iterator v2_iterator;

    for(int v1_index = 0; v1_index < v1.size(); v1_index++) {
        std::vector<double> vec_results;
        for(int v2_index = 0; v2_index < v2.size(); v2_index++) {
            double sim = dotProduct(v1[v1_index], v2[v2_index]) / (v1_norms[v1_index] * v2_norms[v2_index]);
            vec_results.push_back(sim);
        }
        results.push_back(vec_results);
    }

    return results;
}

void CosineSimilarity::calculateNorms() {
    std::vector<std::vector<double> >::iterator vector_iterator;
    std::vector<double>::iterator double_iterator;

    for(vector_iterator = v1.begin(); vector_iterator != v1.end(); vector_iterator++) {
        double norm = 0;
        for(double_iterator = (*vector_iterator).begin(); double_iterator != (*vector_iterator).end(); double_iterator++) {
            norm += (*double_iterator) * (*double_iterator);
        }
        v1_norms.push_back(sqrt(norm));
    }

    for(vector_iterator = v2.begin(); vector_iterator != v2.end(); vector_iterator++) {
        double norm = 0;
        for(double_iterator = (*vector_iterator).begin(); double_iterator != (*vector_iterator).end(); double_iterator++) {
            norm += (*double_iterator) * (*double_iterator);
        }
        v2_norms.push_back(sqrt(norm));
    }
}

double CosineSimilarity::dotProduct(std::vector<double> v1, std::vector<double> v2) {
    // Computes the dot product of two vectors (with same length)
    double dot_product = 0;

    for(int i = 0; i < v1.size(); i++) {
        dot_product += v1[i]*v2[i];
    }

    return dot_product;
}
