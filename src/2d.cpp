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

RGBcolor color;

void Point::print(){
    cout << this->x << " (2d) " << this->y;
}
void Point::print(int end_l){
	if (end_l > 0) cout << this->x << " (2d) " << this->y << endl;
}

Screen_memory::Screen_memory()
{
	v = vector<vector<RGBcolor>>(0);
}
Screen_memory::Screen_memory(int w, int h){
    RGBcolor r = RGBcolor(255, 225, 0, 255);
    tempv = vector<RGBcolor>(w, r);
    v = vector<vector<RGBcolor>>(h, tempv);
}
Screen_memory::Screen_memory(int w, int h, RGBcolor color)
{
	tempv = vector<RGBcolor>(w, color);
	v = vector<vector<RGBcolor>>(h, tempv);
}
vector<RGBcolor> & Screen_memory :: operator [](int index) {
    return v[index];
}
int Screen_memory::size() {
    return v.size();
}
void  Screen_memory :: renderer(SDL_Renderer *renderer){
    for (int  j= 0; j < v.size(); j++){
        for (int i = 0; i < v[j].size(); i++){
            color = v[j][i]; 
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderDrawPoint(renderer, i, j);
        }
    }
}

pair<Point, bool> intersection_between_two_lines(vector<Point> line1, vector<Point> line2)
{
	float a1, b1, c1, a2, b2, c2;
	// cases: slope of lines are infinite
	if (line1[0].x == line1[1].x)
	{
		a1 = 1;
		b1 = 0;
		c1 = -line1[0].x;
	}
	else
	{
		a1 = ((line1[1].y - line1[0].y) / (line1[1].x - line1[0].x));
		b1 = 1;
		c1 = line1[0].y - (a1 * (line1[0].x));
	}
	if (line2[0].x == line2[1].x)
	{
		a2 = 1;
		b2 = 0;
		c2 = -line2[0].x;
	}
	else
	{
		a2 = ((line2[1].y - line2[0].y) / (line2[1].x - line2[0].x));
		b2 = 1;
		c2 = line2[0].y - (a2 * (line2[0].x));
	}
	if ((a1 * b2) == (a2 * b1))
	{
		return {Point(), false};
	}
	else
	{
		return {Point((b1 * c2 - b2 * c1) / (a1 * b2 - a2 * b1), -(a2 * c1 - a1 * c2) / (a1 * b2 - a2 * b1)), true};
	}
}

void temp_insert_point(vector<Point> &pts, Point p, vector<int> &poly){
	if (pts.size() == 0){
		pts.push_back(p);
		poly.push_back(pts.size() - 1);
		return;
	}
	if (!pts.back().isequal(p)){
		pts.push_back(p);
		poly.push_back(pts.size()-1);
	}
}

Object2D::Object2D()
{
	points = vector<Point>(0);
	edges = vector<vector<int>>(0);
	polygons = vector<vector<int>>(0);
	colors = vector<RGBcolor>(0);
	center = Point();
}
Object2D Object2D::clip_object(Point window){
	Object2D ans_obj;
	ans_obj.colors = this->colors;
	for(auto&pi:this->polygons){
		ans_obj.polygons.push_back({});
		for (int i = 0; i < pi.size(); i++){
			// first point inside_frame ?
			if (this->points[pi[i]].x <= window.x && this->points[pi[i]].x >= 0 && this->points[pi[i]].y <= window.y && this->points[pi[i]].y >= 0){
				ans_obj.points.push_back(this->points[pi[i]]);
				ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
				if (this->points[pi[(i + 1) % pi.size()]].x <= window.x && this->points[pi[(i + 1) % pi.size()]].x >= 0 && this->points[pi[(i + 1) % pi.size()]].y <= window.y && this->points[pi[(i + 1) % pi.size()]].y >= 0){
					ans_obj.points.push_back(this->points[pi[(i + 1) % pi.size()]]);
					ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					continue;
				}
			}

			// finding line intersection with grid
			vector<Point> l = {this->points[pi[i]], this->points[pi[(i + 1) % pi.size()]]};
			// grid line x = 0
			vector<Point> grid_line = {Point(0,0) , Point(0,window.y)};
			intersection_between_two_lines(l, grid_line).first.print(1);
			if (intersection_between_two_lines(l, grid_line).second){
				Point intersected_point = intersection_between_two_lines(l, grid_line).first;
				cout << "ahhh" << endl;
				l[0].print(1);
				l[1].print(1);
				if (intersected_point.x <= (max(l[0].x, l[1].x)) 
				&& intersected_point.y <= (max(l[0].y, l[1].y))
				&& intersected_point.x >= (min(l[0].x, l[1].x)) 
				&& intersected_point.y >= (min(l[0].x, l[1].x)) ){
					// check if point is out side (upper side or lower side) of box or inside the box
					cout << "oyooo" << endl;
					if (intersected_point.y > window.y){
						ans_obj.points.push_back(grid_line[1]);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
					else if (intersected_point.y < 0)
					{
						ans_obj.points.push_back(grid_line[0]);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
					else{
						cout << "yes" << endl;
						ans_obj.points.push_back(intersected_point);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
				}
			}
			// grid line x = max
			grid_line = {Point(window.x, 0), Point(window.x, window.y)};
			if (intersection_between_two_lines(l, {Point(window.x, 0), Point(window.x, window.y)}).second)
			{
				Point intersected_point = intersection_between_two_lines({this->points[pi[i]], this->points[pi[(i + 1) % pi.size()]]}, grid_line).first;
				if (intersected_point.x <= (max(l[0].x, l[1].x)) && intersected_point.y <= (max(l[0].y, l[1].y)) && intersected_point.x >= (min(l[0].x, l[1].x)) && intersected_point.y >= (min(l[0].x, l[1].x)))
				{
					// check if point is out side (upper side or lower side) of box or inside the box
					if (intersected_point.y > window.y)
					{
						ans_obj.points.push_back(grid_line[1]);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
					else if (intersected_point.y < 0)
					{
						ans_obj.points.push_back(grid_line[0]);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
					else
					{
						ans_obj.points.push_back(intersected_point);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
				}
			}
			// grid line y = 0
			grid_line = {Point(0, 0), Point(window.x, 0)};
			if (intersection_between_two_lines(l, {Point(window.x, 0), Point(window.x, window.y)}).second)
			{
				Point intersected_point = intersection_between_two_lines({this->points[pi[i]], this->points[pi[(i + 1) % pi.size()]]}, grid_line).first;
				if (intersected_point.x <= (max(l[0].x, l[1].x)) && intersected_point.y <= (max(l[0].y, l[1].y)) && intersected_point.x >= (min(l[0].x, l[1].x)) && intersected_point.y >= (min(l[0].x, l[1].x)))
				{
					// check if point is out side (upper side or lower side) of box or inside the box
					if (intersected_point.x > window.x)
					{
						ans_obj.points.push_back(grid_line[1]);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
					else if (intersected_point.x < 0)
					{
						ans_obj.points.push_back(grid_line[0]);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
					else
					{
						ans_obj.points.push_back(intersected_point);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
				}
			}
			// grid line y = max
			grid_line = {Point(0, window.y), Point(window.x, window.y)};
			if (intersection_between_two_lines(l, {Point(window.x, 0), Point(window.x, window.y)}).second)
			{
				Point intersected_point = intersection_between_two_lines({this->points[pi[i]], this->points[pi[(i + 1) % pi.size()]]}, grid_line).first;
				if (intersected_point.x <= (max(l[0].x, l[1].x)) && intersected_point.y <= (max(l[0].y, l[1].y)) && intersected_point.x >= (min(l[0].x, l[1].x)) && intersected_point.y >= (min(l[0].x, l[1].x)))
				{
					// check if point is out side (upper side or lower side) of box or inside the box
					if (intersected_point.x > window.x)
					{
						ans_obj.points.push_back(grid_line[1]);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
					else if (intersected_point.x < 0)
					{
						ans_obj.points.push_back(grid_line[0]);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
					else
					{
						ans_obj.points.push_back(intersected_point);
						ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
					}
				}
			}

			// second point inside_frame ?
			if (this->points[pi[(i + 1) % pi.size()]].x <= window.x && this->points[pi[(i + 1) % pi.size()]].x >= 0 && this->points[pi[(i + 1) % pi.size()]].y <= window.y && this->points[pi[(i + 1) % pi.size()]].y >= 0)
			{
				ans_obj.points.push_back(this->points[pi[(i + 1) % pi.size()]]);
				ans_obj.polygons.back().push_back(ans_obj.points.size() - 1);
			}
		}
	}
	
	return ans_obj;
}
Object2D Object2D::clip_object_2(const Point &window)
{
	Object2D ans_obj;
	vector<int> p1,  p2, p3, p4;
	vector<Point> points1, points2, points3, points4;
	ans_obj.colors = this->colors;

	// calculate x_max, x_min, y_max, y_min
	float x_max = INT_MIN, x_min = INT_MAX, y_max = INT_MIN, y_min = INT_MAX;

	for (auto &pi : this->polygons)
	{
		// // optimization for polygon fully inside frame 
		for (auto&pts:pi){
			x_max = max (this->points[pts].x, x_max);
			x_min = min(this->points[pts].x, x_min);
			y_max = max(this->points[pts].y, y_max);
			y_min = min(this->points[pts].y, y_min);
		}
		if (x_min >= 0 && x_max <= window.x && y_min >= 0 && y_max <= window.y){
			// for (auto &i : pi)
			// {
			// 	i += ans_obj.points.size();
			// }
			ans_obj.polygons.push_back(pi);
			for (auto &pts : pi)
			{
				ans_obj.points.push_back(this->points[pts]);
			}
			continue;
		}
		// ---

		p1 = {};
		points1 = {};
		// for line x = 0
		for (int i = 0; i < pi.size(); i++){
			Point s = this->points[pi[i]], e = this->points[pi[(i + 1)%pi.size()]];
			pair<Point, bool> inters = intersection_between_two_lines({s,e},{Point(0,0),Point(0,window.y)});
			
			if (min(s.x, e.x) >= 0){
				temp_insert_point(points1, s, p1);
				temp_insert_point(points1, e, p1);
			}
			else if (inters.second && max(s.x,e.x) >= 0 && min(s.x,e.x) <= 0){
				if (s.x > 0){
					temp_insert_point(points1, s, p1);
					temp_insert_point(points1, inters.first, p1);
				}
				else{
					temp_insert_point(points1, inters.first, p1);
					temp_insert_point(points1, e, p1);
				}
			}
		}

		p2 = {};
		points2 = {};
		// for line x = max
		for (int i = 0; i < p1.size(); i++)
		{
			Point s = points1[p1[i]], e = points1[p1[(i + 1) % p1.size()]];
			pair<Point, bool> inters = intersection_between_two_lines({s, e}, {Point(window.x, 0), Point(window.x, window.y)});
			
			if (max(s.x, e.x) <= window.x)
			{
				temp_insert_point(points2, s, p2);
				temp_insert_point(points2, e, p2);
			}
			else if (inters.second && max(s.x, e.x) >= window.x && min(s.x, e.x) <= window.x)
			{
				if (s.x < window.x)
				{
					temp_insert_point(points2, s, p2);
					temp_insert_point(points2, inters.first, p2);
				}
				else
				{
					temp_insert_point(points2, inters.first, p2);
					temp_insert_point(points2, e, p2);
				}
			}
		}

		p3 = {};
		points3 = {};
		// for line y = 0
		for (int i = 0; i < p2.size(); i++)
		{
			Point s = points2[p2[i]], e = points2[p2[(i + 1) % p2.size()]];
			pair<Point, bool> inters = intersection_between_two_lines({s, e}, {Point(0, 0), Point(window.x, 0)});

			if (min(s.y, e.y) >= 0)
			{
				temp_insert_point(points3, s, p3);
				temp_insert_point(points3, e, p3);
			}
			else if (inters.second && max(s.y, e.y) >= 0 && min(s.y, e.y) <= 0)
			{
				if (s.y > 0)
				{
					temp_insert_point(points3, s, p3);
					temp_insert_point(points3, inters.first, p3);
				}
				else
				{
					temp_insert_point(points3, inters.first, p3);
					temp_insert_point(points3, e, p3);
				}
			}
		}


		p4 = {};
		points4 = {};
		// for line y = max
		for (int i = 0; i < p3.size(); i++)
		{
			Point s = points3[p3[i]], e = points3[p3[(i + 1) % p3.size()]];
			pair<Point, bool> inters = intersection_between_two_lines({s, e}, {Point(0, window.y), Point(window.x, window.y)});
			if (max(s.y, e.y) <= window.y)
			{
				temp_insert_point(points4, s, p4);
				temp_insert_point(points4, e, p4);
			}
			else if (inters.second && max(s.y, e.y) >= window.y && min(s.y, e.y) <= window.y)
			{
				if (s.y < window.y)
				{
					temp_insert_point(points4, s, p4);
					temp_insert_point(points4, inters.first, p4);
				}
				else
				{
					temp_insert_point(points4, inters.first, p4);
					temp_insert_point(points4, e, p4);
				}
			}
		}

		for (auto&i:p4){
			i += ans_obj.points.size();
		}
		ans_obj.polygons.push_back(p4);
		for (auto&i:points4){
			ans_obj.points.push_back(i);
		}
	}


	return ans_obj;
}

void mid_point_line_draw(Screen_memory &temp_sm, Point &start, Point &end, const RGBcolor &line_color, int erase_mode) {
	
	Point current = start;

	if ((current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0))
	{
		start = end;
		end = current;
		current = start;

		if ((current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0))
		{
			// finding intersection
			return;
		}
	}

	temp_sm[current.y][current.x] = line_color;
	
	if (floor(start.x) == floor(end.x) && floor(start.y) == floor(end.y)){
		return;
	}
	float dx = end.x - start.x;
	float dy = end.y - start.y;
	if (dx == 0 && dy == 0) return;
	float d = dy - (dx / 2.f);
	float x_step, y_step;
	x_step = (signbit(dx) == 0) ? 1 : -1;
	y_step = (signbit(dy) == 0) ? 1 : -1;
	if (abs(dy) <= abs(dx)) {
		d = (dy * (x_step)) - ((dx / 2.f) * (y_step));
		while (true) {
			current.x += x_step;
			if ((d * (y_step) * (x_step)) < 0) {
				d += dy * (x_step);
			}
			else {
				d += dy * (x_step)-dx * (y_step);
				current.y += y_step;
			}

			if (current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0)
			{
				break;
			}
			temp_sm[current.y][current.x] = line_color;
			
			if (floor(current.x) == floor(end.x)) {
				break;
			}

		}
	}
	else {
		d = (dy / 2.f) * (x_step)-dx * (y_step);
		while (true) {
			current.y += y_step;
			if (d * (y_step) * (x_step) > 0) {
				d -= dx * (y_step);
			}
			else {
				d += dy * (x_step)-dx * (y_step);
				current.x += x_step;
			}

			if (current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0)
			{
				break;
			}
			temp_sm[current.y][current.x] = line_color;
			if (floor(current.y) == floor(end.y)) {
				break;
			}
		}
	}
}

pair<Point,int> intersection_between_segments(vector<Point> v){
	Point ans;
	float delta = 0.01;

	Point l1p1 = v[0], l1p2 = v[1], l2p1 = v[2], l2p2 = v[3];
	float d_l1_y = l1p2.y - l1p1.y, d_l1_x = l1p2.x - l1p1.x, d_l2_y = l2p2.y - l2p1.y, d_l2_x = l2p2.x - l2p1.x;

	if (d_l1_x == 0 && d_l2_x != 0) {
		ans.x = l1p1.x;
		ans.y = ((d_l2_y / d_l2_x) * (ans.x - l2p1.x)) + l2p1.y;
	}
	else if (d_l1_x != 0 && d_l2_x == 0) {
		ans.x = l2p1.x;
		ans.y = ((d_l1_y / d_l1_x) * (ans.x - l1p1.x)) + l1p1.y;
	}
	else {
		float m1 = (d_l1_y) / (d_l1_x), m2 = (d_l2_y) / (d_l2_x);

		if (m1 == m2) {
			return make_pair(Point(), 0);
		}
		else {
			ans.x = ((l2p1.y - (m2 * l2p1.x)) + ((m1 * l1p1.x) - l1p1.y)) / (m1 - m2);
			ans.y = ((d_l1_y / d_l1_x) * (ans.x - l1p1.x)) + l1p1.y;
		}
	}
	// if (ans.y >= 0 && ans.y < MAIN_VIEW_HEIGHT && ans.x >= 0 && ans.x < MAIN_VIEW_WIDTH){
	// 	sm[floor(ans.y)][floor(ans.x)] = RGBcolor(0, 0, 255, 255);
	// }

	if (abs(ans.x - 0) < delta ){
		ans.x = 0;
	}
	if (abs(ans.y - 0) < delta){
		ans.y = 0;
	}
	if (abs(ans.x - l2p1.x) < delta){
		ans.x = l2p1.x;
	}
	if (abs(ans.x - l2p2.x) < delta){
		ans.x = l2p2.x;
	}
	if (abs(ans.y - l2p1.y) < delta){
		ans.y = l2p1.y;
	}
	if (abs(ans.y - l2p2.y) < delta){
		ans.y = l2p2.y;
	}
	
	// cout << "---" << endl;
	// ans.print(1);
	// l2p1.print(1);
	// l2p2.print(1);
	// cout << (signbit(ans.x - l2p1.x) == signbit(l2p2.x - l2p1.x)) << endl;
	// cout << (signbit(ans.y - l2p1.y) == signbit(l2p2.y - l2p1.y)) << endl;

	if (signbit(ans.x - l2p1.x) == signbit(l2p2.x - l2p1.x) && signbit(ans.y - l2p1.y) == signbit(l2p2.y - l2p1.y)) {
		if ( (ans.x <= max(l1p1.x, l1p2.x) && ans.x >= min(l1p1.x, l1p2.x) && ans.y <= max(l1p1.y, l1p2.y) && ans.y >= min(l1p1.y, l1p2.y))
		&&   (ans.x <= max(l2p1.x, l2p2.x) && ans.x >= min(l2p1.x, l2p2.x) && ans.y <= max(l2p1.y, l2p2.y) && ans.y >= min(l2p1.y, l2p2.y)) ){
			ans = Point(floor(ans.x), floor(ans.y));
			return make_pair(ans, 1);
		}
	}
	return make_pair(Point(), 0);;
}

vector <pair<vector<Point>,RGBcolor>> clip(vector <pair<vector<Point>,RGBcolor>> lines, int w, int h){
	vector <pair<vector<Point>,RGBcolor>> clip_ans;
	for (int i = 0; i < lines.size(); i++){
		int oit = clip_ans.size();
		if ((lines[i].first[0].x >= 0 && lines[i].first[0].x < w && lines[i].first[0].y >= 0 && lines[i].first[0].y < h)
			&& (lines[i].first[1].x >= 0 && lines[i].first[1].x < w && lines[i].first[1].y >= 0 && lines[i].first[1].y < h)){
				clip_ans.push_back({lines[i].first,lines[i].second});
				continue;
			}
		if ((lines[i].first[0].x >= 0 && lines[i].first[0].x < w && lines[i].first[0].y >= 0 && lines[i].first[0].y < h)
			&& (lines[i].first[1].x < 0 || lines[i].first[1].x >= w || lines[i].first[1].y < 0 || lines[i].first[1].y >= h)){
				pair<Point,int> tp;
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(0,0),Point(w-1,0)});
				if (tp.second == 1){
					clip_ans.push_back({{lines[i].first[0], tp.first},lines[i].second});
					continue;
				}
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(0,0),Point(0,h-1)});
				if (tp.second == 1){
					clip_ans.push_back({{lines[i].first[0], tp.first},lines[i].second});
					continue;
				}
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(w-1,0),Point(w-1,h-1)});
				if (tp.second == 1){
					clip_ans.push_back({{lines[i].first[0], tp.first},lines[i].second});
					continue;
				}
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(w-1,h-1),Point(0,h-1)});
				if (tp.second == 1){
					clip_ans.push_back({{lines[i].first[0], tp.first},lines[i].second});
					continue;
				}
			}
		Point tpt = lines[i].first[0];
		lines[i].first[0] = lines[i].first[1];
		lines[i].first[1] = tpt;
		if ((lines[i].first[0].x >= 0 && lines[i].first[0].x < w && lines[i].first[0].y >= 0 && lines[i].first[0].y < h)
			&& (lines[i].first[1].x < 0 || lines[i].first[1].x >= w || lines[i].first[1].y < 0 || lines[i].first[1].y >= h)){
				pair<Point,int> tp;
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(0,0),Point(w-1,0)});
				if (tp.second == 1){
					clip_ans.push_back({{lines[i].first[0], tp.first},lines[i].second});
					continue;
				}
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(0,0),Point(0,h-1)});
				if (tp.second == 1){
					clip_ans.push_back({{lines[i].first[0], tp.first},lines[i].second});
					continue;
				}
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(w-1,0),Point(w-1,h-1)});
				if (tp.second == 1){
					clip_ans.push_back({{lines[i].first[0], tp.first},lines[i].second});
					continue;
				}
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(w-1,h-1),Point(0,h-1)});
				if (tp.second == 1){
					clip_ans.push_back({{lines[i].first[0], tp.first},lines[i].second});
					continue;
				}
		}
	
		if ((lines[i].first[0].x < 0 || lines[i].first[0].x >= w || lines[i].first[0].y < 0 || lines[i].first[0].y >= h)
			&& (lines[i].first[1].x < 0 || lines[i].first[1].x >= w || lines[i].first[1].y < 0 || lines[i].first[1].y >= h)){
				pair<Point,int> tp,tp2;
				vector<Point> temp_ans;
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(0,0),Point(w-1,0)});
				if (tp.second == 1){
					temp_ans.push_back(tp.first);
					// clip_ans.push_back({lines[i].first[0], tp.first});
				}
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(0,0),Point(0,h-1)});
				if (tp.second == 1){
					temp_ans.push_back(tp.first);
					// clip_ans.push_back({lines[i].first[0], tp.first});
				}
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(w-1,0),Point(w-1,h-1)});
				if (tp.second == 1){
					temp_ans.push_back(tp.first);
					// clip_ans.push_back({lines[i].first[0], tp.first});
				}
				tp = intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(w-1,h-1),Point(0,h-1)});
				if (tp.second == 1){
					temp_ans.push_back(tp.first);
					// clip_ans.push_back({lines[i].first[0], tp.first});
				}
				if (temp_ans.size() == 2){
					clip_ans.push_back({temp_ans,lines[i].second});
				}
		}
		if (clip_ans.size() == oit){
			// cout << 4 << endl;
			// lines[i].first[0].print(1);
			// lines[i].first[1].print(1);
			// intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(0,0),Point(w-1,0)}).first.print(1);
			// intersection_between_segments({lines[i].first[0],lines[i].first[1],Point(0,h-1),Point(w-1,h-1)}).first.print(1);
		}
	}
	return clip_ans;
}

vector<Point> mid_point_line_draw_c(Screen_memory &actual_sm, Point start, Point end, const RGBcolor &line_color, int erase_mode, Screen_memory &temp_sm)
{
	vector<Point> ans(0);
	Point current = start;

	if ((current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0))
	{
		start = end;
		end = current;
		current = start;

		if ((current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0))
		{
			return ans;
		}
	}
	ans.push_back(current);
	temp_sm[current.y][current.x] = line_color;

	if (floor(start.x) == floor(end.x) && floor(start.y) == floor(end.y))
	{
		return ans;
	}
	float dx = end.x - start.x;
	float dy = end.y - start.y;
	if (dx == 0 && dy == 0)
		return ans;
	float d = dy - (dx / 2.f);
	float x_step, y_step;
	x_step = (signbit(dx) == 0) ? 1 : -1;
	y_step = (signbit(dy) == 0) ? 1 : -1;
	if (abs(dy) <= abs(dx))
	{
		d = (dy * (x_step)) - ((dx / 2.f) * (y_step));
		while (true)
		{
			current.x += x_step;
			if ((d * (y_step) * (x_step)) < 0)
			{
				d += dy * (x_step);
			}
			else
			{
				d += dy * (x_step)-dx * (y_step);
				current.y += y_step;
			}

			if (current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0)
			{
				break;
			}
			ans.push_back(current);
			temp_sm[current.y][current.x] = line_color;
			if (floor(current.x) == floor(end.x))
			{
				break;
			}
		}
	}
	else
	{
		d = (dy / 2.f) * (x_step)-dx * (y_step);
		while (true)
		{
			current.y += y_step;
			if (d * (y_step) * (x_step) > 0)
			{
				d -= dx * (y_step);
			}
			else
			{
				d += dy * (x_step)-dx * (y_step);
				current.x += x_step;
			}

			if (current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0)
			{
				break;
			}
			ans.push_back(current);
			temp_sm[current.y][current.x] = line_color;
			if (floor(current.y) == floor(end.y))
			{
				break;
			}
		}
	}
	return ans;
}

vector<Point> mid_point_line_draw_c(Point start, Point end, const RGBcolor &line_color, int erase_mode, Screen_memory &temp_sm)
{
	vector<Point> ans(0);
	Point current = start;

	if ((current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0))
	{
		start = end;
		end = current;
		current = start;
		if ((current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0))
		{
			return ans;
		}
	}
	ans.push_back(current);
	temp_sm[current.y][current.x] = line_color;

	if (floor(start.x) == floor(end.x) && floor(start.y) == floor(end.y))
	{
		return ans;
	}
	float dx = end.x - start.x;
	float dy = end.y - start.y;
	if (dx == 0 && dy == 0)
		return ans;
	float d = dy - (dx / 2.f);
	float x_step, y_step;
	x_step = (signbit(dx) == 0) ? 1 : -1;
	y_step = (signbit(dy) == 0) ? 1 : -1;
	if (abs(dy) <= abs(dx))
	{
		d = (dy * (x_step)) - ((dx / 2.f) * (y_step));
		while (true)
		{
			current.x += x_step;
			if ((d * (y_step) * (x_step)) < 0)
			{
				d += dy * (x_step);
			}
			else
			{
				d += dy * (x_step)-dx * (y_step);
				current.y += y_step;
			}

			if (current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0)
			{
				break;
			}
			ans.push_back(current);
			temp_sm[current.y][current.x] = line_color;
			if (floor(current.x) == floor(end.x))
			{
				break;
			}
		}
	}
	else
	{
		d = (dy / 2.f) * (x_step)-dx * (y_step);
		while (true)
		{
			current.y += y_step;
			if (d * (y_step) * (x_step) > 0)
			{
				d -= dx * (y_step);
			}
			else
			{
				d += dy * (x_step)-dx * (y_step);
				current.x += x_step;
			}

			if (current.y >= temp_sm.size() || current.x >= temp_sm[0].size() || current.y < 0 || current.x < 0)
			{
				break;
			}
			ans.push_back(current);
			temp_sm[current.y][current.x] = line_color;
			if (floor(current.y) == floor(end.y))
			{
				break;
			}
		}
	}
	return ans;
}

vector<vector<Point>> row_fill(vector<Point> polygon_points, RGBcolor fill_color, Screen_memory &temp_sm){
	vector<vector<Point>> ans;
	int min_x = INT_MAX, max_x = INT_MIN, min_y = INT_MAX, max_y = INT_MIN;
	for (int i = 0; i < polygon_points.size(); i++)
	{
		polygon_points[i].x = floor(polygon_points[i].x);
		polygon_points[i].y = floor(polygon_points[i].y);
		if (polygon_points[i].x < min_x)
			min_x = polygon_points[i].x;
		if (polygon_points[i].x > max_x)
			max_x = polygon_points[i].x;
		if (polygon_points[i].y < min_y)
			min_y = polygon_points[i].y;
		if (polygon_points[i].y > max_y)
			max_y = polygon_points[i].y;
	}

	Point start_p, end_p, next_p;
	vector<int> yv(max_y - min_y + 1, 0);
	vector<float> empty_v(0);
	vector<vector<float>> all_points(max_y - min_y + 1, empty_v);
	for (int i = 0; i < polygon_points.size(); i++)
	{
		start_p = polygon_points[(i - 1 + polygon_points.size()) % polygon_points.size()];
		end_p = polygon_points[i];
		int j = start_p.y;
		float x;
		while (j != end_p.y)
		{
			if (start_p.y > end_p.y)
			{
				j--;
			}
			else
			{
				j++;
			}
			if (j == end_p.y)
				break;
			x = ((((float)j - start_p.y) * (end_p.x - start_p.x)) / (end_p.y - start_p.y)) + start_p.x;
			all_points[j - min_y].push_back(x);
		}

		next_p = polygon_points[(i + 1) % polygon_points.size()];
		if (signbit(end_p.y - next_p.y) != signbit(end_p.y - start_p.y))
		{
			all_points[end_p.y - min_y].push_back(end_p.x);
		}
	}
	for (int i = 0; i < all_points.size(); i++)
	{
		sort(all_points[i].begin(), all_points[i].end());

		for (int j = 0; j < all_points[i].size(); j++)
		{
			if (j % 2 == 0)
			{
				ans.push_back(mid_point_line_draw_c(Point(ceil(all_points[i][j]), i + min_y), Point(floor(all_points[i][j + 1]), i + min_y), fill_color, 0, temp_sm));
			}
		}
	}
	return ans;
}

void row_fill_direct_to_screen(vector<Point> polygon_points, RGBcolor fill_color, Screen_memory &temp_sm)
{
	vector<vector<Point>> ans;
	int min_x = INT_MAX, max_x = INT_MIN, min_y = INT_MAX, max_y = INT_MIN;
	for (int i = 0; i < polygon_points.size(); i++)
	{
		polygon_points[i].x = floor(polygon_points[i].x);
		polygon_points[i].y = floor(polygon_points[i].y);
		if (polygon_points[i].x < min_x)
			min_x = polygon_points[i].x;
		if (polygon_points[i].x > max_x)
			max_x = polygon_points[i].x;
		if (polygon_points[i].y < min_y)
			min_y = polygon_points[i].y;
		if (polygon_points[i].y > max_y)
			max_y = polygon_points[i].y;
	}

	Point start_p, end_p, next_p;
	vector<int> yv(max_y - min_y + 1, 0);
	vector<float> empty_v(0);
	vector<vector<float>> all_points(max_y - min_y + 1, empty_v);
	for (int i = 0; i < polygon_points.size(); i++)
	{
		start_p = polygon_points[(i - 1 + polygon_points.size()) % polygon_points.size()];
		end_p = polygon_points[i];
		int j = start_p.y;
		float x;
		while (j != end_p.y)
		{
			if (start_p.y > end_p.y)
			{
				j--;
			}
			else
			{
				j++;
			}
			if (j == end_p.y)
				break;
			x = ((((float)j - start_p.y) * (end_p.x - start_p.x)) / (end_p.y - start_p.y)) + start_p.x;
			all_points[j - min_y].push_back(x);
		}

		next_p = polygon_points[(i + 1) % polygon_points.size()];
		if (signbit(end_p.y - next_p.y) != signbit(end_p.y - start_p.y))
		{
			all_points[end_p.y - min_y].push_back(end_p.x);
		}
	}
	for (int i = 0; i < all_points.size(); i++)
	{
		sort(all_points[i].begin(), all_points[i].end());

		for (int j = 0; j < all_points[i].size(); j++)
		{
			if (j % 2 == 0)
			{
				Point start = Point(ceil(all_points[i][j]), i + min_y), end = Point(floor(all_points[i][j + 1]), i + min_y);

				mid_point_line_draw(temp_sm, start, end, fill_color, 0);
			}
		}
	}
}

// 