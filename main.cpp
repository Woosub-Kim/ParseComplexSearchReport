#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#ifdef OPENMP
#include <omp.h>
#endif

const char DOT = '.';
const char NL = '\n';
const char TAB = '\t';
const unsigned int INPUT_IDX = 1;
const unsigned int OUTPUT_IDX = 2;
const unsigned int SUCCESS = 0;
const double DONE_INIT = 0.0;

struct Fields {
    Fields() {}
    Fields(std::string &str, std::istringstream &iss, std::string &buffer) {
        iss = std::istringstream(str);
        unsigned int idx = 0;
        while (getline(iss, buffer, TAB)) {
            switch (idx++) {
                case 0:
                    qComplex = buffer.substr(0,buffer.find(DOT));
                    break;
                case 1:
                    tComplex = buffer.substr(0,buffer.find(DOT));;
                    break;
                case 2:
                    qChains = buffer;
                    break;
                case 3:
                    tChains = buffer;
                    break;
                case 4:
                    qTmScore = stod(buffer);
                    break;
                case 5:
                    tTmScore = stod(buffer);
                    break;
            }
            if (idx > 5)
                return;
        }
    }

    std::string qComplex;
    std::string tComplex;
    std::string qChains;
    std::string tChains;
    double qTmScore;
    double tTmScore;
};

int main(int argc, char* argv[]) {
    std::string iFile = argv[INPUT_IDX];
    std::string oFile = argv[OUTPUT_IDX];
    std::map<std::pair<std::string, std::string>, Fields> bestAlignment;
    std::ofstream oFileWriter = std::ofstream(oFile);
#pragma omp parallel
    {
        std::ifstream iFileReader = std::ifstream(iFile);
        std::string line;
        std::istringstream iss;
        std::string buffer;
        Fields fields;
        std::pair<std::string, std::string> key;
#pragma omp while
        while(getline(iFileReader, line)) {
            fields = Fields(line, iss, buffer);
            key = {fields.qComplex, fields.tComplex};
            if (bestAlignment.find(key) == bestAlignment.end()) {
                bestAlignment.insert({key, fields});
                continue;
            }
            if (bestAlignment[key].qTmScore < fields.qTmScore) {
                bestAlignment[key] = fields;
            }
        }
    }

    for (auto &iter: bestAlignment) {
        oFileWriter << iter.second.qComplex << TAB << iter.second.tComplex << TAB << iter.second.qChains << TAB << iter.second.tChains<< TAB << iter.second.qTmScore << TAB << iter.second.tTmScore << NL;
    }
    std::cout << "process succeed"  << std::endl;
    return SUCCESS;
}