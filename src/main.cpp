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
#include <filesystem>
#include <unistd.h>

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

// theta = zy plane angle, phi = zx plane angle from z
float watch_theta = -0.33, watch_phi = 2.04, turning_speed = 0, std_turning_speed = 0.02;
Ray view_ray,up;
Frame view_window;
vector<vector<Point3D>> intersected_points;
vector <pair<vector<Point3D>,RGBcolor>> lines;
// vector<vector<Point>> screen_lines, clipped_lines;
vector <pair<vector<Point>,RGBcolor>> screen_lines, clipped_lines;

TTF_Font* f;
Point text_slot = Point(10 + MAIN_VIEW_WIDTH, 10), init_text_slot = Point(10 + MAIN_VIEW_WIDTH, 10);
int font_h;
long long int new_time,time_diff,old_time;
int frame_rate = STD_FPS,frame_count=0;
float std_movement_speed = 0.11,movement_speed = 0;

map< string, vector <pair<vector<Point3D>,RGBcolor>>> objects;
vector<Object> objs;
Object2D tempobj;
vector<Object2D> obj_2d;
vector<vector<float>> distance_memory(MAIN_VIEW_HEIGHT, vector<float>(MAIN_VIEW_WIDTH, 0)), init_dm(MAIN_VIEW_HEIGHT, vector<float>(MAIN_VIEW_WIDTH, 0));

vector<vector<Point>> fill_points;
long long int tempt;

int movement_mode = 0;

vector<LightSource> light_sources;
float ambient_light;

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
	f = TTF_OpenFont(FONT_PATH, 100);
	if (!f)
	{
		cout << "Failed to load font: " << TTF_GetError() << endl;
	}
	font_h = 34;

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

	if (!text) {
		cout << "Failed to render text: " << TTF_GetError() << endl;
	}

	text_texture = SDL_CreateTextureFromSurface(renderer, text);

	dest = { (int)floor(p.x), (int)floor(p.y), font_w, font_h };
	SDL_RenderCopy(renderer, text_texture, NULL, &dest);

	SDL_FreeSurface(text);
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
	show_text("x : " + stringx + " y : " + stringy + " z : " + stringz, text_slot, font_h, f);
	text_slot.y += font_h;

	// text 3
	show_text("Angle_zy : " + to_string((int)(watch_theta * (180 / PI))) + " Angle_zx : " + to_string((int)(watch_phi * (180 / PI))), text_slot, font_h, f);
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

void init_world_rules()
{
	// viewing position
	view_point = Point3D( -5,2,2.6);
	view_ray = Ray(view_point, view_point + Point3D(watch_theta, watch_phi));
	view_ray.unitize();
	view_ray.p1 = view_ray.p1 + view_point;
	view_ray.p2 = view_ray.p2 + view_point;
	up = Ray(view_ray.p1, view_ray.p1 + Point3D(watch_theta + ((PI) / ((float)2)), watch_phi));

	// set view window
	view_window = Frame(view_ray, 1, 1, up);

	// light
	light_sources = {
		LightSource(Point3D(-1.75,1.2,-2), 0.6),
	};
	ambient_light = 0.4;
}

void set_objects(vector<Object> &objv ) {
	// getting objects from object class

	// cubes
	// objv.push_back(newCube(2, Point3D()));
	// objv.push_back(newCube(2, Point3D(6,0,6)));
	// objv.push_back(newCube(2, Point3D(-6, 0, 6)));
	// objv.push_back(newCube(2, Point3D(6, 0, -6)));
	// objv.push_back(newCube(2, Point3D(-6, 0, -6)));

	// reading obj files
	Object cube = Object().readObject("src\\Objects\\cube.obj").rotate(0,0).scale(5).move(Point3D(15,0,-5));
	// objv.push_back(Object().readObject("src\\Objects\\low-poly-sphere.obj").move(Point3D(10,100,10)));
	Object sphere = Object().readObject("src\\Objects\\sphere.obj");
	// objv.push_back(Object().readObject("src\\Objects\\utah-teapot.obj"));
	// objv.push_back(Object().readObject("src\\Objects\\fox.obj"));
	// objv.push_back(Object().readObject("src\\Objects\\shoe.obj"));
	Object teapot = Object().readObject("src\\Objects\\teapot_clr.obj").rotate(0, 0);
	teapot.concave_object = true;
	Object torus = Object().readObject("src\\Objects\\torus.obj").rotate(-PI / 3, -PI / 4).scale(4.2).move(Point3D(-15, 0, 0));
	torus.concave_object = true;
	Object cone = Object().readObject("src\\Objects\\cone.obj").scale(100).rotate(-PI / 2,0);
	cone.concave_object = true;
	cone.move(Point3D(15, 0, 15));
	Object sword = Object().readObject("src\\Objects\\sword.obj").scale(10);
	Object timepass = Object().readObject("src\\Objects\\timepass.obj").rotate(0, 0);
	timepass.concave_object = true;
	Object sphere_s_r = Object().readObject("src\\Objects\\poly-sphere.obj").scale(5.2);
	// vector<Object> land = Object().readMultipleObject("src\\Objects\\timepass.obj");
	// cout << "size -- " << land.size() << endl;
	// torus.move(Point3D(0,0,-4));
	// objv.push_back(cube);
	// objv.push_back(sphere_s_r);
	// objv.push_back(teapot);
	// objv.push_back(torus);
	// objv.push_back(cone);
	objv.push_back(timepass);
	// objv.push_back(sphere_s_r);
	// objv.push_back(sword);
	// for (auto&m_obj_i:land){
	// 	objv.push_back(m_obj_i);
	// 	if (m_obj_i.normals.size() == 0){
	// 		cout << "hereeeeeeeeeeeeeeeeeee" << endl;
	// 	}
	// }
	
	// light sources
	for (auto&ls:light_sources){
		objv.push_back(newCube(0.2, RGBcolor(255), ls.p));
		objv.back().self_luminious = true;
	}

	// planes
	// int plane_size = 1, no_of_planes_x = 30;
	// float plane_y = -6;
	// for (int i = -no_of_planes_x; i < no_of_planes_x; i++)
	// {
	// 	for (int j = -no_of_planes_x; j < no_of_planes_x; j++)
	// 	{
	// 		// , 0, rand() % 150 + 50
	// 		objv.push_back(newPlane(plane_size, Ray(Point3D(i * plane_size, plane_y, j * plane_size), Point3D(i * plane_size, plane_y+1, j * plane_size)), RGBcolor(255)));
	// 		objv.push_back(newPlane(plane_size, Ray(Point3D(i * plane_size, plane_y, j * plane_size), Point3D(i * plane_size, plane_y-1, j * plane_size)), RGBcolor(255)));
	// 	}
	// }
	// for (float i = -5; i < 5; i++)
	// {
	// 	float z = -5.5;
	// 	for (float j = -1.5; j < 5; j++)
	// 	{
	// 		objv.push_back(newPlane(1, Ray(Point3D(i, j, z), Point3D(i, j+1, z)), RGBcolor(255)).rotate(PI/2,0));
	// 		objv.push_back(newPlane(1, Ray(Point3D(i, j, z), Point3D(i, j-1, z)), RGBcolor(255)).rotate(PI / 2, 0));
	// 	}
	// }

	// objv.push_back(newPlane(2, Ray(Point3D(0, 1, 0), Point3D(0,2,0)), RGBcolor(255)));

}

void process_projection_using_normal(vector<Object> obj_vector, vector<vector<float>> sorted_obj_index)
{
	// 2d object 'obj' to 2d object 'tempobj'
	int index=0;
	obj_2d = vector<Object2D>(objs.size());
	vector<Object> obj_3d_clipped; 
	for (auto &i_obj : sorted_obj_index)
	{
		Object o = obj_vector[i_obj[1]].clip_3d_object(view_window, view_point, view_ray);
		obj_2d[index] = o.object_to_2d(view_window, view_point, view_ray);
		// obj_2d[index].colors = o.illumination(ambient_light, light_sources);
		// obj_2d[index].vertex_intensity = o.gouraud_shading(ambient_light, light_sources);
		obj_3d_clipped.push_back(o);
		index++;
	}	

	// clipping the 2d objects
	for (auto&obj_2d_i:obj_2d){
		
		obj_2d_i = obj_2d_i.clip_object_2(Point(MAIN_VIEW_WIDTH, MAIN_VIEW_HEIGHT));
	}

	vector<vector<float>> polygon_distance;
	vector<Point> points_tmp;
	vector<float> intensities;
	// writing to sm by filling 2d polygons (screen memory)
	for (int obji = 0; obji < obj_2d.size(); obji++)
	{
		int obj_3d_i = sorted_obj_index[obji][1];

		// for each 2d object
		polygon_distance = vector<vector<float>>(0);

		for (int pi = 0; pi < obj_2d[obji].polygons.size(); pi++)
		{
			// for each polygon in the 2d object:
			if (obj_3d_clipped[obji].polygons[pi].size() == 0)
			{
				continue;
			}

			Point3D polygon_to_viewpoint = view_point - obj_3d_clipped[obji].points[obj_3d_clipped[obji].polygons[pi][0]];
			// if polygon not facing us, skip
			if (obj_3d_clipped[obji].normals[pi].dot_product(polygon_to_viewpoint) < 0)
			{
				continue;
			}

			// calculate distance between polygon to viewpoint
			polygon_distance.push_back({(polygon_to_viewpoint).len(), (float) pi});
		}

		if (objs[obj_3d_i].concave_object){
			sort(polygon_distance.begin(), polygon_distance.end());
			reverse(polygon_distance.begin(), polygon_distance.end());
		}

		int pi;
		for (int spi = 0; spi < polygon_distance.size(); spi++)
		{
			// for each SORTED polygon in the 2d object
			pi = polygon_distance[spi][1];

			fill_points = vector<vector<Point>>(0);
			// lists of points for filling the 2d polygon (can't pass direct polygon as it has point index instead of actual points)
			points_tmp = vector<Point>(0);
			intensities = vector<float>(0);

			for (int si = 0; si < obj_2d[obji].polygons[pi].size(); si++)
			{
				// inserting each point into the vector for the fill_polygon
				points_tmp.push_back(obj_2d[obji].points[obj_2d[obji].polygons[pi][si]]);
				intensities.push_back(obj_2d[obji].vertex_intensity[obj_2d[obji].polygons[pi][si]]);
			}

			// border_fill_direct_to_screen(points_tmp, RGBcolor(255), sm, intensities);
			row_fill_direct_to_screen(points_tmp, obj_2d[obji].colors[pi], sm, intensities);
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
		view_ray.set_angle_from_current_coords();
		watch_theta = view_ray.angle_yz;
		watch_phi = view_ray.angle_xz;
		view_ray.unitize();
		up = Ray(view_point, view_point + Point3D( view_ray.p2.theta() + PI/2, view_ray.p2.phi() ) );
		view_ray.p1 = view_ray.p1 + view_point;
		view_ray.p2 = view_ray.p2 + view_point;
	}

	// set view window
	view_window = Frame(view_ray, 1, 1, up);
	// light_sources.back().p = view_point;

	// sort objects
	vector<vector<float>> object_order;
	for (int i = 0; i < objs.size(); i++){
		object_order.push_back({(objs[i].center - view_point).len(), (float)i});
	}
	sort(object_order.begin(), object_order.end());
	reverse(object_order.begin(), object_order.end());

	long long t1 = SDL_GetTicks64();
	process_projection_using_normal(objs, object_order);
	long long t2 = SDL_GetTicks64();
	// cout << "TIME : " << (t2 - t1) << endl;
}

void handle_input()
{
	const Uint8 *p = SDL_GetKeyboardState(NULL);
	turning_speed = (std_turning_speed * ((float)STD_FPS)) / (max(frame_rate,1));
	movement_speed = (std_movement_speed * ((float)STD_FPS)) / (max(frame_rate, 1));

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

}

void clip_test(Object2D o){
	Point offset = Point(100,100);
	vector<Point> temp_pts = {
		Point(0, 0),
		Point(400, 0),
		Point(400, 400),
		Point(0, 400)
	};
	for (auto&i:temp_pts){
		i = i + offset;
	}
	mid_point_line_draw(sm, temp_pts[0], temp_pts[1], RGBcolor(255), 0);
	mid_point_line_draw(sm, temp_pts[2], temp_pts[1], RGBcolor(255), 0);
	mid_point_line_draw(sm, temp_pts[2], temp_pts[3], RGBcolor(255), 0);
	mid_point_line_draw(sm, temp_pts[0], temp_pts[3], RGBcolor(255), 0);

	Frame f = Frame(Ray(Point3D(0, 0, -1), Point3D()), 1, 1, Ray(Point3D(0, 0, 0), Point3D(0, 1, 0)));
	Object2D oo = newPlane(1, Ray(Point3D(0, -1, 0), Point3D(0, 0, 0))).object_to_2d(f, Point3D(0, 0, -2), Ray(Point3D(0, 0, -2), Point3D(0, 0, 0)));
	oo.points = {
		Point(100, 100),
		Point(200, 300),
		Point(-100, 600),
		Point(500, 300)
	};

	for (auto &i : oo.polygons)
	{
		for (auto &j : i)
		{
			Point s = oo.points[j] + offset, e = oo.points[(j + 1) % i.size()] + offset;
			mid_point_line_draw(sm, s, e, RGBcolor(255, 0, 0), 0);
		}
	}
	o = oo.clip_object_2(Point(400, 400));
	for (auto &i:o.polygons){
		for (auto &j:i){
			Point s = o.points[j] + offset, e = o.points[(j + 1) % i.size()] + offset;
			mid_point_line_draw(sm, s,e , RGBcolor(0,0,255), 0 );
		}
	}

}

void take_screenshot(){
	SDL_Surface *sshot = SDL_CreateRGBSurface(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
	string ss_name = "src/Screenshots/3d-view_" + to_string(rand()) + ".bmp";
	if ( SDL_SaveBMP(sshot, ss_name.c_str()) ){
		cout << SDL_GetError() << endl;
		cout << "Screen shot ERROR" << endl;
	}
	else{
		cout << "screenshot taken : " << ss_name << endl;
	}
	SDL_FreeSurface(sshot);
}

int main(int argv, char** args){

	basic_inits();

	// setting view_point, light source etc.
	init_world_rules();

	set_objects(objs);

	// calculating first frame before starting game loop
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
					if (event.key.keysym.sym == SDLK_m)
					{
						movement_mode = (movement_mode + 1) % 2;
					}
					if (event.key.keysym.sym == SDLK_p)
					{
						take_screenshot();
					}
					break;
				default:
					break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0, 0XFF);
		SDL_RenderClear(renderer);

		handle_input();

		//text
		text_overlay();

		sm.renderer(renderer);
		SDL_RenderPresent(renderer);
		
		//frame rate
		new_time = SDL_GetTicks64();
		time_diff = new_time - old_time;
		frame_count++;
		if (time_diff >= 1000){
			frame_rate = ceil(((float)(frame_count*1000))/(float)time_diff);
			frame_count = 0;
			old_time = new_time;
			// cout << frame_rate << endl;
		}
		
	}
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}










// 