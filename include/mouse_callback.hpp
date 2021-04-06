
// mouse_callback.hpp : Mouse Callback Stuff
// Austin Hester CS642o april 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef MOUSE_CALLBACK_H
#define MOUSE_CALLBACK_H

#include <opencv2/core/core.hpp>


typedef struct {
    const std::string* window_name;
    cv::Mat* whole_map;
    cv::Mat* region_of_interest;
} MapData;

void mouse_callback_draw_zeros(int event, int x, int y, int d, void* userdata);
void init_callback(MapData* frequency_mask);

#endif
