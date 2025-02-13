#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <atomic>


using namespace std;

struct BStackHeader {
    char signature[8] = "BSTACK\0";  
    uint64_t originalSize;           
    uint8_t bitDepth;   
    char extension[16] = {0} ;       
};

/*
    Bit Stacking expected functionality:
        Each first bit is in Layer 1,
		Each second bit is in Layer 2,
		Each third bit is in Layer 3,
		Each fourth bit is in Layer 4,
        ...

		This allows for compressors to better view redundancy in the data.



    THESE ARE WAYYYY TOO SLOW TO BE FEASIBLE
*/

void printProgress(size_t processed, size_t totalBytes) {
    double percentage = (double)processed / totalBytes * 100;

    #pragma omp critical  // Ensure only one thread prints at a time
    {
        cout << "\rEncoding " << processed << " of " << totalBytes
             << " bytes (" << fixed << setprecision(2) << percentage << "%)    "
             << flush;
    }
}

void bitStackEncode(const string& inputFile,  int bitDepth) {
    ifstream input(inputFile, ios::binary | ios::ate);
    if (!input) {
        cerr << "Error: Cannot open input file: " << inputFile << endl;
        return;
    }
    size_t fileSize = input.tellg();
    input.seekg(0, ios::beg);
    vector<uint8_t> rawData(fileSize);
    input.read(reinterpret_cast<char*>(rawData.data()), fileSize);
    input.close();

    string fileBase = filesystem::path(inputFile).stem().string();  
    string fileExt = filesystem::path(inputFile).extension().string();

    string outputFile = fileBase + ".bstack";
    ofstream output(outputFile, ios::binary);
    if (!output) {
        cerr << "Error: Cannot open output file: " << outputFile << endl;
        return;
    }

    BStackHeader header = { "BSTACK\0", fileSize, static_cast<uint8_t>(bitDepth) };
    strncpy(header.extension, fileExt.c_str(), sizeof(header.extension) - 1); 
    output.write(reinterpret_cast<char*>(&header), sizeof(header));


    size_t layerSize = (fileSize + (bitDepth - 1)) / bitDepth;


    vector<vector<uint8_t>> bitLayers(bitDepth);
    for (int i = 0; i < bitDepth; i++) {
        bitLayers[i].resize(layerSize, 0); 
    }

    std::atomic<size_t> processedBytes(0);  

    size_t updateInterval = max(fileSize / 1000, (size_t)1);

    int bytesPerIteration = bitDepth / 8;

    #pragma omp parallel for
    for (int i = 0; i < fileSize; i += bytesPerIteration) {  

        processedBytes+= bytesPerIteration;  

        if (processedBytes % updateInterval == 0 || processedBytes == fileSize) {
            #pragma omp critical
            {
                cout << "\rEncoding " << processedBytes << " of " << fileSize
                    << " bytes (" << fixed << setprecision(2) << (100.0 * processedBytes / fileSize) << "%)    "
                    << flush;
            }
        }

        uint32_t value = 0;

        for (int b = 0; b < bitDepth / 8; b++) {
            if (i + b < fileSize)
                value |= rawData[i + b] << (8 * b);
        }


        for (int layer = 0; layer < bitDepth; layer++) {

            size_t layerIndex = i / bitDepth; 
            uint8_t bitValue = (value >> (7 - (layer % 8))) & 1;


            if (layerIndex >= bitLayers[layer].size()) {
                    std::cerr << "Error: Index out of range for bitLayers[" << layer << "][" << layerIndex << "]\n";
                    continue;
            }

            bitLayers[layer][layerIndex] |= (bitValue << (7 - (i % 8)));
             
        }

    }

    for (int layer = 0; layer < bitDepth; layer++) {
        output.write(reinterpret_cast<char*>(bitLayers[layer].data()), bitLayers[layer].size());
    }


    cout << "Encoded binary file " << inputFile << " into " << outputFile << " successfully!" << endl;
}

void bitStackDecode(const string& inputFile) {
    ifstream input(inputFile, ios::binary);
    bool error_flag = false;
    if (!input) {
        cerr << "Error: Cannot open input file: " << inputFile << endl;
        return;
    }

    BStackHeader header;
    input.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (memcmp(header.signature, "BSTACK\0", 7) != 0) {
        cerr << "Error: Invalid BSTACK file format!" << endl;
        return;
    }

    size_t fileSize = header.originalSize;
    int bitDepth = header.bitDepth;
    size_t layerSize = (fileSize + (bitDepth - 1)) / bitDepth;

    vector<vector<uint8_t>> bitLayers(bitDepth, vector<uint8_t>(layerSize, 0));
    for (int layer = 0; layer < bitDepth; layer++) {
        input.read(reinterpret_cast<char*>(bitLayers[layer].data()), layerSize);
    }
    input.close();

    vector<uint8_t> reconstructedData(fileSize, 0);
    std::atomic<size_t> processedBytes(0);  
    size_t updateInterval = max(fileSize / 1000, (size_t)1);

    for (int i = 0; i < fileSize; i++) {

        cout << "\rEncoding " << processedBytes << " of " << fileSize << " bytes (" << fixed << setprecision(2) << (100.0 * processedBytes / fileSize) << "%)    " << flush;
         
        uint8_t byte = 0;

        for (int layer = 0; layer < bitDepth; layer++) {
            size_t index = i / bitDepth;  
            uint8_t bitOffset = i % 8;  

            if (index < bitLayers[layer].size()) {
                uint8_t bitValue = (bitLayers[layer][index] >> (7 - bitOffset)) & 1;
                byte |= (bitValue << (bitDepth - 1 - layer));  
            } else {
                cerr << "Error: Index out of bounds! layer=" << layer << ", index=" << index << endl;
                #pragma omp critical
                error_flag = true;
            }
        }

        reconstructedData[i] = byte;
    }

    

    string outputFile = filesystem::path(inputFile).stem().string();
    outputFile += header.extension;

    

    ofstream output(outputFile, ios::binary);
    if (!output) {
        cerr << "Error: Cannot open output file: " << outputFile << endl;
        return;
    }
    
    if (error_flag) {
        cerr << "An error occurred. Exiting safely." << endl;
        return;  // Exit the function early instead of printing below
    }
    output.write(reinterpret_cast<char*>(reconstructedData.data()), fileSize);
    output.close();

    cout << "Decoded BSTACK file " << inputFile << " into " << outputFile << " successfully!" << endl;
}








