#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <random>
#include <map>
#include <climits>
#include <bits/stdc++.h>

using namespace std;

int main(){

    vector<vector<int>> v = {
        {3, 1},
        {2, 2},
        {1, 3},
        {4, 4},
    };
    sort(v.begin(), v.end());
    for (int i = 0; i < v.size(); i++){
        cout << v[i][0] << " " << v[i][1] << endl;
    }
    return 0;
}