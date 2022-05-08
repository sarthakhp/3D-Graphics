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

// my headers
#include <input_handler.h>
#include <3d.h>
#include <2d.h>


class Edge3D{
public:
    float len;
    Point3D* p1,*p2;

    // methods
    Edge3D(vector<Point3D*> v);
    void set(vector<Point3D*> v);
};

class Object{
public:
    // points
    vector<Point3D> points;
    
    // edges
    vector<vector<int>> edges;

    // planes
    vector<vector<int>> polygons;

    // normals
    vector<Point3D> normals;

    // colors
    vector<RGBcolor> colors;

    Point3D center; 

    // methods
    Object();

    // equation
    // circle
    int r;
    vector<Point3D> intersection();
};

Object newCube();
Object newPlane();

void assign_pixel(Screen_memory &temp_sm, Screen_memory &actual_sm, vector<vector<float>> &distance_memory, Point &current, Object &object, int poly_i);
// void mid_point_line_draw_c(Screen_memory &sm, Point start, Point end, const RGBcolor &line_color, int erase_mode, vector<vector<float>> &distance_memory, Screen_memory &temp_sm, Object &obj, int polygon_index);

void points_to_pixel(vector<vector<Point>> fill_points, Point3D view_point, Frame view_window, Object objs, int pi, Screen_memory &sm, vector<vector<float>> &dm);

bool find_intersection(Ray r, Object objs, int pi, Point &ap);