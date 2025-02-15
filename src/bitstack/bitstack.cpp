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

    //#pragma omp parallel for
    for (int i = 0; i < fileSize; i += bytesPerIteration) {  

        processedBytes+= bytesPerIteration;  

        // if (processedBytes % updateInterval == 0 || processedBytes == fileSize) {
        //     #pragma omp critical
        //     {
        //         cout << "\rEncoding " << processedBytes << " of " << fileSize
        //             << " bytes (" << fixed << setprecision(2) << (100.0 * processedBytes / fileSize) << "%)    "
        //             << flush;
        //     }
        // }

        uint32_t value = 0;

        for (int b = 0; b < bitDepth / 8; b++) {
            if (i + b >= fileSize){
                break;
            }
            
            
            value |= static_cast<uint32_t>(rawData[i + b]) << (8 * b);

            cout << "Value: " << (int)value << endl;
        }

        for (int layer = 0; layer < bitDepth; layer++) {
            
            size_t layerIndex = ((i + (layer / 8) )/ bitDepth); //might be issue
            uint8_t bitValue = (value >> (layer)) & 1;
            cout << "Layer: " << layer << ", Index: " << layerIndex << ", Bit Value: " << (int)bitValue << endl;
            // until this part is good ^

            int off = (i + (layer / 8)) % 8;

            if (layerIndex >= bitLayers[layer].size()) {
                std::cerr << "Error: Index out of range for bitLayers[" << layer << "][" << layerIndex << "]\n";
                continue;
            }

            bitLayers[layer][layerIndex] |= (bitValue << (7 - off));

            cout << "BitLayers[" << layer << "][" << layerIndex << "] = " << (int)bitLayers[layer][layerIndex] << endl;
             
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
    size_t encodedFileSize = filesystem::file_size(inputFile);
    size_t storedLayerSize = (encodedFileSize - sizeof(BStackHeader)) / bitDepth;

    size_t expectedLayerSize = (fileSize + (bitDepth - 1)) / bitDepth;

    vector<vector<uint8_t>> bitLayers(bitDepth, vector<uint8_t>(storedLayerSize, 0));
    for (int layer = 0; layer < bitDepth; layer++) {
    size_t bytesRead = 0;

        while (bytesRead < storedLayerSize) {
            input.read(reinterpret_cast<char*>(&bitLayers[layer][bytesRead]), storedLayerSize - bytesRead);
            size_t justRead = input.gcount();

            if (justRead == 0) {
                cerr << "Error: Unexpected end of file while reading layer " << layer << endl;
                return;
            }

            bytesRead += justRead;
        }

        cout << "Layer " << layer << " fully read: " << bytesRead << " bytes." << endl;
    }

    input.close();

    vector<uint8_t> reconstructedData(fileSize, 0);
    std::atomic<size_t> processedBytes(0);  
    size_t updateInterval = max(fileSize / 1000, (size_t)1);

    

    int bytesPerIteration = bitDepth / 8;

   // #pragma omp parallel for
    for (int i = 0; i < fileSize; i+= bytesPerIteration) {  


    //    processedBytes+= bytesPerIteration;  

    //     if (processedBytes % updateInterval == 0 || processedBytes == fileSize) {
    //         #pragma omp critical
    //         {
    //             cout << "\rEncoding " << processedBytes << " of " << fileSize
    //                 << " bytes (" << fixed << setprecision(2) << (100.0 * processedBytes / fileSize) << "%)    "
    //                 << flush;
    //         }
    //     }
         
       vector<uint8_t> byte (bytesPerIteration, 0);     
        for (int b = 0; b < bytesPerIteration; b++) {
            if (i + b >= fileSize) {
                break;
            }

            for (int layer = 8 * b; layer < (8 + 8*b); layer++) {
                size_t index = (i + b)/ bitDepth;  

                if (index < bitLayers[layer].size()) {
                    uint8_t bitValue = (bitLayers[layer][index] >> (7 - ((i + b) % 8))) & 1;


                    cout << "Layer: " << layer << ", Index: " << index << ", Bit Value: " << (int)bitValue << endl;

                    byte[layer / 8] |= (bitValue << (layer % 8)); 
                    
                } else {
                    cerr << "Error: Index out of bounds! layer=" << layer << ", index=" << index << endl;
                    #pragma omp critical
                    error_flag = true;
                }
            }
            
            cout << "Byte: " << (int)byte[b] << endl;
        }
        

        for (int c = 0; c < bytesPerIteration; c++) {


            if (i + c >= fileSize) {
                break;
            }
            reconstructedData[i + c] = byte[c];
        }
        
       
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








