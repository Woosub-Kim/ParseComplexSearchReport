#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#ifdef OPENMP
#include <omp.h>
#endif

// CONSTANTS
const char NL = '\n';
const char TAB = '\t';
const char DOT = '.';
const unsigned int INPUT_IDX = 1;
const unsigned int OUTPUT_IDX = 2;
const unsigned int SUCCESS = 0;
const unsigned int FAIL = 1;
const unsigned int COL_QUERY_COMPLEX = 0;
const unsigned int COL_TARGET_COMPLEX = 1;
const unsigned int COL_QUERY_CHAIN_LIST = 2;
const unsigned int COL_TARGET_CHAIN_LIST = 3;
const unsigned int COL_QUERY_TM_SCORE = 4;
const unsigned int COL_TARGET_TM_SCORE = 5;
const unsigned int LAST_COLUMN = 5;

// TYPES
struct Fields {
    Fields() {}
    Fields(std::string &str, std::istringstream &iss, std::string &buffer) {
        iss = std::istringstream(str);
        unsigned int column = 0;
        while (getline(iss, buffer, TAB)) {
            switch (column++) {
                case COL_QUERY_COMPLEX:
                    qComplex = buffer.substr(0,buffer.find(DOT));
                    break;
                case COL_TARGET_COMPLEX:
                    tComplex = buffer.substr(0,buffer.find(DOT));
                    break;
                case COL_QUERY_CHAIN_LIST:
                    qChains = buffer;
                    break;
                case COL_TARGET_CHAIN_LIST:
                    tChains = buffer;
                    break;
                case COL_QUERY_TM_SCORE:
                    qTmScore = stod(buffer);
                    break;
                case COL_TARGET_TM_SCORE:
                    tTmScore = stod(buffer);
                    break;
            }
            if (column > LAST_COLUMN)
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
typedef std::pair<std::string, std::string> Key_t;
typedef std::map<Key_t , Fields> BestAlignment_t;

// MAIN FUNC
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "INPUT FILE AND OUTPUT FILE ARE REQUIRED!"  << std::endl;
        return FAIL;
    }
    std::string iFile = argv[INPUT_IDX];
    std::string oFile = argv[OUTPUT_IDX];
    std::ifstream iFileReader = std::ifstream(iFile);
    std::ofstream oFileWriter = std::ofstream(oFile);
    std::istringstream iss;
    BestAlignment_t bestAlignment;

#pragma omp parallel
    {
        std::string line;
        std::string buffer;
        Fields fields;
        Key_t key;
#pragma omp while
        // for each line
        while(getline(iFileReader, line)) {
            // get fields and key from the line
            fields = Fields(line, iss, buffer);
            key = {fields.qComplex, fields.tComplex};
            // when it is the best alignment for the current key, keep this alignment
            if (bestAlignment.find(key) == bestAlignment.end()) {
                bestAlignment.insert({key, fields});
                continue;
            }
            bestAlignment[key] = fields.qTmScore > bestAlignment[key].qTmScore ? fields : bestAlignment[key];
        }
    }
    // write best alignments
    for (auto &iter: bestAlignment) {
        Fields &fields = iter.second;
        oFileWriter << fields.qComplex << TAB << fields.tComplex << TAB << fields.qChains << TAB << fields.tChains << TAB << fields.qTmScore << TAB << fields.tTmScore << NL;
    }
    // close & clear
    iFileReader.close();
    oFileWriter.close();
    iss.clear();
    iFile.clear();
    oFile.clear();
    bestAlignment.clear();
    std::cout << "process succeed"  << std::endl;
    return SUCCESS;
}