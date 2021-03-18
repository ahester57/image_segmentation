// segment.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Austin Hester CS642o mar 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>

#include "./include/cla_parse.hpp"
#include "./include/dir_func.hpp"
#include "./include/segment_helper.hpp"

#define DEBUG 1


const std::string WINDOW_NAME = "Segmentation";

std::string output_image_filename;


cv::Mat
segment(cv::Mat image, bool manual_mode)
{
    // initialize images
    cv::Size input_image_size = image.size();


    return image;
}


int
main(int argc, const char** argv)
{
    // CLA variables
    std::string input_image_filename;
    // CLA flags
    bool double_input_size;
    bool blur_output;
    bool equalize_output;
    bool manual_mode;

    // parse and save command line args
    int parse_result = parse_arguments(
        argc, argv,
        &input_image_filename,
        &output_image_filename,
        &double_input_size,
        &blur_output,
        &equalize_output,
        &manual_mode
    );
    if (parse_result != 1) return parse_result;

    cv::Mat input_image = open_image( input_image_filename, true );

    // double the input size if given 'd' flag
    if (double_input_size) {
        cv::resize( input_image, input_image, input_image.size() * 2);
    }

    cv::imshow( WINDOW_NAME + " Input Image", input_image );

    cv::Mat output_image = segment( input_image, manual_mode );

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
    output_image.release();

    return 0;
}
