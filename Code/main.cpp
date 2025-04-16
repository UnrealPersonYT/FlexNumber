#include "fnum.hpp"
#include <chrono>
#include <fstream> // Include fstream for file operations

int main(void)
{
    fnum FlexNumber = 2;
    for(size_t index = 0; index < 1024; ++index) FlexNumber *= 2;

    std::cout << "Started Writting" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    std::ofstream outputFile("Test.txt");
    if(outputFile.is_open())
    {
        outputFile << FlexNumber << std::endl;
        outputFile.close();

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Result written to result.txt in " << elapsed.count() << " seconds." << std::endl;
    }else
    {
        std::cerr << "Failed to open the file for writing." << std::endl;
    }

    return 0;
}