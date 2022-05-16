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

#define PI 3.14159265359

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

    cout << - sin(PI/4) + cos (PI/4) << endl;

    return 0;
}