#include <SDL_headers/SDL.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <random>
#include <map>
#include <climits>
#include <bits/stdc++.h>

//
#include <input_handler.h>
#include <3d.h>
#include <2d.h>
#include <myHeader/Object.h>
#include <myHeader/values.h>
using namespace std;

int main(){

    Frame f = Frame(Ray(Point3D(0, 0, -1), Point3D()), 1, 1, Ray(Point3D(0, 0, 0), Point3D(0, 1, 0)));
    f.p1.print(1);
    Object2D o = newPlane().object_to_2d(f, Point3D(0, 0, -2));
    
    for (auto&i:o.points){
        i.print(1);
    }
    
    
    
    int somen = 0;
    cout << "press any key..."; 
    cin >> somen;
    return 0;

    return 0;
}