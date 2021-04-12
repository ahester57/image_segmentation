// mouse_callback.cpp : Mouse Callback Stuff
// Austin Hester CS642o april 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "./include/dir_func.hpp"
#include "./include/mouse_callback.hpp"
#include "./include/segment_helper.hpp"


// draw zeros on image
void
mouse_callback_draw_zeros(int event, int x, int y, int d, void* userdata)
{
    MapData* map_data = (MapData*) userdata;

    switch (event) {
        case cv::EVENT_LBUTTONUP:
            // find the marker at that point
            int marker_value = map_data->markers.at<int>( y, x );
            higlight_selected_region( map_data, marker_value );
            // show marked_up_image
            cv::imshow( map_data->window_name, map_data->marked_up_image );
            // save marked_up_image
            write_img_to_file(
                map_data->marked_up_image,
                "./out",
                "regions/out_" + std::to_string(marker_value) + ".png"
            );
            break;
    }
}


// assign mouse callbacks
void
init_callback(MapData* map_data)
{
    cv::setMouseCallback( map_data->window_name, mouse_callback_draw_zeros, map_data );
}
