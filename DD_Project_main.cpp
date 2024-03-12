#include <iostream>
#include <fstream>
#include <string>
#include<vector>
using namespace std;

int main(){
    string fPath, outputFilePath;
    cout << "Enter the circuits' file : ";
    getline(cin, fPath);
    cout << "Enter output file name and path: ";
    getline(cin, outputFilePath);

    // Reading contents of file into vectors
    vector<vector<string>> lists;
    ifstream file;
    string fileName;
  
        file.open(fileName);
        if (!file.is_open()) {
            cerr << "Error opening file: " << fileName << endl;

        

        vector<string> s;
       string x;
        while (file >> x) {
            s.push_back(x);
        }
        file.close();
    return 0;
}}
