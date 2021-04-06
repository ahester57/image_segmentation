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

std::string output_image_filename;


cv::Mat
segment(cv::Mat image)
{
    cv::Size input_image_size = image.size();

    // create mask, only distance filter on foreground
    //TODO make this better at background detection, not just black backgrounds
    cv::Mat mask = make_background_mask( image );

    // canny edge detection, returning contour map
    cv::Mat canny_edges = draw_color_canny_contours( image ); // for usa.png, saturation is best to use

    // create bordered map
    cv::Mat borders = create_bordered_map( canny_edges, mask );

    // distance transform on thresholded
    cv::Mat distance = distance_finder( borders );

    // create markers for foreground objects // aka "markers"
    cv::Mat markers = draw_markers( distance );

    cv::Mat markers_8U;
    markers.convertTo( markers_8U, CV_8U );

    canny_edges.release();
    mask.release();
    borders.release();
    distance.release();
    markers.release();
    return markers_8U;
}


int
main(int argc, const char** argv)
{
    // CLA variables
    std::string input_image_filename;
    // CLA flags
    float scale_image_value;
    bool blur_output;
    bool equalize_output;

    // parse and save command line args
    int parse_result = parse_arguments(
        argc, argv,
        &input_image_filename,
        &output_image_filename,
        &scale_image_value,
        &blur_output,
        &equalize_output
    );
    if (parse_result != 1) return parse_result;

    cv::Mat input_image = open_image( input_image_filename );

    // crop if odd resolution
    input_image = input_image(
        cv::Rect( 0, 0, input_image.cols & -2, input_image.rows & -2 )
    );

    // scale the input size if given 's' flag
    if (scale_image_value != 1.f) {
        input_image = resize_affine( input_image, scale_image_value );
    }

    cv::imshow( WINDOW_NAME, input_image );

    // initialize the mouse callback
    MapData map_data = { &WINDOW_NAME, &input_image, &input_image };
    init_callback( &map_data );

    // 'event loop' for keypresses
    while (wait_key());

    // segment the image
    cv::Mat output_image = segment( input_image );

    // blur the output if given 'b' flag
    if (blur_output) {
        cv::medianBlur( output_image, output_image, 3 );
    }

    // equalize the output if given 'e' flag
    if (equalize_output) {
        cv::equalizeHist( output_image, output_image );
    }

    cv::imshow( WINDOW_NAME + " Output Image", output_image );
    write_img_to_file( output_image, "./out", output_image_filename );

    // 'event loop' for keypresses
    while (wait_key());

    cv::destroyAllWindows();
    input_image.release();
    output_image.release();

    return 0;
}
