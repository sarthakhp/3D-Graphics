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

map<string, int> Init_input_map();
map<string, int> input_handler(SDL_Event event, map<string,int> m);

