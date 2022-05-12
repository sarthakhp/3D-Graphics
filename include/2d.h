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

struct Point {
	float x;
	float y;
	Point() {
		x = 0;
		y = 0;
	}
	Point(float newx, float newy) {
		x = newx;
		y = newy;
	}
	Point operator+(Point const &p){
		Point ans;
		ans.x = x + p.x;
		ans.y = y + p.y;
		return ans;
	}
	bool isequal(Point p){
		if (p.x == x && p.y == y){
			return true;
		}
		return false;
	}
    void print();
    void print(int endl);
};

struct RGBcolor {
    int r, g, b, a;
    RGBcolor() {
        r = 255;
        g = 255;
        b = 255;
        a = 255;
    }
    RGBcolor(int ci){
        r = ci; g = ci; b = ci; a = 255;
    }
    RGBcolor(int ri, int gi, int bi, int ai) {
        r = ri;
        g = gi;
        b = bi;
        a = ai;
    }
    RGBcolor(int ri, int gi, int bi) {
        r = ri;
        g = gi;
        b = bi;
        a = 255;
    }
    bool is_equal(RGBcolor object2) {
        if (r == object2.r && g == object2.g && b == object2.b && a == object2.a) {
            return true;
        }
        return false;
    }
    RGBcolor operator + (RGBcolor & c_temp){
        RGBcolor ans;
        ans.r = ((r + c_temp.r)>255)?255:(r + c_temp.r);
        ans.g = ((g + c_temp.g)>255)?255:(g + c_temp.g);
        ans.b = ((b + c_temp.b)>255)?255:(b + c_temp.b);
        ans.a = ((a + c_temp.a)>255)?255:(a + c_temp.a);
        return ans;
    }
    RGBcolor operator - (RGBcolor const &c_temp){
        RGBcolor ans;
        ans.r = ((r - c_temp.r)<0)?0:(r - c_temp.r);
        ans.g = ((g - c_temp.g)<0)?0:(g - c_temp.g);
        ans.b = ((b - c_temp.b)<0)?0:(b - c_temp.b);
        ans.a = ((a - c_temp.a)<0)?0:(a - c_temp.a);
        return ans;
    }
};

class Screen_memory {
	public:
        // int MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT;
		vector<RGBcolor> tempv;
		vector<vector<RGBcolor>> v;

        Screen_memory();
		Screen_memory(int w, int h);
        Screen_memory(int w, int h, RGBcolor color);
        vector<RGBcolor>& operator [](int index);
		int size();

		void renderer(SDL_Renderer *renderer);
};

class Object2D
{
public:
    // points
    vector<Point> points;

    // edges
    vector<vector<int>> edges;

    // planes
    vector<vector<int>> polygons;

    // colors
    vector<RGBcolor> colors;

    Point center;

    // methods
    Object2D();
    Object2D clip_object(Point window_2d);
    Object2D clip_object_2(const Point &window);
};

void mid_point_line_draw(Screen_memory &sm, Point &start, Point &end, const RGBcolor &line_color, int erase_mode);

pair<Point,int> intersection_between_segments(vector<Point> v);

pair<Point, bool> intersection_between_two_lines(vector<Point> line1, vector<Point> line2);

vector <pair<vector<Point>,RGBcolor>> clip(vector <pair<vector<Point>,RGBcolor>> lines, int w, int h);

vector<Point> mid_point_line_draw_c(Screen_memory &actual_sm, Point start, Point end, const RGBcolor &line_color, int erase_mode, Screen_memory &temp_sm);

vector<Point> mid_point_line_draw_c(Point start, Point end, const RGBcolor &line_color, int erase_mode, Screen_memory &temp_sm);

vector<vector<Point>> row_fill(vector<Point> polygon_points, RGBcolor fill_color, Screen_memory &temp_sm);

void row_fill_direct_to_screen(vector<Point> polygon_points, RGBcolor fill_color, Screen_memory &temp_sm);










// end