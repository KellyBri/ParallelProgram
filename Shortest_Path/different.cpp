#include <iostream>
#include <vector>
#include <fstream>


#define INFINITE 9999999



int main(int argc, char **argv){


    //read input file
    std::vector<int> correct;
    std::ifstream inputFile(argv[1], std::ios::in | std::ios::binary);
    int a;
    while( inputFile.read((char*)&a, 4) ){
        correct.push_back(a);
    }
    inputFile.close();

    //read input file
    std::vector<int> answer;
    inputFile.open(argv[2], std::ios::in | std::ios::binary);
    while( inputFile.read((char*)&a, 4) ){
        answer.push_back(a);
    }
    inputFile.close();

    int i=0;
    auto ita=answer.begin(); 
    for(auto itc = correct.begin(); itc != correct.end(); ++itc, ++ita, ++i){
        if(*itc != *ita){
            std::cout<<"Fuck "<<i<<"\n";
        }
    }


    
    return 0;
}
