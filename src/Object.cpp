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
#include <fstream>
#include <sstream>

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

void temp_insert_3d_point(vector<Point> &pts,vector<float> &intensties, Point p, vector<int> &poly)
{
    if (pts.size() == 0)
    {
        pts.push_back(p);
    	intensties.push_back(0);
        poly.push_back(pts.size() - 1);
        return;
    }
    if (!pts.back().isequal(p))
    {
        pts.push_back(p);
        poly.push_back(pts.size() - 1);
    }

}
void temp_insert_3d_point_to_3d_object(vector<Point3D> &pts, Point3D p, vector<int> &poly)
{
    if (pts.size() == 0)
    {
        pts.push_back(p);
        poly.push_back(pts.size() - 1);
        return;
    }
    if (!pts.back().isequal(p))
    {
        int index=0;
        for (auto&op:pts){
            if (op.isequal(p)){
                for (auto&pts_p:poly){
                    if (pts_p == index){
                        return;
                    }
                }
                poly.push_back(index);
                return;
            }
            index++;
        }
        pts.push_back(p);
    }
    poly.push_back(pts.size() - 1);
}
vector<string> split_string(string original_string, char delimiter)
{
    std::string tmp = "";
    std::vector<std::string> parts;

    std::istringstream iss(original_string);
    while (std::getline(iss, tmp, delimiter))
    {
        parts.push_back(tmp);
    }
    return parts;
}

Object::Object(){
    points = vector<Point3D>(0);
    edges = vector<vector<int>>(0);
    polygons = vector<vector<int>>(0, vector<int>(0));
    normals = vector<Point3D>(0);
    colors = vector<RGBcolor>(0);
    center = Point3D();
    Kd = vector<vector<float>>(0, vector<float>(3));
    Ks = vector<vector<float>>(0, vector<float>(3));
    self_luminious = false;
}
Object Object::clip_3d_object(Frame view_window, Point3D view_point, Ray normal){
    Object tempobj = *this;
    tempobj.points = vector<Point3D>(0);
    tempobj.polygons = vector<vector<int>>(0);

    for (auto &pi : this->polygons)
    {
        tempobj.polygons.push_back({});
        for (int i = 0; i < pi.size(); i++)
        {
            Point3D s = this->points[pi[i]], e = this->points[pi[(i + 1) % pi.size()]];
            float angle_s = (s - view_window.p1).dot_product(normal.p2 - normal.p1), angle_e = (e - view_window.p1).dot_product(normal.p2 - normal.p1);
            Point3D inters = find_intersection(view_window, Ray(s, e));
            // float mp_vi =  + ((vi_e - vi_s) * ((inters - s).len() / (e - s).len()));
            // if both start and end are positive
            if (angle_s >= 0 && angle_e >= 0)
            {
                temp_insert_3d_point_to_3d_object(tempobj.points, s, tempobj.polygons.back());
                temp_insert_3d_point_to_3d_object(tempobj.points, e, tempobj.polygons.back());
            }
            else if (angle_s < 0 && angle_e >= 0)
            {
                temp_insert_3d_point_to_3d_object(tempobj.points, inters, tempobj.polygons.back());
                temp_insert_3d_point_to_3d_object(tempobj.points,  e, tempobj.polygons.back());
            }
            else if (angle_s >= 0 && angle_e < 0)
            {
                temp_insert_3d_point_to_3d_object(tempobj.points, s, tempobj.polygons.back());
                temp_insert_3d_point_to_3d_object(tempobj.points, inters, tempobj.polygons.back());
            }
        }
    }

    // vertex normals;
    tempobj.vertex_normals = vector<Point3D>(tempobj.points.size(), Point3D());
    int index = 0;
    for (auto &pi : tempobj.polygons)
    {
        for (auto &v : pi)
        {
            tempobj.vertex_normals[v] = tempobj.vertex_normals[v] + tempobj.normals[index];
        }
        index++;
    }

    return tempobj;
}
Object2D Object::object_to_2d(Frame view_window, Point3D view_point, Ray normal){

    normal.p1 = Point3D();
    normal.p2 = (view_window.p2 - view_window.p1).cross_product(view_window.p4 - view_window.p1);

    // 3d object 'obj' to 2d object 'tempobj'
    Object2D tempobj;
    tempobj.center = find_intersection(view_window, Ray(view_point, this->center)).to_2d(view_window.p1, view_window.p2, view_window.p4, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT);
    tempobj.colors = this->colors;
    tempobj.edges = this->edges;
    tempobj.polygons = this->polygons;
    tempobj.points = vector<Point>(this->points.size());
    tempobj.vertex_intensity = vector<float>(this->points.size(), 1);

    for (auto &pi : this->polygons)
    {
        for (int i = 0; i < pi.size(); i++){
            tempobj.points[pi[i]] = ( find_intersection(view_window, Ray(view_point, this->points[pi[i]])).to_2d(view_window.p1, view_window.p2, view_window.p4, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT) );
        }
    }

    return tempobj;
}
vector<RGBcolor> Object::illumination(float ambient_light, vector< LightSource> light_source)
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

        for (auto&ls:light_source){
            if (this->polygons[index].size() == 0){
                break;
            }
            light_to_poly = (ls.p - this->points[this->polygons[index][0]]);
            cos_theta = ni.unitize().dot_product((ls.p - this->points[this->polygons[index][0]]).unitize());
            // if (cos_theta <= 0){
            //     cos_theta = 0;
            // }

            // formula for fading of intensity accourding to distance of light source
            closeness = 1 / (1 + pow((light_to_poly.len()/10),2));
            intensity += (ls.intensity*cos_theta*closeness);
        }


        if (intensity > 1) intensity = 1;
        if (intensity < 0.2) intensity = 0.2;
        RGBcolor c_t = this->colors[index];
        ans_colors.push_back(RGBcolor((c_t.r * intensity),
                                      (c_t.g * intensity),
                                      (c_t.b * intensity)));
        index++;
    }
    return ans_colors;
}
vector<float> Object::gouraud_shading(float ambient_light, vector< LightSource> light_source){
    if (this->self_luminious)
    {
        return vector<float>(this->vertex_normals.size(), 1);
    }
    vector<float> ans;
    int index = 0;
    float cos_theta, intensity, closeness;
    Point3D light_to_poly;
    for (auto &ni : this->vertex_normals)
    {
        intensity = ambient_light;
        for (auto &ls : light_source)
        {
            light_to_poly = (ls.p - this->points[index]);
            cos_theta = ni.unitize().dot_product(light_to_poly.unitize());
            // if (cos_theta <= 0)
            // {
            //     cos_theta = 0;
            // }

            // formula for fading of intensity accourding to distance of light source
            closeness = 1 / (1 + pow((light_to_poly.len() / 10), 2));
            intensity += (ls.intensity * cos_theta);
        }

        intensity = min (intensity, (float)1);
        intensity = max(intensity, (float)0.1);
        ans.push_back(intensity);
        index++;
    }
    return ans;
}
Object Object::readObject(string path){


    // material file:
    map<string, RGBcolor> mtl_colors;
    ifstream inFile;
    // cout << split_string( path, '.')[0] + ".mtl" << endl;
    inFile.open(split_string( path, '.')[0] + ".mtl");
    if (!inFile)
    {
        printf("No mtl file !\n");
        mtl_colors["no_file"] = RGBcolor(0, 0, 255);
    }
    else{
        string line;
        vector<vector<int>> vn;
        vector<Point3D> vn_points;
        float r,g,b;
        string mtl_name;
        while (getline(inFile, line))
        {
            std::stringstream ss{line};
            string s;
            ss >> s;
            
            if (s == "newmtl")
            {
                ss >> mtl_name;
            }
            else if (s == "Kd")
            {
                ss >> r;
                ss >> g;
                ss >> b;
                mtl_colors[ mtl_name] = RGBcolor(r*255, g*255, b*255);

                mtl_name = "";
            }
            else if (s == "Ks"){

            }
        }
        int c=0;
        for (auto&i:mtl_colors){
            cout << i.second.r << endl;
            c++;
        }
        cout << "total " << c << endl;
    }
    inFile.close();
    

    *this = Object();
    inFile.open(path);
    if (!inFile)
    {
        printf("Impossible to open the file !\n");
        return *this;
    }

    string line;
    vector<vector<int>> vn;
    vector<Point3D> vn_points;
    Point3D p;
    string mtl_name = "no_file";

    while (getline(inFile, line))
    {
        std::stringstream ss{line};
        string s;
        ss >> s;
        if (s == "v")
        {
            ss >> p.x;
            ss >> p.y;
            ss >> p.z;
            this->points.push_back(p);
        }
        else if (s == "vn")
        {
            ss >> p.x;
            ss >> p.y;
            ss >> p.z;
            
            vn_points.push_back(p);            
        }
        else if (s == "f")
        {
            this->polygons.push_back({});
            this->colors.push_back(mtl_colors[mtl_name]);
            vn.push_back({});
            string s;
            while (ss >> s)
            {
                vector<string> parts = split_string(s, '/');
                this->polygons.back().push_back(stoi(parts[0]) - 1);
                vn.back().push_back(stoi(parts[2]) - 1);
            }
        }
        else if (s == "usemtl"){
            mtl_name = "";
            ss >> mtl_name;
        }
    }

    int index = 0;
    for (auto&pi:this->polygons){
        Point3D ni = (this->points[pi[2]] - this->points[pi[0]]).cross_product(this->points[pi[1]] - this->points[pi[0]]);

        float dot_p = ni.dot_product(vn_points[ vn[index][0] ]);

        this->normals.push_back( (dot_p >= 0) ? ni : Point3D() - ni );
        // this->colors.push_back(RGBcolor(rand() % 255, rand() % 255, rand() % 255));
        // this->colors.push_back(RGBcolor(0, 0, 255));

        index++;
    }
    
    // vertex normals;
    this->vertex_normals = vector<Point3D>(this->points.size(), Point3D());
    index = 0;
    for (auto&pi:this->polygons){
        for (auto&v:pi){
            this->vertex_normals[v] = this->vertex_normals[v] + this->normals[index];
        }
        index++;
    }

    // center
    this->center = Point3D();
    for (auto&pts:this->points){
        this->center = this->center + pts;
    }
    this->center = Point3D(this->center.x / this->points.size(), this->center.y / this->points.size(), this->center.z / this->points.size());
    
    return *this;
}
vector<Object> Object::readMultipleObject(string path){

    // material file:
    map<string, RGBcolor> mtl_colors;
    ifstream inFile;
    // cout << split_string( path, '.')[0] + ".mtl" << endl;
    inFile.open(split_string(path, '.')[0] + ".mtl");
    if (!inFile)
    {
        printf("No mtl file !\n");
        mtl_colors["no_file"] = RGBcolor(0, 0, 255);
    }
    else
    {
        string line;
        vector<vector<int>> vn;
        vector<Point3D> vn_points;
        float r, g, b;
        string mtl_name;
        while (getline(inFile, line))
        {
            std::stringstream ss{line};
            string s;
            ss >> s;

            if (s == "newmtl")
            {
                ss >> mtl_name;
            }
            else if (s == "Kd")
            {
                ss >> r;
                ss >> g;
                ss >> b;
                mtl_colors[mtl_name] = RGBcolor(r * 255, g * 255, b * 255);
                mtl_name = "";
            }
        }
        int c = 0;
        for (auto &i : mtl_colors)
        {
            cout << i.second.r << endl;
            c++;
        }
        cout << "total " << c << endl;
    }
    inFile.close();

    *this = Object();
    vector<Object> m_o(0);
    inFile.open(path);
    if (!inFile)
    {
        printf("Impossible to open the file !\n");
        return m_o;
    }

    string line;
    vector<vector<int>> vn(0);
    vector<Point3D> vn_points(0);
    Point3D p;
    int total_pts = 0;
    string mtl_name = "no_file";
    bool first_object = true;
    while (getline(inFile, line))
    {
        std::stringstream ss{line};
        string s;
        ss >> s;
        if (s == "v")
        {
            ss >> p.x;
            ss >> p.y;
            ss >> p.z;
            m_o.back().points.push_back(p);
        }
        else if (s == "vn")
        {
            ss >> p.x;
            ss >> p.y;
            ss >> p.z;

            vn_points.push_back(p);
        }
        else if (s == "f")
        {
            m_o.back().polygons.push_back({});
            m_o.back().colors.push_back(mtl_colors[mtl_name]);
            vn.push_back({});
            string s;
            while (ss >> s)
            {
                vector<string> parts = split_string(s, '/');
                m_o.back().polygons.back().push_back(stoi(parts[0]) - 1 - total_pts);
                vn.back().push_back(stoi(parts[2]) - 1 - total_pts);
            }
        }
        else if (s == "usemtl")
        {
            mtl_name = "";
            ss >> mtl_name;
        }
        else if (s == "o"){
            if (m_o.size()){
                int index = 0;
                for (auto &pi : m_o.back().polygons)
                {
                    Point3D ni = (m_o.back().points[pi[2]] - m_o.back().points[pi[0]]).cross_product(m_o.back().points[pi[1]] - m_o.back().points[pi[0]]);

                    float dot_p = ni.dot_product(vn_points[vn[index][0]]);

                    m_o.back().normals.push_back((dot_p >= 0) ? ni : Point3D() - ni);
                    // m_o.back().colors.push_back(RGBcolor(rand() % 255, rand() % 255, rand() % 255));
                    // m_o.back().colors.push_back(RGBcolor(0, 0, 255));

                    index++;
                }
                if (m_o.back().normals.size() == 0){
                    cout << "nouooooooooooooooooo : ";
                    cout << m_o.size() << endl;
                }

                // vertex normals;
                m_o.back().vertex_normals = vector<Point3D>(m_o.back().points.size(), Point3D());
                index = 0;
                for (auto &pi : m_o.back().polygons)
                {
                    for (auto &v : pi)
                    {
                        m_o.back().vertex_normals[v] = m_o.back().vertex_normals[v] + m_o.back().normals[index];
                    }
                    index++;
                }

                // center
                m_o.back().center = Point3D();
                for (auto &pts : m_o.back().points)
                {
                    m_o.back().center = m_o.back().center + pts;
                }
                m_o.back().center = Point3D(m_o.back().center.x / m_o.back().points.size(), m_o.back().center.y / m_o.back().points.size(), m_o.back().center.z / m_o.back().points.size());
                total_pts += m_o.back().points.size();
                vn = vector<vector<int>>(0,vector<int>(0));
                vn_points = vector<Point3D>(0);

            }
            m_o.push_back(Object());
        }
    }
    int index = 0;
    for (auto &pi : m_o.back().polygons)
    {
        Point3D ni = (m_o.back().points[pi[2]] - m_o.back().points[pi[0]]).cross_product(m_o.back().points[pi[1]] - m_o.back().points[pi[0]]);

        float dot_p = ni.dot_product(vn_points[vn[index][0]]);

        m_o.back().normals.push_back((dot_p >= 0) ? ni : Point3D() - ni);
        // m_o.back().colors.push_back(RGBcolor(rand() % 255, rand() % 255, rand() % 255));
        // m_o.back().colors.push_back(RGBcolor(0, 0, 255));

        index++;
    }
    if (m_o.back().normals.size() == 0)
    {
        cout << "nouooooooooooooooooo : ";
        cout << m_o.size() << endl;
    }

    // vertex normals;
    m_o.back().vertex_normals = vector<Point3D>(m_o.back().points.size(), Point3D());
    index = 0;
    for (auto &pi : m_o.back().polygons)
    {
        for (auto &v : pi)
        {
            m_o.back().vertex_normals[v] = m_o.back().vertex_normals[v] + m_o.back().normals[index];
        }
        index++;
    }

    // center
    m_o.back().center = Point3D();
    for (auto &pts : m_o.back().points)
    {
        m_o.back().center = m_o.back().center + pts;
    }
    m_o.back().center = Point3D(m_o.back().center.x / m_o.back().points.size(), m_o.back().center.y / m_o.back().points.size(), m_o.back().center.z / m_o.back().points.size());

    return m_o;
    
}
Object Object::move(Point3D move_by)
{
    this->center = this->center + move_by;
    for (auto &p : this->points)
    {
        p = p + move_by;
    }
    return *this;
}
Object Object::move_to_origin()
{
    Point3D original_center = this->center;
    this->move(Point3D() - this->center);
    return *this;
}
Object Object::rotate(float theta_yz, float phi_xz){
    
    Point3D original_center = this->center;
    this->move(Point3D() - this->center);

    
    for (auto &p : this->points)
    {
        Point3D tp = p;

        // first around x axis (theta_yz):
        p.y = (tp.y * cos(theta_yz)) - (tp.z * sin(theta_yz));
        p.z = (tp.y * sin(theta_yz)) + (tp.z * cos(theta_yz));

        tp = p;
        // around y axis phi_xz
        p.x = (tp.x * cos(phi_xz)) + (tp.z * sin(phi_xz));
        p.z = -(tp.x * sin(phi_xz)) + (tp.z * cos(phi_xz));
    }

    // normals:
    for (auto&n:this->normals){

        Point3D tn = n;
        
        // around x axis
        n.y = (tn.y * cos(theta_yz)) - (tn.z * sin(theta_yz));
        n.z = (tn.y * sin(theta_yz)) + (tn.z * cos(theta_yz));

        tn = n;
        // around y axis
        n.x = (tn.x * cos(phi_xz)) + (tn.z * sin(phi_xz));
        n.z = -(tn.x * sin(phi_xz)) + (tn.z * cos(phi_xz));

    }
    for (auto &n : this->vertex_normals)
    {

        Point3D tn = n;

        // around x axis
        n.y = (tn.y * cos(theta_yz)) - (tn.z * sin(theta_yz));
        n.z = (tn.y * sin(theta_yz)) + (tn.z * cos(theta_yz));

        tn = n;
        // around y axis
        n.x = (tn.x * cos(phi_xz)) + (tn.z * sin(phi_xz));
        n.z = -(tn.x * sin(phi_xz)) + (tn.z * cos(phi_xz));
    }

    this->move(original_center);

    return *this;
}
Object Object::scale(float x, float y, float z){
    Point3D original_center = this->center;
    this->move(Point3D() - this->center);

    for (auto &p : this->points)
    {
        p = Point3D(p.x * x, p.y * y, p.z * z);
    }

    this->move(original_center);
    return *this;
}
Object Object::scale(float s)
{
    Point3D original_center = this->center;
    this->move(Point3D() - this->center);

    for (auto &p : this->points)
    {
        p = Point3D(p.x * s, p.y * s, p.z * s);
    }

    this->move(original_center);
    return *this;
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
    plane.vertex_normals.push_back(p[0]);
    plane.vertex_normals.push_back(p[1]);
    plane.vertex_normals.push_back(p[2]);
    plane.vertex_normals.push_back(p[3]);

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