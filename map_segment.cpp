// map_segment.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Austin Hester CS642o apr 2021
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
    // canny edge detection, returning contour map
    cv::Mat canny_edges = grayscale ? draw_canny_contours( map_data->whole_map )
                                    : draw_color_canny_contours( map_data->whole_map, hsv_plane ); // for usa.png, saturation is best to use imo

    // create bordered map
    cv::Mat borders = create_bordered_map( canny_edges, map_data->map_mask );
    canny_edges.release();

    // distance transform on thresholded
    cv::Mat distance = distance_finder( borders );
    borders.release();

    // find contours of distance transform
    map_data->contours = find_distance_contours( distance );

    // find boundaries of the contours
    map_data->boundaries = draw_bounding_rects( map_data->contours );

    // create markers for foreground objects // aka "markers"
    map_data->markers = draw_contours_as_markers( map_data->contours, distance.size() );
    distance.release();

    // apply watershed algorithm
    cv::watershed( map_data->whole_map, map_data->markers );

#if DEBUG
    cv::Mat markers_8U;
    map_data->markers.convertTo( markers_8U, CV_8U );
    cv::bitwise_and( markers_8U, ~map_data->map_mask, markers_8U );
    cv::imshow( "Markers 8U", markers_8U );
    markers_8U.release();
#endif

    // create new marked_up_image (the one we click on)
    map_data->marked_up_image = cv::Mat::zeros( map_data->markers.size(), CV_8UC3 );

    // draw original map back on
    draw_in_states( map_data );

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

#if DEBUG
    cv::imshow( WINDOW_NAME, input_image );
    // 'event loop' for keypresses
    while (wait_key());
#endif

    std::string output_window_name = WINDOW_NAME + " Output Image";

    MapData map_data;
    map_data.window_name = output_window_name;
    input_image.copyTo( map_data.whole_map );
    map_data.map_mask = cv::Mat();
    map_data.region_of_interest = cv::Mat();
    map_data.contours = std::vector<std::vector<cv::Point>>();
    map_data.boundaries = std::vector<cv::Rect>();
    map_data.markers = cv::Mat();
    map_data.marked_up_image = cv::Mat();

    // create mask, only distance filter on foreground
    //TODO make this better at background detection, not just black backgrounds
    map_data.map_mask = make_background_mask( map_data.whole_map );

    // segment the image by intensity
    segment( &map_data, grayscale, 2 );

    // replace the input_image with output from above segmentation
    map_data.whole_map = cv::Mat::zeros( map_data.whole_map.size(), map_data.whole_map.type() );
    map_data.marked_up_image.copyTo( map_data.whole_map );

    // segment again on saturation
    segment( &map_data, grayscale, 1 );

    // blur the output if given 'b' flag
    if (blur_output) {
        cv::medianBlur( map_data.marked_up_image, map_data.marked_up_image, 3 );
    }

    // equalize the output if given 'e' flag
    if (equalize_output) {
        equalize_image( &map_data.marked_up_image );
    }

    cv::imshow( map_data.window_name, map_data.marked_up_image );
    write_img_to_file( map_data.marked_up_image, "./out", output_image_filename );

    // initialize the mouse callback
    init_callback( &map_data );

    // 'event loop' for keypresses
    while (wait_key());


    cv::destroyAllWindows();

    map_data.whole_map.release();
    map_data.markers.release();
    map_data.map_mask.release();
    map_data.region_of_interest.release();
    map_data.marked_up_image.release();

    input_image.release();

    return 0;
}
