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

void temp_insert_3d_point(vector<Point> &pts, Point p, vector<int> &poly)
{
    if (pts.size() == 0)
    {
        pts.push_back(p);
        poly.push_back(pts.size() - 1);
        return;
    }
    if (!pts.back().isequal(p))
    {
        pts.push_back(p);
        poly.push_back(pts.size() - 1);
    }
}

Object::Object(){
    points = vector<Point3D>(0);
    edges = vector<vector<int>>(0);
    polygons = vector<vector<int>>(0);
    normals = vector<Point3D>(0);
    colors = vector<RGBcolor>(0);
    center = Point3D();
    self_luminious = false;
}
Object2D Object::object_to_2d(Frame view_window, Point3D view_point, Ray normal){

    normal.p1 = Point3D();
    normal.p2 = (view_window.p2 - view_window.p1).cross_product(view_window.p4 - view_window.p1);

    // 3d object 'obj' to 2d object 'tempobj'
    Object2D tempobj;
    tempobj.center = find_intersection(view_window, Ray(view_point, this->center)).to_2d(view_window.p1, view_window.p2, view_window.p4, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT);
    tempobj.edges = this->edges;
    tempobj.colors = this->colors;
    tempobj.points = vector<Point>(0);

    for (auto&pi:this->polygons){
        tempobj.polygons.push_back({});
        for (int i = 0; i < pi.size(); i++){
            Point3D s = this->points[pi[i]], e = this->points[pi[(i+1)%pi.size()]];
            float angle_s = (s - view_window.p1).dot_product(normal.p2 - normal.p1), angle_e = (e - view_window.p1).dot_product(normal.p2 - normal.p1);
            
            // if both start and end are positive
            if (angle_s >= 0 && angle_e >= 0){
                temp_insert_3d_point(tempobj.points, find_intersection(view_window, Ray(view_point, s)).to_2d(view_window.p1, view_window.p2, view_window.p4, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT), tempobj.polygons.back());
                temp_insert_3d_point(tempobj.points, find_intersection(view_window, Ray(view_point, e)).to_2d(view_window.p1, view_window.p2, view_window.p4, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT), tempobj.polygons.back());
            }
            else if (angle_s < 0 && angle_e >= 0){
                temp_insert_3d_point(tempobj.points, find_intersection(view_window, Ray(s, e)).to_2d(view_window.p1, view_window.p2, view_window.p4, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT), tempobj.polygons.back());
                temp_insert_3d_point(tempobj.points, find_intersection(view_window, Ray(view_point, e)).to_2d(view_window.p1, view_window.p2, view_window.p4, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT), tempobj.polygons.back());
            }else if (angle_s >= 0 && angle_e < 0 ){
                temp_insert_3d_point(tempobj.points, find_intersection(view_window, Ray(view_point, s)).to_2d(view_window.p1, view_window.p2, view_window.p4, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT), tempobj.polygons.back());
                temp_insert_3d_point(tempobj.points, find_intersection(view_window, Ray(s, e)).to_2d(view_window.p1, view_window.p2, view_window.p4, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT), tempobj.polygons.back());
            }
        }
    }


    return tempobj;
}
vector<RGBcolor> Object::illumination(float ambient_light, Point3D light_source, float lsi)
{
    if (this->self_luminious){
        return this->colors;
    }
    vector<RGBcolor> ans_colors;
    int index = 0;
    float cos_theta, intensity, closeness;
    Point3D light_to_poly;
    for (auto&ni:this->normals){
        intensity = ambient_light;
        light_to_poly = (light_source - this->points[this->polygons[index][0]]);
        cos_theta = ni.unitize().dot_product((light_source - this->points[this->polygons[index][0]]).unitize());
        if (cos_theta <= 0){
            cos_theta = 0;
        }

        // formula for fading of intensity accourding to distance of light source
        closeness = 1 / (1 + pow((light_to_poly.len()/4),2));

        intensity += (lsi*cos_theta*closeness);
        if (intensity > 1) intensity = 1;
        RGBcolor c_t = this->colors[index];
        ans_colors.push_back(RGBcolor((c_t.r * intensity),
                                      (c_t.g * intensity),
                                      (c_t.b * intensity)));
        index++;
    }
    return ans_colors;
}

Object newCube()
{
    Object cube = Object();
    Point3D c = cube.center;
    float t = 1;
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
Object newCube(float length, Point3D center)
{
    Object cube = Object();
    cube.center = center;
    Point3D c = cube.center;
    float t = length/2;
    vector<Point3D> p(8);
    p[0] = Point3D(c.x - t, c.y + t, c.z + t); // 1
    p[1] = Point3D(c.x + t, c.y + t, c.z + t); // 2
    p[2] = Point3D(c.x + t, c.y - t, c.z + t); // 3
    p[3] = Point3D(c.x - t, c.y - t, c.z + t); // 4
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
    cube.normals.push_back(Point3D(0, 0, -1));
    cube.normals.push_back(Point3D(-1, 0, 0));
    cube.normals.push_back(Point3D(1, 0, 0));
    cube.normals.push_back(Point3D(0, 1, 0));
    cube.normals.push_back(Point3D(0, -1, 0));

    cube.colors.push_back(RGBcolor(255, 0, 0));
    cube.colors.push_back(RGBcolor(255, 165, 0));
    cube.colors.push_back(RGBcolor(0, 255, 0));
    cube.colors.push_back(RGBcolor(0, 0, 255));
    cube.colors.push_back(RGBcolor(255, 255, 255));
    cube.colors.push_back(RGBcolor(255, 255, 0));

    return cube;
}
Object newCube(float length, RGBcolor color, Point3D center)
{
    Object cube = Object();
    cube.center = center;
    Point3D c = cube.center;
    float t = length / 2;
    vector<Point3D> p(8);
    p[0] = Point3D(c.x - t, c.y + t, c.z + t); // 1
    p[1] = Point3D(c.x + t, c.y + t, c.z + t); // 2
    p[2] = Point3D(c.x + t, c.y - t, c.z + t); // 3
    p[3] = Point3D(c.x - t, c.y - t, c.z + t); // 4
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
    cube.normals.push_back(Point3D(0, 0, -1));
    cube.normals.push_back(Point3D(-1, 0, 0));
    cube.normals.push_back(Point3D(1, 0, 0));
    cube.normals.push_back(Point3D(0, 1, 0));
    cube.normals.push_back(Point3D(0, -1, 0));

    for (auto&pi:cube.polygons){
        cube.colors.push_back(color);
    }
    return cube;
}

Object newPlane(float length, Ray r){
    Object plane = Object();
    plane.center = r.p1;
    Point3D c = plane.center;
    float t = length/2;
    vector<Point3D> p(4);
    p[0] = Point3D(c.x - t, c.y, c.z + t);
    p[1] = Point3D(c.x + t, c.y, c.z + t); 
    p[2] = Point3D(c.x + t, c.y, c.z - t);
    p[3] = Point3D(c.x - t, c.y, c.z - t);
    plane.points = p;

    plane.edges.push_back({0, 1});
    plane.edges.push_back({1, 2});
    plane.edges.push_back({2, 3});
    plane.edges.push_back({3, 1});

    plane.polygons.push_back({0, 1, 2, 3});

    plane.normals.push_back(r.p2 - r.p1);

    plane.colors.push_back(RGBcolor(255, 255, rand()%150 + 50));

    return plane;
}
Object newPlane(float length, Ray r, RGBcolor color)
{
    Object plane = Object();
    plane.center = r.p1;
    Point3D c = plane.center;
    float t = length / 2;
    vector<Point3D> p(4);
    p[0] = Point3D(c.x - t, c.y, c.z + t);
    p[1] = Point3D(c.x + t, c.y, c.z + t);
    p[2] = Point3D(c.x + t, c.y, c.z - t);
    p[3] = Point3D(c.x - t, c.y, c.z - t);
    plane.points = p;

    plane.edges.push_back({0, 1});
    plane.edges.push_back({1, 2});
    plane.edges.push_back({2, 3});
    plane.edges.push_back({3, 1});

    plane.polygons.push_back({0, 1, 2, 3});

    plane.normals.push_back(r.p2 - r.p1);

    plane.colors.push_back(color);

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

// 