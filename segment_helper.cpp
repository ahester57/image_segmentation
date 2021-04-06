// segment_helper.cpp : This file contains the helper functions for the main
// Austin Hester CS642o mar 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>

#include "./include/segment_helper.hpp"


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


cv::Mat
distance_finder(cv::Mat borders)
{
    cv::Mat distance;
    cv::distanceTransform( borders, distance, cv::DIST_L2, 3 );
    cv::normalize( distance, distance, 0.f, 1.f, cv::NORM_MINMAX );
    return distance;
}

// Trivial "black background only" background finder
cv::Mat
make_background_mask(cv::Mat image)
{
    cv::Mat mask;
    cv::inRange( image, cv::Scalar::all(0), cv::Scalar::all(0), mask );
    return mask;
}


cv::Mat
create_bordered_map(cv::Mat canny_edges, cv::Mat mask)
{
    cv::Mat canny_edges_8U; // aka "borders"
    canny_edges.convertTo( canny_edges_8U, CV_8U, 100 );
    cv::bitwise_not( canny_edges_8U, canny_edges_8U );
    // apply mask to map
    canny_edges_8U.setTo( cv::Scalar(0, 0, 0), mask );
    // threshold bordered map output
    cv::threshold( canny_edges_8U, canny_edges_8U, 55, 255, cv::THRESH_BINARY | cv::THRESH_OTSU );
    return canny_edges_8U;
}


std::vector<std::vector<cv::Point>>
find_contours(cv::Mat distance_transform)
{
    cv::threshold( distance_transform, distance_transform, 0.1f, 1.f, cv::THRESH_BINARY );

    cv::Mat distance_8U;
    distance_transform.convertTo( distance_8U, CV_8U );

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours( distance_8U, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );

    distance_8U.release();
    return contours;
}

cv::Mat
draw_markers(std::vector<std::vector<cv::Point>> contours, cv::Size canvas_size)
{
    cv::Mat markers = cv::Mat::zeros( canvas_size, CV_32S );

    for (size_t i = 0; i < contours.size(); i++) {
        int ii = static_cast<int>( i );
        cv::drawContours( markers, contours, ii, cv::Scalar( i + 1 ), -1 );
    }

    return markers;
}
