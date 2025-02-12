#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <filesystem>

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
*/


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


    for (size_t i = 0; i < fileSize; i += (bitDepth / 8)) {  
        cout << "Encoding " << i << " of " << fileSize << " bytes" << endl;

        uint32_t value = 0;

        for (int b = 0; b < bitDepth / 8; b++) {
            if (i + b < fileSize)
                value |= rawData[i + b] << (8 * b);
                cout << "Value: " << value << endl;
        }

        /*FOR HANS TO REMEMBER:
                
				MSB is the first bit  aka i % 8 = 0, this means its shift is 7, and is placed at left most bit
				LSB is the last bit  aka i % 8 = 7, this means its shift is 0, and stays at right most bit

                FOR LAYERS:

				Layer 1: 1st bit of each byte, LSB
				Last Layer: Final Bit of each byte, MSB
                    
        */
      
        for (size_t i = 0; i < fileSize; i++) {
            uint8_t byte = rawData[i];

            for (int bitPos = 0; bitPos < bitDepth; bitPos++) {
                cout << "BitPos: " << bitPos << endl;
                size_t layerIndex = i / bitDepth; 
                uint8_t bitValue = (byte >> (7 - (bitPos % 8))) & 1;


                if (layerIndex >= bitLayers[bitPos].size()) {
                    std::cerr << "Error: Index out of range for bitLayers[" << bitPos << "][" << layerIndex << "]\n";
                    continue;
                }

                bitLayers[bitPos][layerIndex] |= (bitValue << (7 - (i % 8)));
             
            }
        }

    }

    for (int bitPos = 0; bitPos < bitDepth; bitPos++) {
        output.write(reinterpret_cast<char*>(bitLayers[bitPos].data()), bitLayers[bitPos].size());
    }


    cout << "Encoded binary file " << inputFile << " into " << outputFile << " successfully!" << endl;
}

void bitStackDecode(const string& inputFile) {
    ifstream input(inputFile, ios::binary);
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
    vector<vector<uint8_t>> bitLayers(bitDepth, vector<uint8_t>(layerSize));

    for (int bitPos = 0; bitPos < bitDepth; bitPos++) {
        input.read(reinterpret_cast<char*>(bitLayers[bitPos].data()), layerSize);
    }
    input.close();

    vector<uint8_t> reconstructedData(fileSize, 0);

    for (size_t i = 0; i < fileSize; i++) {
        uint8_t byte = 0;

        for (int bitPos = 0; bitPos < bitDepth; bitPos++) {
           size_t index = i / 8;  // Access bytes correctly
            size_t bitIndex = i % 8;  // Correct bit within byte


            if (index < bitLayers[bitPos].size()) {
               uint8_t bitValue = (bitLayers[bitPos][index] >> bitPos) & 1;

                byte |= (bitValue << bitPos);
            } else {
                cerr << "Error: Index out of bounds! bitPos=" << bitPos << ", index=" << index << endl;
                return;
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

    output.write(reinterpret_cast<char*>(reconstructedData.data()), fileSize);
    output.close();

    cout << "Decoded BSTACK file " << inputFile << " into " << outputFile << " successfully!" << endl;
}






