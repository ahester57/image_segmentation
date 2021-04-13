// segment_helper.cpp : This file contains functions useful for segmentation
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

#include "./include/hsv_convert.hpp"
#include "./include/segment_helper.hpp"


// extract ROI ignoring out of bounds
cv::Mat
extract_roi_safe(cv::Mat image, cv::Rect rect)
{
    // adjust contour_bounds to prevent out of bounds errors
    cv::Rect image_rect = cv::Rect( {}, image.size() );
    cv::Rect intersection = image_rect & rect;
    cv::Rect inter_roi = intersection - image_rect.tl();

    // grab the ROI
    return image( inter_roi );
}

// equalize given image
void
equalize_image(cv::Mat* image)
{
    int n_channels = image->channels();
    assert( n_channels == 1 || n_channels == 3 );

    if (n_channels == 1) {
        cv::equalizeHist( *image, *image );
    } else if (n_channels == 3) {
        cv::Mat hsv_image;
        bgr_to_hsv( *image, &hsv_image );
        cv::Mat hsv_planes[3];
        cv::split( hsv_image, hsv_planes );
        cv::equalizeHist( hsv_planes[2], hsv_planes[2] );
        cv::merge( hsv_planes, 3, hsv_image );
        hsv_to_bgr( hsv_image, image );
    }
}

// find normalized distance transform of binary image
cv::Mat
distance_finder(cv::Mat binary_image)
{
    cv::Mat distance;
    cv::distanceTransform( binary_image, distance, cv::DIST_L2, 3 );
    cv::normalize( distance, distance, 0.f, 1.f, cv::NORM_MINMAX );
    return distance;
}

// Trivial "black background only" background finder
cv::Mat
make_background_mask(cv::Mat image)
{
    //TODO make this better at background detection, not just black backgrounds
    cv::Mat mask;
    cv::inRange( image, cv::Scalar::all(0), cv::Scalar::all(0), mask );
    return ~mask;
}

// find drawable contours from distance transformation
std::vector<std::vector<cv::Point>>
find_distance_contours(cv::Mat distance_transform)
{
    cv::threshold( distance_transform, distance_transform, 0.01f, 1.f, cv::THRESH_BINARY );

    cv::Mat distance_8U;
    distance_transform.convertTo( distance_8U, CV_8U );

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours( distance_8U, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );

    distance_8U.release();
    return contours;
}

// draw singular contour
cv::Mat
draw_contour_as_marker(std::vector<std::vector<cv::Point>> contours, cv::Size canvas_size, int marker_value)
{
    cv::Mat marker = cv::Mat::zeros( canvas_size, CV_8U );
    cv::drawContours( marker, contours, marker_value - 1, cv::Scalar( 255 ), -1 );
    return marker;
}

// draw all contours
cv::Mat
draw_contours_as_markers(std::vector<std::vector<cv::Point>> contours, cv::Size canvas_size)
{
    cv::Mat markers = cv::Mat::zeros( canvas_size, CV_32S );
    for (size_t i = 0; i < contours.size(); i++) {
        int ii = static_cast<int>( i );
        cv::drawContours( markers, contours, ii, cv::Scalar( i + 1 ), -1 );
    }

    return markers;
}

// create auxillary array of bounding rectangles
std::vector<cv::Rect>
draw_bounding_rects(std::vector<std::vector<cv::Point>> contours)
{
    std::vector<cv::Rect> boundaries = std::vector<cv::Rect>( contours.size() );
    for (size_t i = 0; i < contours.size(); i++) {
        int ii = static_cast<int>( i );
        std::vector<cv::Point> contours_poly;
        cv::approxPolyDP( contours[ii], contours_poly, 3, true );
        boundaries[ii] = cv::boundingRect( contours_poly );
        // bump up size of rects just a bit
        boundaries[ii].x -= 5;
        boundaries[ii].y -= 5;
        boundaries[ii] += cv::Size( 10, 10 );
    }

    return boundaries;
}

// double size of rect of and place at center
cv::Rect
center_and_double_rect(cv::Rect rect, cv::Size target_size)
{
    int old_x = rect.x;
    rect.x = target_size.width / 2 - rect.width;
    if (old_x < 0) {
        rect.x += 2 * old_x;
    }
    int old_y = rect.y;
    rect.y = target_size.height / 2 - rect.height;
    if (old_y < 0) {
        rect.y += 2 * old_y;
    }
    rect += cv::Size(
        old_x < 0 ? rect.width + 2 * old_x : rect.width,
        old_y < 0 ? rect.height + 2 * old_y : rect.height
    );
    return rect;
}
