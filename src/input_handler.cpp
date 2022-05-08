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

#include <input_handler.h>

map<string, int> Init_input_map(){
    map<string, int> m = {
        {"quit",0},
    };
    return m;
}

map<string, int> input_handler(SDL_Event event, map<string,int> m){
    switch (event.type){
    case SDL_QUIT:
        m.at("quit") = 1;
        break;

    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
        {
            m.at("quit") = 1;
            cout << "quit" << endl;
        }
        break;
    }
    return m;
}
