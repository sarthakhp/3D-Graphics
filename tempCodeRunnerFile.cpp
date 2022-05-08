#include<iostream>
#include<vector>
using namespace std;


void swap(vector<int> &v, int i, int j){
    while ( i+1 != v[i] && v[i] <= v.size() && v[j] != j+1){
        int tmp = v[i];
        v[i] = v[j];
        v[j] = tmp;
        j = v[i] - 1;        
    }
}

int firstMissingPositive(vector<int> &A) {
    int ss = A.size();
    for(long long int i = 0 ; i < ss ; i++)
    {

        // if (A[i] == 606) cout << "sfdsd2222222";
        // if (A[i] == 1){
        //     cout << "sfdfs";
        // }
        if (A[i] <= A.size() && A[i] > 0){
            swap(A, i, A[i] - 1);
        }
    }
    // for (int i = 0; i < 10; i++){
    //     cout << A[i] << " ";
    // }
    // cout << endl;
    cout << A.size() << "==" << endl;
    for (int i = 0; i < 5; i++){
        cout << i << " : he" << endl; 
        if (A[i] != i+1){
            return i+1;
        }
    }
}

int main(){
    vector<int> g = {1,2,3,4,5};
    cout << firstMissingPositive(g);
    
    return 0;
}



// 8 9 5 6 7 1 2 3 4

// 1 2 3 4 5 6 7




