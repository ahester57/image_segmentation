// map_segment.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

#include "./include/affine.hpp"
#include "./include/canny.hpp"
#include "./include/cla_parse.hpp"
#include "./include/dir_func.hpp"
#include "./include/equalize.hpp"
#include "./include/map_helper.hpp"
#include "./include/mouse_callback.hpp"
#include "./include/rectangle.hpp"
#include "./include/segmentation.hpp"

#define DEBUG 0


const std::string WINDOW_NAME = "Map Segmentation";


// event loop
// call in a while loop to only register q or <esc>
int
wait_key()
{
    char key_pressed = cv::waitKey(0) & 255;
    // 'q' or  <escape> quits out
    if (key_pressed == 27 || key_pressed == 'q') {
        return 0;
    }
    return 1;
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
    bool pad_input = false;

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
        std::cout << "Scaled Image size is:\t\t" << input_image.cols << "x" << input_image.rows << std::endl;
    }

    if (pad_input) {
        cv::copyMakeBorder( input_image, input_image, 50, 50, 50, 50, cv::BORDER_CONSTANT, cv::Scalar(0) );
        std::cout << "Padded Image size is:\t\t" << input_image.cols << "x" << input_image.rows << std::endl;
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

    // create mask, only distance filter on foreground
    //TODO make this better at background detection, not just black backgrounds
    map_data.map_mask = make_background_mask( map_data.whole_map );

    // segment the image by intensity
    segment( &map_data, 2 );

    // replace the input_image with output from above segmentation
    map_data.whole_map = cv::Mat::zeros( map_data.whole_map.size(), map_data.whole_map.type() );
    map_data.marked_up_image.copyTo( map_data.whole_map );

    // segment again on saturation
    segment( &map_data, 1 );

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
