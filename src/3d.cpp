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
#include <3d.h>
#include <2d.h>
using namespace std;

Point3D::Point3D(){
    x = 0;
    y = 0;
    z = 0;
}
Point3D::Point3D(float newx, float newy, float newz)
{
    x = newx;
    y = newy;
    z = newz;
}
Point3D::Point3D(float theta, float phi){
    this->x = cosf(theta)*sinf(phi);
    this->y = sinf(theta);
    this->z = cosf(theta)*cosf(phi);
}
Point3D Point3D::operator+(Point3D const &p){
    Point3D ans;
    ans.x = x + p.x;
    ans.y = y + p.y;
    ans.z = z + p.z;
    return ans;
}
Point3D Point3D::operator-(Point3D const &p){
    Point3D ans;
    ans.x = x - p.x;
    ans.y = y - p.y;
    ans.z = z - p.z;
    return ans;
}
Point3D Point3D::operator*(float m){
    Point3D ans;
    ans.x = this->x * m;
    ans.y = this->y * m;
    ans.z = this->z * m;
    return ans;
}
bool Point3D::isequal(Point3D p)
{
    if (p.x == x && p.y == y && p.z == z)
    {
        return true;
    }
    return false;
}
Point3D Point3D::cross_product(Point3D p){
    Point3D ans;
    ans.x = (this->y * p.z) - (this->z*p.y);
    ans.y = - ((this->x * p.z) - (this->z * p.x));
    ans.z = (this->x * p.y) - (this->y * p.x);
    return ans;
}
float Point3D::dot_product(Point3D p){
    return this->x*p.x + this->y*p.y + this->z*p.z;
}
float Point3D::len(){
    return sqrt(pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2));
}
float Point3D::angle_between(Point3D p){
    Point3D tp1 = *this,tp2 = p;
    if (isnan(acos(tp1.unitize().dot_product(tp2.unitize())))){
        return acos(round(tp1.unitize().dot_product(tp2.unitize())));
    }
    return acos(tp1.unitize().dot_product(tp2.unitize()));
    // return acos((this->dot_product(p))/(this->len() * p.len()));
}
void Point3D::print(){
   cout << this->x << " , " << this->y << " , " << this->z;
}
void Point3D::print(int flag){
    if (flag == 0) cout << this->x << " , " << this->y << " , " << this->z;
    if (flag > 0) cout << this->x << " , " << this->y << " , " << this->z << endl;
}
Point3D Point3D::unitize(){
    float length = sqrt(pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2));
    return Point3D(this->x / length, this->y / length, this->z / length);
}
Point Point3D::to_2d(Point3D p0, Point3D px, Point3D py, int x_len, int y_len){
    float x, y, l, theta_x, theta_y;
    theta_x = (*this - p0).angle_between(px - p0);
    theta_y = (*this - p0).angle_between(py - p0);
    l = (*this - p0).len();
    x = ((float)x_len) * ((l * cos(theta_x)) /
                                    ((px - p0).len()));
    y = ((float)y_len) * ((l * cos(theta_y)) /
                                     ((py - p0).len()));

    if (this->isequal(p0)){
        x = 0;
        y = 0;
    }
    
    return Point(x,y);
}
float Point3D::theta(){
    // zy plane angle
    return (signbit(this->y) == 0? 1 : -1) * this->angle_between(Point3D(this->x, 0, this->z));
}
float Point3D::phi(){
    // zx plane angle
    return (signbit(this->x) == 0? 1 : -1) * Point3D(this->x, 0, this->z).angle_between(Point3D(0,0,1));
}

Ray::Ray(){
    p1 = Point3D();
    p2 = Point3D(1, 1, 1);
    this->set_angle_from_current_coords();
}
Ray::Ray(Point3D newp1, Point3D newp2){
    p1 = newp1;
    p2 = newp2;
    this->set_angle_from_current_coords();
}
Ray Ray::unitize(){
    p2 = Point3D(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
    p1 = Point3D(0, 0, 0);
    float length = sqrt(pow(p2.x, 2) + pow(p2.y, 2) + pow(p2.z, 2));
    p2 = Point3D(p2.x / length, p2.y / length, p2.z / length);
    return Ray(p1,p2);
}

void Ray::set_angle_from_current_coords(){
    Point3D temp(p2.x - p1.x,p2.y - p1.y,p2.z - p1.z);
    angle_yz = atan2(temp.y,temp.z);
    angle_xz = atan2(temp.z,temp.x);
}
Ray::Ray(float a_xz, float a_yz,float length, Point3D start_point){
    p1 = start_point;
    p2.x = p1.x + (length * cos(a_yz) * cos(a_xz));
    p2.z = p1.z + (length * cos(a_yz) * sin(a_xz));
    p2.y = p1.y + (length * cos(a_xz) * sin(a_yz)); 
}

Cube::Cube(){
    center = Point3D(0,0,0);
    side_length = 1;
    this->set_points();
    this->set_v();
    lines.push_back({{Point3D(1,1,1),Point3D(1,1,-1)},RGBcolor()});
	lines.push_back({{Point3D(1,1,1),Point3D(1,-1,1)},RGBcolor()});
	lines.push_back({{Point3D(1,1,1),Point3D(-1,1,1)},RGBcolor()});
	lines.push_back({{Point3D(1,1,-1),Point3D(1,-1,-1)},RGBcolor()});
	lines.push_back({{Point3D(1,1,-1),Point3D(-1,1,-1)},RGBcolor()});
	lines.push_back({{Point3D(-1,1,-1),Point3D(-1,-1,-1)},RGBcolor()});
	lines.push_back({{Point3D(-1,1,-1),Point3D(-1,1,1)},RGBcolor()});
	lines.push_back({{Point3D(-1,1,1),Point3D(-1,-1,1)},RGBcolor()});
	lines.push_back({{Point3D(1,-1,1),Point3D(1,-1,-1)},RGBcolor()});
	lines.push_back({{Point3D(1,-1,-1),Point3D(-1,-1,-1)},RGBcolor()});
	lines.push_back({{Point3D(-1,-1,-1),Point3D(-1,-1,1)},RGBcolor()});
	lines.push_back({{Point3D(-1,-1,1),Point3D(1,-1,1)},RGBcolor()});
}
Cube::Cube(Point3D c, float len){
    center = c;
    side_length = len;
    this->set_points();
    this->set_v();
}
void Cube::set_points(){
    Point3D c = center;
    float t = side_length/(float)2;
    vector<Point3D>p(8);
    p[0] = Point3D(c.x - t, c.y + t, c.z + t);
    p[1] = Point3D(c.x + t, c.y + t, c.z + t);
    p[2] = Point3D(c.x + t, c.y - t, c.z + t);
    p[3] = Point3D(c.x - t, c.y - t, c.z + t);
    p[4] = Point3D(c.x - t, c.y + t, c.z - t);
    p[5] = Point3D(c.x + t, c.y + t, c.z - t);
    p[6] = Point3D(c.x + t, c.y - t, c.z - t);
    p[7] = Point3D(c.x - t, c.y - t, c.z - t);
}
void Cube::set_v(){
    
    vector<Point3D> p = points;
    // sides.push_back({p[0],p[1]});
    // sides.push_back({p[1],p[2]});
    // sides.push_back({p[2],p[3]});
    // sides.push_back({p[3],p[0]});
    // sides.push_back({p[0],p[4]});
    // sides.push_back({p[1],p[5]});
    // sides.push_back({p[2],p[6]});
    // sides.push_back({p[3],p[7]});
    // sides.push_back({p[4],p[5]});
    // sides.push_back({p[5],p[6]});
    // sides.push_back({p[6],p[7]});
    // sides.push_back({p[7],p[4]});
}

Axes::Axes(){
    lines.push_back({{Point3D(0,0,-4), Point3D(0,0,4)},RGBcolor(0,0,0,255)});
	lines.push_back({{Point3D(-3,0,0), Point3D(3,0,0)},RGBcolor(0,0,0,255)});
	lines.push_back({{Point3D(0,-2,0), Point3D(0,2,0)},RGBcolor(0,0,0,255)});
}

Frame::Frame(){
    
}
Frame::Frame(Ray r, float d, float l, Ray up){
    // up direction = +ve y
    float h = l/(float)2;
    Ray unit_r = r, unit_up = up;
    unit_r.unitize();
    unit_up.unitize();
    Point3D side_vector = unit_r.p2.cross_product(unit_up.p2);
    Ray t(Point3D(0,0,0), side_vector);
    t.unitize();
    side_vector = (t.p2)*h;
    
    p1 = r.p1 + ((unit_r.p2)*d) - side_vector + (unit_up.p2)*h;
    p2 = r.p1 + ((unit_r.p2)*d) + side_vector + (unit_up.p2)*h;
    p3 = r.p1 + ((unit_r.p2)*d) + side_vector - (unit_up.p2)*h;
    p4 = r.p1 + ((unit_r.p2)*d) - side_vector - (unit_up.p2)*h;
}

Polygon3D::Polygon3D(vector<Point3D *> v){
    for (int i = 0; i < v.size(); i++)
    {
        this->points.push_back(v[i]);
    }
}

Point3D find_intersection(Frame f, Ray r){
    Point3D n = (f.p1 - f.p2).cross_product(f.p3 - f.p2).unitize(),
         a = f.p1, 
         o = r.p1, 
         d = (r.p2 - r.p1).unitize(), ans;
    float t = ((n.x*a.x + n.y*a.y + n.z*a.z) - n.x*o.x - n.y*o.y - n.z*o.z)/(n.x*d.x + n.y*d.y + n.z*d.z);
    ans.x = o.x + t*d.x;
    ans.y = o.y + t*d.y;
    ans.z = o.z + t*d.z;
    return ans;
}

void Translate(map< string, pair<vector<Point3D>,RGBcolor>> &object){

}

Point3D get_from_2d(Frame view_window, Point p, int w, int h)
{
    return view_window.p1 + ((view_window.p4 - view_window.p1).unitize() * ((p.y / h) * (view_window.p4 - view_window.p1).len())) + ((view_window.p2 - view_window.p1).unitize() * ((p.x / w) * (view_window.p2 - view_window.p1).len()));
}