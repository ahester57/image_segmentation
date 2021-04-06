// mouse_callback.cpp : Mouse Callback Stuff
// Austin Hester CS642o april 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "./include/mouse_callback.hpp"


// draw zeros on image
void
mouse_callback_draw_zeros(int event, int x, int y, int d, void* userdata)
{
    MapData* map_data = (MapData*) userdata;

    switch (event) {
        case cv::EVENT_LBUTTONUP:
            // push the new point
            // draw a circle mask at chosen points
            cv::circle( *map_data->whole_map, cv::Point2f( x, y ), 4, cv::Scalar(0), cv::FILLED );
            cv::imshow( *map_data->window_name, *map_data->whole_map );
            break;
    }
}


// assign mouse callbacks
void
init_callback(MapData* map_data)
{
    cv::setMouseCallback( *map_data->window_name, mouse_callback_draw_zeros, map_data );
}
