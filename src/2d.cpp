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

Screen_memory::Screen_memory(int w, int h){
    RGBcolor r = RGBcolor(255, 225, 0, 255);
    tempv = vector<RGBcolor>(w, r);
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

Object2D::Object2D()
{
	center = Point();
}

void mid_point_line_draw(Screen_memory &temp_sm, const Point &start, const Point &end, const RGBcolor &line_color, int erase_mode) {
	
	Point current = start;
	
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
