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
const unsigned int FAIL = 1;
const unsigned int QUERY_COMPLEX_NAME = 0;
const unsigned int TARGET_COMPLEX_NAME = 1;
const unsigned int QUERY_CHAIN_LIST = 2;
const unsigned int TARGET_CHAIN_LIST = 3;
const unsigned int TM_SCORE_BASED_QUERY_LEN = 4;
const unsigned int TM_SCORE_BASED_TARGET_LEN = 5;
const unsigned int PARSING_FIELD_VALE_DONE = 5;

struct Fields {
    Fields() {}
    Fields(std::string &str, std::istringstream &iss, std::string &buffer) {
        iss = std::istringstream(str);
        unsigned int idx = 0;
        while (getline(iss, buffer, TAB)) {
            switch (idx++) {
                case QUERY_COMPLEX_NAME:
                    qComplex = buffer.substr(0,buffer.find(DOT));
                    break;
                case TARGET_COMPLEX_NAME:
                    tComplex = buffer.substr(0,buffer.find(DOT));;
                    break;
                case QUERY_CHAIN_LIST:
                    qChains = buffer;
                    break;
                case TARGET_CHAIN_LIST:
                    tChains = buffer;
                    break;
                case TM_SCORE_BASED_QUERY_LEN:
                    qTmScore = stod(buffer);
                    break;
                case TM_SCORE_BASED_TARGET_LEN:
                    tTmScore = stod(buffer);
                    break;
            }
            if (idx > PARSING_FIELD_VALE_DONE)
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
    if (argc < 3) {
        std::cerr << "INPUT FILE AND OTPUT FILE ARE REQUIRED!"  << std::endl;
        return FAIL;
    }

    std::string iFile = argv[INPUT_IDX];
    std::string oFile = argv[OUTPUT_IDX];
    std::map<std::pair<std::string, std::string>, Fields> bestAlignment;
    std::ifstream iFileReader = std::ifstream(iFile);
    std::ofstream oFileWriter = std::ofstream(oFile);

#pragma omp parallel
    {
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
    iFileReader.close();
    oFileWriter.close();

    std::cout << "process succeed"  << std::endl;
    return SUCCESS;
}