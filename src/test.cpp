#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <random>
#include <map>
#include <climits>
#include <fstream>
#include <bits/stdc++.h>

using namespace std;

vector<string> split_string(string original_string, char delimiter){
    std::string tmp = "";
    std::vector<std::string> parts;

    std::istringstream iss(original_string);
    while (std::getline(iss, tmp, delimiter))
    {
        parts.push_back(tmp);
    }
    return parts;
}

int main(){

    string path = "cube.obj";
    ifstream inFile;

    inFile.open(path);
    if (!inFile){
        printf("Impossible to open the file !\n");
        return false;
    }

    int c=0;
    string line;
    vector<vector<int>> v, vn, p;
    while (getline(inFile, line))
    {
        c++;
        std::stringstream ss{line};
        string s;
        ss >> s;
        if (s == "v"){
            v.push_back({});
            int a;
            while (ss >> a){
                v.back().push_back(a);
            }
        }
        else if (s == "vn")
        {
            vn.push_back({});
            int a;
            while (ss >> a)
            {
                vn.back().push_back(a);
            }
        }
        else if (s == "f")
        {
            p.push_back({});
            string s;
            while (ss >> s)
            {
                vector<string> parts = split_string(s,'/');
                p.back().push_back( stoi( parts[0] ) );
            }
        }
    }
    for (auto&vi:p){
        for (auto&j:vi){
            cout << j << " ";
        }
        cout << endl;
    }
    cout << "hello" << endl;

    return 0;
}