// map_segment.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Austin Hester CS642o mar 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>

#include "./include/affine.hpp"
#include "./include/canny.hpp"
#include "./include/cla_parse.hpp"
#include "./include/dir_func.hpp"
#include "./include/mouse_callback.hpp"
#include "./include/segment_helper.hpp"

#define DEBUG 1


const std::string WINDOW_NAME = "Map Segmentation";



void
segment(MapData* map_data, bool grayscale, int hsv_plane = 2)
{
    cv::Size input_image_size = map_data->whole_map->size();

    // create mask, only distance filter on foreground
    //TODO make this better at background detection, not just black backgrounds
    map_data->map_mask = make_background_mask( *map_data->whole_map );

    // canny edge detection, returning contour map
    cv::Mat canny_edges = grayscale ? draw_canny_contours( *map_data->whole_map )
                                    : draw_color_canny_contours( *map_data->whole_map, hsv_plane ); // for usa.png, saturation is best to use imo

    // // create bordered map
    cv::Mat borders = create_bordered_map( canny_edges, *map_data->map_mask );
    canny_edges.release();

    // distance transform on thresholded
    cv::Mat distance = distance_finder( borders );
    borders.release();

    map_data->contours = find_distance_contours( distance );

    // create markers for foreground objects // aka "markers"
    map_data->markers = draw_contours_as_markers( *map_data->contours, distance.size() );
    distance.release();

    // apply watershed algorithm
    cv::watershed( *map_data->whole_map, *map_data->markers );

    cv::Mat markers_8U;
    map_data->markers->convertTo( markers_8U, CV_8U );
    cv::bitwise_and( markers_8U, ~*map_data->map_mask, markers_8U );
    cv::imshow( "Markers 8U", markers_8U );
    markers_8U.release();

    map_data->marked_up_image = new cv::Mat();
    *map_data->marked_up_image = cv::Mat::zeros( map_data->markers->size(), CV_8UC3 );

    // fill in states
    for (int i = 0; i < map_data->markers->rows; i++)
    {
        for (int j = 0; j < map_data->markers->cols; j++)
        {
            int pixel = map_data->markers->at<int>( i, j );
            if (pixel > 0 && pixel <= static_cast<int>(map_data->contours->size())) {
                map_data->marked_up_image->at<cv::Vec3b>( i, j ) = map_data->whole_map->at<cv::Vec3b>( i, j );
            }
        }
    }

}


int
main(int argc, const char** argv)
{
    // CLA variables
    std::string input_image_filename;
    std::string output_image_filename;
    // CLA flags
    float scale_image_value;
    bool blur_output;
    bool equalize_output;
    int hsv_plane;
    bool grayscale;

    // parse and save command line args
    int parse_result = parse_arguments(
        argc, argv,
        &input_image_filename,
        &output_image_filename,
        &scale_image_value,
        &blur_output,
        &equalize_output,
        &hsv_plane,
        &grayscale
    );
    if (parse_result != 1) return parse_result;

    cv::Mat input_image = open_image( input_image_filename, grayscale );

    // crop if odd resolution
    input_image = input_image(
        cv::Rect( 0, 0, input_image.cols & -2, input_image.rows & -2 )
    );

    // scale the input size if given 's' flag
    if (scale_image_value != 1.f) {
        input_image = resize_affine( input_image, scale_image_value );
    }

    cv::imshow( WINDOW_NAME, input_image );




    // 'event loop' for keypresses
    while (wait_key());

    MapData map_data = { NULL, &input_image, NULL, &input_image, NULL, NULL };

    // segment the image
    segment( &map_data, grayscale, hsv_plane );

    // blur the output if given 'b' flag
    if (blur_output) {
        cv::medianBlur( *map_data.marked_up_image, *map_data.marked_up_image, 3 );
    }

    // equalize the output if given 'e' flag
    if (equalize_output) {
        equalize_image( map_data.marked_up_image, grayscale );
    }

    std::string output_window_name = WINDOW_NAME + " Output Image";
    cv::imshow( output_window_name, *map_data.marked_up_image );
    write_img_to_file( *map_data.marked_up_image, "./out", output_image_filename );

    // initialize the mouse callback
    map_data.window_name = &output_window_name;
    init_callback( &map_data );

    // 'event loop' for keypresses
    while (wait_key());


    cv::destroyAllWindows();

    delete map_data.contours;
    delete map_data.markers;
    delete map_data.map_mask;
    delete map_data.region_of_interest;
    delete map_data.marked_up_image;

    input_image.release();

    return 0;
}
