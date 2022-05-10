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
#include <bits/stdc++.h>

// 
#include <input_handler.h>
#include <3d.h>
#include <2d.h>
#include <myHeader/Object.h>
#include <myHeader/values.h>

using namespace std;

// global vars
SDL_Renderer *renderer;
SDL_Window* window;
SDL_Event event;
bool isRunning = true;
Screen_memory sm, init_sm, temp_sm;
vector<Point3D> all_points;
Point3D view_point,watch_direction;
float watch_theta = 0, watch_phi = PI, turning_speed = 0, std_turning_speed = 0.02;
Ray view_ray,up;
Frame view_window;
vector<vector<Point3D>> intersected_points;
vector <pair<vector<Point3D>,RGBcolor>> lines;
// vector<vector<Point>> screen_lines, clipped_lines;
vector <pair<vector<Point>,RGBcolor>> screen_lines, clipped_lines;

TTF_Font* f;
Point text_slot = Point(10 + MAIN_VIEW_WIDTH, 10), init_text_slot = Point(10 + MAIN_VIEW_WIDTH, 10);
int font_h = 30;
long long int new_time,time_diff,old_time;
int frame_rate = STD_FPS,frame_count=0;
float std_movement_speed = 0.11,movement_speed = 0;

map< string, vector <pair<vector<Point3D>,RGBcolor>>> objects;
vector<Object> objs;
Object2D tempobj;
vector<Object2D> obj_2d;
vector<vector<float>> distance_memory(MAIN_VIEW_HEIGHT, vector<float>(MAIN_VIEW_WIDTH, 0)), init_dm(MAIN_VIEW_HEIGHT, vector<float>(MAIN_VIEW_WIDTH, 0));
vector<Point> points_tmp;
vector<vector<Point>> fill_points;
long long int tempt;

int movement_mode = 0;

void basic_inits(){

	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow("Hello SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	// text initialisation
	TTF_Init();
	if (TTF_Init() < 0)
	{
		cout << "Error initializing SDL_ttf: " << TTF_GetError() << endl;
	}
	f = TTF_OpenFont(FONT_PATH, 200);
	if (!f)
	{
		cout << "Failed to load font: " << TTF_GetError() << endl;
	}

	sm = Screen_memory(MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT);
	init_sm = Screen_memory(MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT, RGBcolor(0, 0, 0));
	temp_sm = Screen_memory(MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT);
}

void show_text(string s, Point p, float size, TTF_Font* f) {
	if (s.size() == 0) return;
	SDL_Surface* text;
	SDL_Texture* text_texture;
	SDL_Color color = { 255, 255, 255 };
	SDL_Rect dest;
	Point startpoint__to_display, endpoint_to_display;
	string initial_point_string;
	int font_w;

	text = TTF_RenderText_Solid(f, s.c_str(), color);
	font_w = (float)font_h * ((float)text->w / (float)text->h);
	// TTF_SizeText();

	if (!text) {
		cout << "Failed to render text: " << TTF_GetError() << endl;
	}

	text_texture = SDL_CreateTextureFromSurface(renderer, text);

	dest = { (int)floor(p.x), (int)floor(p.y), font_w, font_h };
	SDL_RenderCopy(renderer, text_texture, NULL, &dest);

	SDL_DestroyTexture(text_texture);
}

void text_overlay() {
	
	std::string text_string;
	// text 1		
	show_text("movement_speed: " + to_string(movement_speed), text_slot, font_h, f);
	text_slot.y += font_h;
	
	//text 2
	show_text("Frame rate: " + to_string( (int)ceil(frame_rate)), text_slot, font_h, f);
	text_slot.y += font_h;

	//text 3
	show_text("time : " + to_string(tempt), text_slot, font_h, f);
	text_slot.y += font_h;

	//text 4
	string stringx, stringy, stringz;
	stringstream ss,ssy,ssz;
	ss << std::fixed << std::setprecision(2) << view_point.x;
	stringx = ss.str();
	ss.clear();
	ssy << std::fixed << std::setprecision(2) << view_point.y;
	stringy = ssy.str();
	// ss.clear();
	ssz << std::fixed << std::setprecision(2) << view_point.z;
	stringz = ssz.str();

	// show_text("view point x : " + to_string(round(view_point.x)) + " y : " + to_string(round(view_point.y)) + " z : " + to_string(round(view_point.x)), text_slot, font_h, f);
	show_text("view point x : " + stringx + " y : " + stringy + " z : " + stringz, text_slot, font_h, f);
	text_slot.y += font_h;

	// text 5
	string s = (movement_mode == 0 ? "free" : "origin");
	show_text("movement mode : " + (string)(movement_mode == 0 ? "free" : "origin"), text_slot, font_h, f);
	text_slot.y += font_h;

	// text 6
	show_text("movement mode : " + to_string(movement_mode), text_slot, font_h, f);
	text_slot.y += font_h;

	text_slot = init_text_slot;
}

void get_old_objects(){
	// objects.insert({"cube", Cube().lines});
	// objects.insert({"axes", Axes().lines});
	

	for (auto &i : objects)
	{
		for (int j = 0; j < i.second.size(); j++)
		{
			lines.push_back(i.second[j]);
		}
	}
}

void set_objects(vector<Object> &objv ) {
	// request objects
	objv.push_back(newCube());
	// objv.push_back(newPlane());

	obj_2d = vector<Object2D>(objv.size());
}

void process_projection(Object obj, int index){
	
	// 2d object 'obj' to 2d object 'tempobj'
	tempobj.center = find_intersection(view_window, Ray(view_point, obj.center)).to_2d(view_window.p1, view_window.p2, view_window.p4, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT);
	tempobj.edges = obj.edges;
	tempobj.polygons = obj.polygons;
	tempobj.colors = obj.colors;
	tempobj.points = vector<Point>(0);
	for (int i = 0; i < obj.points.size(); i++){
		tempobj.points.push_back( find_intersection(view_window, Ray(view_point, obj.points[i] )).to_2d(view_window.p1, view_window.p2, view_window.p4, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT) );
	}
	
	// seeing if first calculate or recalculate
	if (obj_2d.size() == objs.size()){
		obj_2d[index] = tempobj;
	}
	else{
		obj_2d.push_back(tempobj);
	}
	
	// comparing distance and writing to sm (screen memory)
	for (int obji = 0; obji < obj_2d.size(); obji++)
	{
		// for each 2d object: 
		for (int pi = 0; pi < obj_2d[obji].polygons.size(); pi++)
		{ 
			// for each polygon in the 2d object: 
			
			fill_points = vector<vector<Point>>(0);

			// lists of points for filling the 2d polygon (can't pass direct polygon as it has point index instead of actual points)
			points_tmp = vector<Point>(0);

			for (int si = 0; si < obj_2d[obji].polygons[pi].size(); si++)
			{
				// for each point 

				// inserting all points of edge  
				fill_points.push_back(
				 mid_point_line_draw_c(sm, obj_2d[obji].points[obj_2d[obji].polygons[pi][si]],
									  obj_2d[obji].points[obj_2d[obji].polygons[pi][(si + 1) % obj_2d[obji].polygons[pi].size()]],
									  obj_2d[obji].colors[pi], 0, temp_sm) 
				);
				points_tmp.push_back(obj_2d[obji].points[obj_2d[obji].polygons[pi][si]]);
			}

			if (objs[obji].normals[pi].dot_product(view_ray.p2 - view_ray.p1) < 0){
				points_to_pixel(fill_points, view_point, view_window, objs[obji], pi, sm, distance_memory);
				fill_points = row_fill(points_tmp, obj_2d[obji].colors[pi], temp_sm);
				points_to_pixel(fill_points, view_point, view_window, objs[obji], pi, sm, distance_memory);
			}
			// SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0XFF);
			// SDL_RenderClear(renderer);
			// temp_sm.renderer(renderer);
			// SDL_RenderPresent(renderer);
			// int j;
			// cin >> j;
			temp_sm = init_sm;
		}
	}
}

void process_projection_using_normal(vector<Object> obj_vector)
{
	// 2d object 'obj' to 2d object 'tempobj'
	int index=0;
	for(auto&obj:obj_vector){
		// seeing if first calculate or recalculate
		obj_2d[index] = obj.object_to_2d(view_window, view_point);
		index++;
	}

	// // clipping the 2d objects
	// for (auto&obj_2d_i:obj_2d){
	// 	obj_2d_i = obj_2d_i.clip_object(Point(MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT));
	// }
	

	// writing to sm by filling 2d polygons (screen memory)
	for (int obji = 0; obji < obj_2d.size(); obji++)
	{
		// for each 2d object:
		for (int pi = 0; pi < obj_2d[obji].polygons.size(); pi++)
		{
			// for each polygon in the 2d object:

			// if polygon not facing us, skip
			if (objs[obji].normals[pi].dot_product( view_point - objs[obji].points[ objs[obji].polygons[pi][0] ] ) < 0){
				continue;
			}
			
			fill_points = vector<vector<Point>>(0);
			// lists of points for filling the 2d polygon (can't pass direct polygon as it has point index instead of actual points)
			points_tmp = vector<Point>(0);

			for (int si = 0; si < obj_2d[obji].polygons[pi].size(); si++)
			{
				// for each point, inserting into the vector for the fill_polygon
				obj_2d[obji].points[obj_2d[obji].polygons[pi][si]].print(1);
				points_tmp.push_back(obj_2d[obji].points[obj_2d[obji].polygons[pi][si]]);
			}
			
			row_fill_direct_to_screen(points_tmp, obj_2d[obji].colors[pi], sm);
			
		}
	}
}

void recalculate()
{
	sm = init_sm;
	distance_memory = init_dm;

	if (movement_mode == 0){
		view_ray = Ray(view_point, view_point + Point3D(watch_theta, watch_phi));
		view_ray.unitize();
		view_ray.p1 = view_ray.p1 + view_point;
		view_ray.p2 = view_ray.p2 + view_point;
		up = Ray(view_ray.p1, view_ray.p1 + Point3D(watch_theta + ((PI) / ((float)2)), watch_phi));
	}
	else if (movement_mode == 1){
		view_ray = Ray(view_point, Point3D(0,0,0));
		view_ray.unitize();
		up = Ray(view_point, view_point + Point3D( view_ray.p2.theta() + PI/2, view_ray.p2.phi() ) );
		view_ray.p1 = view_ray.p1 + view_point;
		view_ray.p2 = view_ray.p2 + view_point;
	}

	// set view window
	view_window = Frame(view_ray, 1, 1, up);

	// project in_vector to view_window and add to view_lines_vector
	for (int i = 0; i < lines.size(); i++)
	{

		Point3D lp1 = lines[i].first[0], lp2 = lines[i].first[1];
		// case 1 : full line behind frame
		if (cos(view_ray.unitize().p2.angle_between(lp1 - view_window.p1)) <= 0 &&
			cos(view_ray.unitize().p2.angle_between(lp2 - view_window.p1)) <= 0)
		{
			lp1 = (view_window.p1 * 2) - view_window.p2;
			lp2 = (view_window.p1 * 2) - view_window.p2;
		}
		// case 2 : first point behind frame
		if (cos(view_ray.unitize().p2.angle_between(lp1 - view_window.p1)) < 0)
		{
			lp1 = find_intersection(view_window, Ray(lp1, lp2));
		}
		// case 3 : second
		if (cos(view_ray.unitize().p2.angle_between(lp2 - view_window.p1)) < 0)
		{
			lp2 = find_intersection(view_window, Ray(lp1, lp2));
		}

		intersected_points[i] = {find_intersection(view_window, Ray(view_point, lp1)),
								 find_intersection(view_window, Ray(view_point, lp2))};

		float x, y, l, theta, theta_y;
		theta = (intersected_points[i][0] - view_window.p1).angle_between(view_window.p2 - view_window.p1);
		theta_y = (intersected_points[i][0] - view_window.p1).angle_between(view_window.p4 - view_window.p1);
		l = (intersected_points[i][0] - view_window.p1).len();
		x = ((float)MAIN_VIEW_WIDTH) * ((l * cos(theta)) /
										((view_window.p2 - view_window.p1).len()));
		y = ((float)MAIN_VIEW_HEIGHT) * ((l * cos(theta_y)) /
										 ((view_window.p4 - view_window.p1).len()));

		if (intersected_points[i][0].isequal(view_window.p1))
		{
			x = 0;
			y = 0;
		}
		Point p1 = Point(floor(x), floor(y));

		theta = (intersected_points[i][1] - view_window.p1).angle_between(view_window.p2 - view_window.p1);
		theta_y = (intersected_points[i][1] - view_window.p1).angle_between(view_window.p4 - view_window.p1);
		l = (intersected_points[i][1] - view_window.p1).len();
		x = ((float)MAIN_VIEW_WIDTH) * ((l * cos(theta)) /
										((view_window.p2 - view_window.p1).len()));

		y = ((float)MAIN_VIEW_HEIGHT) * ((l * cos(theta_y)) /
										 ((view_window.p4 - view_window.p1).len()));
		if (intersected_points[i][1].isequal(view_window.p1))
		{
			x = 0;
			y = 0;
		}

		screen_lines[i] = {{p1, Point(floor(x), floor(y))}, lines[i].second};
	}
	// clip partial lines in 2d and add to in vector
	clipped_lines = clip(screen_lines, MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT);
	// clipped_lines = screen_lines;

	for (int obji = 0; obji < objs.size(); obji++)
	{
	}
	long long t1 = SDL_GetTicks64();
	process_projection_using_normal(objs);
	long long t2 = SDL_GetTicks64();
	cout << "TIME : " << (t2 - t1) << endl;
}

void handle_input()
{
	const Uint8 *p = SDL_GetKeyboardState(NULL);
	if (p[SDL_SCANCODE_W])
	{
		view_point = view_point + (view_ray.p2 - view_ray.p1) * movement_speed;
		recalculate();
	}
	if (p[SDL_SCANCODE_A])
	{
		view_point = view_point - (view_ray.p2 - view_ray.p1).cross_product(up.p2 - up.p1) * movement_speed;
		recalculate();
	}
	if (p[SDL_SCANCODE_S])
	{
		view_point = view_point - (view_ray.p2 - view_ray.p1) * movement_speed;
		recalculate();
	}
	if (p[SDL_SCANCODE_D])
	{
		view_point = view_point + (view_ray.p2 - view_ray.p1).cross_product(up.p2 - up.p1) * movement_speed;
		recalculate();
	}
	if (p[SDL_SCANCODE_KP_PLUS])
	{
		std_movement_speed += 0.1;
	}
	if (p[SDL_SCANCODE_KP_MINUS])
	{
		std_movement_speed -= 0.1;
		if (movement_speed < 0)
		{
			movement_speed = 0;
		}
	}
	if (p[SDL_SCANCODE_UP])
	{
		watch_theta += turning_speed;
		recalculate();
	}
	if (p[SDL_SCANCODE_DOWN])
	{
		watch_theta -= turning_speed;
		recalculate();
	}
	if (p[SDL_SCANCODE_LEFT])
	{
		watch_phi += turning_speed;
		recalculate();
	}
	if (p[SDL_SCANCODE_RIGHT])
	{
		watch_phi -= turning_speed;
		recalculate();
	}
	if (p[SDL_SCANCODE_PAGEUP]){
		view_point = view_point +( (up.p2-up.p1) * movement_speed);
		recalculate();
	}
	if (p[SDL_SCANCODE_PAGEDOWN]){
		view_point = view_point - ((up.p2 - up.p1) * movement_speed);
		recalculate();
	}
	if (p[SDL_SCANCODE_M]){
		movement_mode = (movement_mode + 1)%2;
	}
}

int main(int argv, char** args){

	basic_inits();

	set_objects(objs);
	
	// Old Objects
	get_old_objects();
	
	//viewing position
	view_point = Point3D(0,0,5);
	
	// viewing orientation
	// watch_phi += 0.78 + PI/2;
	// watch_theta = -0.76;
	view_ray = Ray(view_point, view_point + Point3D(watch_theta,watch_phi));
	view_ray.unitize();
	view_ray.p1 = view_ray.p1 + view_point;
	view_ray.p2 = view_ray.p2 + view_point;
	Point3D(watch_theta,watch_phi).print(1);
	up = Ray(view_ray.p1, view_ray.p1 + Point3D(watch_theta+((PI)/((float)2)),watch_phi) );
	
	// set view window
	view_window = Frame(view_ray, 1, 1, up);

	// project in_vector to view_window and add to view_lines_vector
	for (int i = 0; i < lines.size(); i++){

		Point3D lp1 = lines[i].first[0], lp2 = lines[i].first[1];
		//case 1 : full line behind frame
		if (cos(view_ray.unitize().p2.angle_between(lp1 - view_window.p1)) <= 0 && 
			cos(view_ray.unitize().p2.angle_between(lp2 - view_window.p1)) <= 0){
				lp1 = (view_window.p1*2) - view_window.p2;
				lp2 = (view_window.p1*2) - view_window.p2;
			}
		//case 2 : first point behind frame
		if (cos(view_ray.unitize().p2.angle_between(lp1 - view_window.p1)) < 0){
			lp1 = find_intersection(view_window, Ray(lp1,lp2));
		}
		//case 3 : second
		if (cos(view_ray.unitize().p2.angle_between(lp2 - view_window.p1)) < 0){
			lp2 = find_intersection(view_window, Ray(lp1,lp2));
		}
		
		intersected_points.push_back({find_intersection(view_window, Ray(view_point, lp1)),
									find_intersection(view_window, Ray(view_point, lp2))}); 
		
		float x,y,l,theta,theta_y;
		theta = (intersected_points[i][0] - view_window.p1).angle_between(view_window.p2 - view_window.p1); 
		theta_y = (intersected_points[i][0] - view_window.p1).angle_between(view_window.p4 - view_window.p1); 
		l = (intersected_points[i][0] - view_window.p1).len();
		x = ((float)MAIN_VIEW_WIDTH) * (  (l * cos(theta))  /
								((view_window.p2 - view_window.p1).len()));
		y = ((float)MAIN_VIEW_HEIGHT) * ( (l * cos(theta_y))  /
								((view_window.p4 - view_window.p1).len()));
		
		if (intersected_points[i][0].isequal(view_window.p1)){
			x = 0;
			y = 0;
		}
		Point p1 = Point(floor(x),floor(y));
		
		theta = (intersected_points[i][1] - view_window.p1).angle_between(view_window.p2 - view_window.p1); 
		theta_y = (intersected_points[i][1] - view_window.p1).angle_between(view_window.p4 - view_window.p1); 
		l = (intersected_points[i][1] - view_window.p1).len();
		x = ((float)MAIN_VIEW_WIDTH) * (  (l * cos(theta))  /
								((view_window.p2 - view_window.p1).len()));
		
		y = ((float)MAIN_VIEW_HEIGHT) * ( (l * cos(theta_y))  /
								((view_window.p4 - view_window.p1).len()));
		if (intersected_points[i][1].isequal(view_window.p1)){
			x = 0;
			y = 0;
		}
		
		screen_lines.push_back({{p1,Point(floor(x),floor(y))},lines[i].second});
		
	}
	
	// clip partial lines in 2d and add to in vector
	clipped_lines = clip(screen_lines, MAIN_VIEW_WIDTH,MAIN_VIEW_HEIGHT);
	// clipped_lines = screen_lines;

	// 241 (2d) -17
	// 234 (2d) 331
	// 223 (2d) 38
	// 224 (2d) 408
	// 230 (2d) 88
	// 234 (2d) 425
	// 198 (2d) -3
	// 198 (2d) 294
	Point ap1(198,-3), ap2(198,294);
	// intersection_between_segments({ap1,ap2,Point(0,0),Point(400-1,0)}).first.print(1);
	// intersection_between_segments({ap1,ap2,Point(0,400-1),Point(400-1,400-1)}).first.print(1);

	recalculate();

	old_time = SDL_GetTicks64();

	// game loop
	while(isRunning){
	
		while(SDL_PollEvent(&event)){
			switch (event.type){
				case SDL_QUIT:
					isRunning = false;
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE){
						isRunning = false;
					}
					
					break;
			}
		}

		handle_input();

		// draw view lines vector on screen.
		for (int i = 0; i < clipped_lines.size(); i++){
			mid_point_line_draw(sm, clipped_lines[i].first[0], clipped_lines[i].first[1], clipped_lines[i].second, 0);
		}

		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0, 0XFF);
		SDL_RenderClear(renderer);
		sm.renderer(renderer);
		
		//text
		text_overlay();

		SDL_RenderPresent(renderer);
		
		//frame rate
		new_time = SDL_GetTicks64();
		time_diff = new_time - old_time;
		frame_count++;
		if (time_diff >= 1000){
			frame_rate = ceil(((float)(frame_count*1000))/(float)time_diff);
			frame_count = 0;
			old_time = new_time;
		}
		movement_speed = (std_movement_speed*(float)STD_FPS)/((float)frame_rate);
		turning_speed = (std_turning_speed*((float)STD_FPS))/((float)frame_rate);
	}
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}












// 