// mouse_callback.cpp : Mouse Callback Stuff
// Austin Hester CS642o april 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "./include/mouse_callback.hpp"

#include "./include/dir_func.hpp"

#define DEBUG 1

#if DEBUG
    #include <iostream>
#endif


// draw zeros on image
void
mouse_callback_draw_zeros(int event, int x, int y, int d, void* userdata)
{
    MapData* map_data = (MapData*) userdata;

    switch (event) {

    // RIGHT MOUSE BUTTON
        case cv::EVENT_RBUTTONUP:
            // zero-out region of interest
            map_data->marked_up_image = cv::Mat::zeros( map_data->whole_map.size(), map_data->whole_map.type() );

            // draw original map back on
            draw_in_states( map_data );

            // show marked_up_image
            cv::imshow( map_data->window_name, map_data->whole_map );
            break;

    // LEFT MOUSE BUTTON
        case cv::EVENT_LBUTTONUP:
            // check bounds (needed if double ROI is larger than input image
            if (x > map_data->markers.size().width || y > map_data->markers.size().height) {
#if DEBUG
                std::cout << "OOB" << std::endl;
#endif
                break;
            }

            // find the marker at that point
            int marker_value = map_data->markers.at<int>( y, x );

#if DEBUG
            std::cout << "Marker Value:\t\t" << marker_value << std::endl;
#endif
            // check marker exists
            if (marker_value <= 0 || marker_value > map_data->contours.size()) {
#if DEBUG
                std::cout << "Marker Out of Range." << std::endl;
#endif
                break;
            }

            // extract region_of_interest
            higlight_selected_region( map_data, marker_value );

            // show region_of_interest in new window
            // cv::imshow( "region_of_interest", map_data->region_of_interest );

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
