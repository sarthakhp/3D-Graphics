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
// #include <2d.h>
#include <myHeader/Object.h>

#include <myHeader/values.h>

int dist = 0;

Edge3D::Edge3D(vector<Point3D *> v)
{
    this->p1 = v[0];
    this->p2 = v[1];
    this->len = ((*v[0]) - (*v[1])).len();
}
void Edge3D::set(vector<Point3D*> v){
    this->p1 = v[0];
    this->p2 = v[1];
    this->len = ((*v[0]) - *v[1]).len();
}

Object::Object(){
    center = Point3D();
}

Object newCube()
{
    Object cube = Object();
    Point3D c = cube.center;
    float t = 0.75;
    vector<Point3D> p(8);
    p[0] = Point3D(c.x - t, c.y + t, c.z + t); //1
    p[1] = Point3D(c.x + t, c.y + t, c.z + t); //2
    p[2] = Point3D(c.x + t, c.y - t, c.z + t); //3
    p[3] = Point3D(c.x - t, c.y - t, c.z + t); //4
    p[4] = Point3D(c.x - t, c.y + t, c.z - t);
    p[5] = Point3D(c.x + t, c.y + t, c.z - t);
    p[6] = Point3D(c.x + t, c.y - t, c.z - t);
    p[7] = Point3D(c.x - t, c.y - t, c.z - t);
    cube.points = p;
    // vertexArray = {0, 1, 2, 2, 1, 4}

    // new line ^ ^
    // changing new branch only, i ll be gone in a minute ;-;

    cube.edges.push_back({0, 1});
    cube.edges.push_back({1, 2});
    cube.edges.push_back({2, 3});
    cube.edges.push_back({3, 1});
    cube.edges.push_back({4, 5});
    cube.edges.push_back({5, 6});
    cube.edges.push_back({6, 7});
    cube.edges.push_back({7, 4});
    cube.edges.push_back({0, 4});
    cube.edges.push_back({1, 5});
    cube.edges.push_back({2, 6});
    cube.edges.push_back({3, 7});

    cube.polygons.push_back({0, 1, 2, 3});
    cube.polygons.push_back({4, 5, 6, 7});
    cube.polygons.push_back({0, 4, 7, 3});
    cube.polygons.push_back({1, 2, 6, 5});
    cube.polygons.push_back({0, 1, 5, 4});
    cube.polygons.push_back({3, 2, 6, 7});

    cube.normals.push_back(Point3D(0, 0, 1));
    cube.normals.push_back(Point3D(0, 0,-1));
    cube.normals.push_back(Point3D(-1, 0, 0));
    cube.normals.push_back(Point3D(1, 0, 0));
    cube.normals.push_back(Point3D(0, 1, 0));
    cube.normals.push_back(Point3D(0,-1, 0));

    cube.colors.push_back(RGBcolor(255,0,0));
    cube.colors.push_back(RGBcolor(255,165,0));
    cube.colors.push_back(RGBcolor(0,255,0));
    cube.colors.push_back(RGBcolor(0,0,255));
    cube.colors.push_back(RGBcolor(255,255,255));
    cube.colors.push_back(RGBcolor(255,255,0));

    return cube;
}
Object newPlane(){
    Object plane = Object();
    Point3D c = plane.center;
    float t = 0.75;
    vector<Point3D> p(4);
    p[0] = Point3D(c.x - t, c.y - t, c.z + t); // 4
    p[1] = Point3D(c.x + t, c.y - t, c.z + t); // 3
    p[2] = Point3D(c.x + t, c.y - t, c.z - t);
    p[3] = Point3D(c.x - t, c.y - t, c.z - t);
    plane.points = p;
    // vertexArray = {0, 1, 2, 2, 1, 4}

    plane.edges.push_back({0, 1});
    plane.edges.push_back({1, 2});
    plane.edges.push_back({2, 3});
    plane.edges.push_back({3, 1});

    plane.polygons.push_back({0, 1, 2, 3});

    plane.colors.push_back(RGBcolor(255, 0, 0));

    return plane;
}

bool find_intersection(Ray r, Object obj, int pi, Point3D &ans){
    obj.points[obj.polygons[pi][0]];
    obj.points[obj.polygons[pi][1]];
    obj.points[obj.polygons[pi][2]];
    Point3D n = (obj.points[obj.polygons[pi][0]] - obj.points[obj.polygons[pi][1]]).cross_product(obj.points[obj.polygons[pi][2]] - obj.points[obj.polygons[pi][1]]).unitize(),
                                            a = obj.points[obj.polygons[pi][0]],
                                            o = r.p1,
                                            d = (r.p2 - r.p1).unitize();
    float t = ((n.x * a.x + n.y * a.y + n.z * a.z) - n.x * o.x - n.y * o.y - n.z * o.z) / (n.x * d.x + n.y * d.y + n.z * d.z);
    ans.x = o.x + t * d.x;
    ans.y = o.y + t * d.y;
    ans.z = o.z + t * d.z;
    if ((ans - r.p1).dot_product(r.p2 - r.p1) > 0){
        return 1;
    }
    return 0;
}

void points_to_pixel(vector<vector<Point>> fill_points, Point3D view_point, Frame view_window, Object objs, int pi, Screen_memory &sm, vector<vector<float>> &dm){
    Point3D ip, ap;
    for (int i = 0; i < fill_points.size(); i++){
        for (int j = 0; j < fill_points[i].size(); j++){
            // convert to 3d world
            ip = get_from_2d(view_window, fill_points[i][j], MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT);
            
            if ( find_intersection(Ray(view_point, ip), objs, pi, ap ) ){

                dist = (ap - view_point).len() - (ip - view_point).len();
                if ((dist < dm[fill_points[i][j].y][fill_points[i][j].x] || dm[fill_points[i][j].y][fill_points[i][j].x] <= 0) && dist > 0 )
                {
                    dm[fill_points[i][j].y][fill_points[i][j].x] = dist;
                    sm[fill_points[i][j].y][fill_points[i][j].x] = objs.colors[pi];
                }
            }

        }
    }
}