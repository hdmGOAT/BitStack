#include "bitstack/bitstack.h"

#include <iostream>
#include <fstream>
#include <bitset>


using namespace std;


int main(){

	string fileName = "BlackMarble_2016_928m_conus.tif";

	string inputFilePath = "C:\\Users\\User\\Desktop\\testImages\\tifs\\" + fileName;


	string outputImagePath = "C:/Users/User/Desktop/testImages/" + fileName +".bstack";

	bitStackEncode(inputFilePath, "C:/Users/User/Desktop/testImages/" + fileName + ".bstack", 8);


	bitStackDecode(outputImagePath, "C:/Users/User/Desktop/testImages/" + fileName + "_decoded.tif");

    return 0;
}