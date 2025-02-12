#include <iostream>
#include <filesystem>
#include <string>
#include "bitstack/bitstack.h" 

namespace fs = std::filesystem;
using namespace std;

void printUsage() {
    cout << "Usage:\n";
    cout << "  Encode a file: btsk -e <input_file> <bit_depth>\n";
    cout << "  Decode a file: btsk -d <input_file>\n";
    cout << "  Encode a folder: btsk -E <folder> <bit_depth>\n";
    cout << "  Decode a folder: btsk -D <folder>\n";
}

bool isValidFile(const fs::path& path) {
    return fs::is_regular_file(path);  
}

void encodeFolder(const string& folderPath, int bitDepth) {
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (isValidFile(entry.path())) {
            string inputFile = entry.path().string();
            cout << "Encoding: " << inputFile << endl;
            bitStackEncode(inputFile, bitDepth);
        }
    }
}

void decodeFolder(const string& folderPath) {
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (isValidFile(entry.path()) && entry.path().extension() == ".bstack") {
            string inputFile = entry.path().string();
            cout << "Decoding: " << inputFile << endl;
            bitStackDecode(inputFile);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2 || string(argv[1]) == "-?") {
        printUsage();
        return 1;
    }

    string mode = argv[1];

    if (mode == "-e" && argc == 4) {  
        string inputFile = argv[2];
        int bitDepth = stoi(argv[3]);
        bitStackEncode(inputFile, bitDepth);
    } 
    else if (mode == "-d" && argc == 3) {  
        string inputFile = argv[2];
        bitStackDecode(inputFile);
    } 
    else if (mode == "-E" && argc == 4) {  
        string folderPath = argv[2];
        int bitDepth = stoi(argv[3]);
        encodeFolder(folderPath, bitDepth);
    } 
    else if (mode == "-D" && argc == 3) {  
        string folderPath = argv[2];
        decodeFolder(folderPath);
    } 
    else {
        cerr << "Error: Invalid command.\n";
        printUsage();
        return 1;
    }

    return 0;
}
