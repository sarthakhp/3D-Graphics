#pragma once

#include <SDL_headers/SDL.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <random>
#include <map>
#include <climits>
#include <SDL_headers/SDL_ttf.h>
using namespace std;

#include <2d.h>

class Point3D{

    public:
        float x,y,z;
        Point3D();
        Point3D(float newx, float newy, float newz);
        Point3D(float theta, float phi);
        Point3D operator+(Point3D const &p);
        Point3D operator-(Point3D const &p);
        Point3D operator*(float m);
        bool isequal(Point3D p);
        Point3D cross_product(Point3D p);
        float dot_product(Point3D p);
        float len();
        float angle_between(Point3D);
        void print();
        void print(int flag);
        Point3D unitize();
        Point to_2d(Point3D p0, Point3D px, Point3D py, int x_len, int y_len);
        float theta();
        float phi();
        
};

class Ray{
    public:
        Point3D p1;
        Point3D p2;
        float angle_yz,angle_xz; 
        
        Ray();
        Ray(Point3D newp1, Point3D newp2);
        Ray(float a_xz, float a_yz, float length, Point3D start_point);
        Ray unitize();
        void set_angle_from_current_coords();
        void set_angle(float a1,float a2);
        
};
class Line{
    Point3D p1;
    Point3D p2;
};

class Cube{
    public:
        Point3D center;
        float side_length;
        vector<Point3D> points;
        vector <pair<vector<Point3D>,RGBcolor>> lines;
        Cube();
        Cube(Point3D c, float l);
        void set_points();
        void set_v();

};
class Axes{
    public:
        vector <pair<vector<Point3D>,RGBcolor>> lines;
        Axes();
};
class Frame{
    public:
        Point3D p1,p2,p3,p4;
        Ray up;
        Frame();
        Frame(Ray r, float distance, float side_len, Ray up);
};
class Polygon3D{
public:
    vector<Point3D *> points;

    // methods
    Polygon3D(vector<Point3D *> v);
    void set(vector<Point3D *> v);
};

Point3D find_intersection(Frame f, Ray r);
Point3D get_from_2d(Frame f, Point p, int w, int h);

void Translate(map<string, pair<vector<Point3D>, RGBcolor>> &object);
