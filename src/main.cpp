#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <cstdlib>

#include "ArgumentParser.hpp"
#include "CSVParser.hpp"
#include "CosineSimilarity.hpp"
#include "CudaCosineSimilarity.hpp"

std::vector<std::vector<double> > serialCosineSimilarity(std::vector<std::vector<double> > v1, std::vector<std::vector<double> > v2, double *ms);
std::vector<std::vector<double> > cudaCosineSimilarity(std::vector<std::vector<double> > v1, std::vector<std::vector<double> > v2, double *ms);
std::vector<std::vector<double> > convertToFloat(std::vector<std::vector<std::string> > content);
bool checkResultEquality(std::vector<std::vector<double> >, std::vector<std::vector<double> >, int, int, double);

const char *help_msg =
    "Arguments:\n"
    "\t-h  Help - display this help message\n"
    "\t-v1 First Vector File\n"
    "\t-v2 Second Vector File\n"
    "\t-co Cuda Output File (default=cuda_results.txt)\n"
    "\t-so Serial Output File (default=serial_results.txt)\n"
    "\t-lb Line buffer size - buffer that holds output lines to reduce number of write calls (default=50000)\n"
    "\t-d  Decimals - number of decimals kept for results\n"
    "\n"
    "Usage:\n"
    "\t<arg>=<value>\n"
    "\n"
    "Examples:\n"
    "\t-v1=vectors_1.txt\n"
    "\t-d=5\n";

int main(int argc, char **argv) {
    ArgumentParser *arg_parser = new ArgumentParser();
    arg_parser->parse(argc, argv);

    bool help = arg_parser->getFlag("-h");

    if(help) {
        std::cout << help_msg << std::endl;
        exit(0);
    }

    arg_parser->printArguments();

    std::string v1_file = arg_parser->getStr("-v1");
    std::string v2_file = arg_parser->getStr("-v2");
    std::string cuda_output_file = arg_parser->getStr("-co");
    std::string serial_output_file = arg_parser->getStr("-so");
    int line_buffer_size = arg_parser->getInt("-lb");
    int decimals = arg_parser->getInt("-d");
    double tolerance;

    if(line_buffer_size == NULL) {
        line_buffer_size = 50000;
    }

    if(decimals == NULL) {
        decimals = 5;
    }
    else if(decimals < 1) {
        std::cout << "Number of decimals cannot be less than 1, defaulting to 5" << std::endl;
        decimals = 5;
    }

    tolerance = (double) 1/std::pow(10.0, decimals+1.0);

    if(v1_file.empty()) {
        std::cout << "Given V1 filename is invalid" << std::endl;
        std::cout << help_msg << std::endl;
        exit(0);
    }

    if(v2_file.empty()) {
        std::cout << "Given V2 filename is invalid" << std::endl;
        std::cout << help_msg << std::endl;
        exit(0);
    }

    if(cuda_output_file.empty()) {
        cuda_output_file = "cuda_results.txt";
    }

    if(serial_output_file.empty()) {
        serial_output_file = "serial_results.txt";
    }

    CSVParser *csv_parser = new CSVParser();
    std::vector<std::vector<double> > v1 = convertToFloat(csv_parser->parse((char *) v1_file.c_str()));
    std::vector<std::vector<double> > v2 = convertToFloat(csv_parser->parse((char *) v2_file.c_str()));

    std::cout << "Parsed the two vector files" << std::endl;

    double cuda_ms, serial_ms;

    std::cout << "Executing CUDA Version" << std::endl;
    std::vector<std::vector<double> > cuda_results = cudaCosineSimilarity(v1, v2, &cuda_ms);
    std::cout << "CUDA Version done" << std::endl;

    std::cout << "Executing Serial Version" << std::endl;
    std::vector<std::vector<double> > serial_results = serialCosineSimilarity(v1, v2, &serial_ms);
    std::cout << "Serial Version done" << std::endl;

    if(checkResultEquality(cuda_results, serial_results, v1.size(), v2.size(), tolerance)) {
        std::cout << "Same results given by both implementations" << std::endl;
    }

    std::cout << "CUDA running time: " << cuda_ms << "ms" << std::endl;
    std::cout << "Serial running time: " << serial_ms << "ms" << std::endl;

    std::cout << "Writing results to files (" << cuda_output_file << " and " << serial_output_file << ")" << std::endl;

    std::ostringstream cuda_string_buffer, serial_string_buffer;

    std::ofstream cuda_output, serial_output;
    cuda_output.open(cuda_output_file);
    serial_output.open(serial_output_file);

    cuda_string_buffer.precision(decimals);
    serial_string_buffer.precision(decimals);

    int line_counter = 0;
    for(int i = 0; i < v1.size(); i++) {
        for(int j = 0; j < v2.size(); j++) {
            cuda_string_buffer << cuda_results[i][j] << std::endl;
            serial_string_buffer << serial_results[i][j] << std::endl;
            line_counter += 1;
            if(line_counter == line_buffer_size) {
                line_counter = 0;

                cuda_output << cuda_string_buffer.str();
                serial_output << serial_string_buffer.str();

                cuda_string_buffer.str("");
                serial_string_buffer.str("");
            }
        }
    }

    cuda_output << cuda_string_buffer.str();
    serial_output << serial_string_buffer.str();

    cuda_output.close();
    serial_output.close();

    return 0;
}

std::vector<std::vector<double> > serialCosineSimilarity(std::vector<std::vector<double> > v1, std::vector<std::vector<double> > v2, double *ms) {
    std::cout << "Running Cosine Similarity" << std::endl;

    std::clock_t start = std::clock();
    CosineSimilarity *cosine = new CosineSimilarity(v1, v2);
    std::vector<std::vector<double> > results = cosine->run();
    std::clock_t end = std::clock();

    *ms = (end - start) / (double) (CLOCKS_PER_SEC / 1000);

    return results;
}

std::vector<std::vector<double> > cudaCosineSimilarity(std::vector<std::vector<double> > v1, std::vector<std::vector<double> > v2, double *ms) {
    std::cout << "Running CUDA Cosine Similarity" << std::endl;

    std::clock_t start = std::clock();
    double *results = cudaCosine(v1, v2);
    std::clock_t end = std::clock();

    std::vector<std::vector<double> > t_results;
    int vector_size = v1[0].size();

    std::vector<double> v1_results;
    for(int i = 0; i < v1.size(); i++) {
        v1_results.clear();
        for(int j = 0; j < v2.size(); j++) {
            v1_results.push_back(results[i*v2.size() + j]);
        }
        t_results.push_back(v1_results);
    }

    *ms = (end - start) / (double) (CLOCKS_PER_SEC / 1000);

    return t_results;
}

std::vector<std::vector<double> > convertToFloat(std::vector<std::vector<std::string> > content) {
    std::vector<std::vector<double> > converted;

    for(int i = 0; i < content.size(); i++) {
        std::vector<double> converted_vec;
        for(int j = 0; j < content[i].size(); j++) {
            converted_vec.push_back(std::stod(content[i][j]));
        }
        converted.push_back(converted_vec);
    }

    return converted;
}

bool checkResultEquality(std::vector<std::vector<double> > cuda_results,
    std::vector<std::vector<double> > serial_results,
    int v1_size, int v2_size, double tolerance) {

    bool same = true;
    for(int i = 0; i < v1_size; i++) {
        for(int j = 0; j < v2_size; j++) {
            if(abs(cuda_results[i][j] - serial_results[i][j]) > tolerance) {
                same = false;
                std::cout << "Uneven results at (" << i << "," << j << ") ";
                std::cout << "CUDA gave " << cuda_results[i][j] << " while Serial gave " << serial_results[i][j] << std::endl;
                break;
            }
        }
        if(!same) {
            break;
        }
    }

    return same;
}
