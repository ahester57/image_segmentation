// segment_helper.cpp : This file contains the helper functions for the main
// Austin Hester CS642o mar 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>

#include "./include/affine.hpp"
#include "./include/hsv_convert.hpp"
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


void
higlight_selected_region(MapData* map_data, int marker_value)
{

    // zero-out region of interest
    *map_data->marked_up_image = cv::Mat::zeros( map_data->whole_map->size(), map_data->whole_map->type() );

    // draw original map back on
    draw_in_states( map_data );
    // highlight selected region
    draw_in_roi( map_data, marker_value );
    // draw the bounding rect of the selected region
    cv::rectangle( *map_data->marked_up_image, (*map_data->boundaries)[marker_value - 1], cv::Scalar::all(255), 2 );

    // draw the region seperately
    cv::Mat region_only = extract_selected_region( map_data, marker_value );
    region_only.copyTo( map_data->region_of_interest );
    cv::imshow( "region_only", map_data->region_of_interest );
    // wait_key();

    region_only.release();
}


// extract selected region from contours
cv::Mat
extract_selected_region(MapData* map_data, int marker_value)
{
    // draw contours and get bounding rectangle
    cv::Mat drawn_contour = draw_contour_as_marker( *map_data->contours, map_data->marked_up_image->size(), marker_value );
    cv::Rect contour_bounds = (*map_data->boundaries)[marker_value - 1];

    // grab the ROI
    drawn_contour = extract_roi_safe( drawn_contour, contour_bounds );

    // double the size
    drawn_contour = resize_affine( drawn_contour, 2.f );

    return drawn_contour;
}


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


// draw original states back onto marked_up_image
void
draw_in_states(MapData* map_data)
{
    // create single channel mask
    cv::Mat mask_8u;
    map_data->map_mask->convertTo( mask_8u, CV_8U );

    // fill in states
    for (int i = 0; i < map_data->markers.rows; i++)
    {
        for (int j = 0; j < map_data->markers.cols; j++)
        {
            int pixel = map_data->markers.at<int>( i, j );
            if (pixel > 0 && pixel <= static_cast<int>(map_data->contours->size())) {
                map_data->marked_up_image->at<cv::Vec3b>( i, j ) = map_data->whole_map->at<cv::Vec3b>( i, j );
            }
        }
    }
    mask_8u.release();
}


void
draw_in_roi(MapData* map_data, int marker_value)
{
    // create single channel mask
    cv::Mat mask_8u;
    map_data->map_mask->convertTo( mask_8u, CV_8U );

    // fill in selected region with 255, 255, 150
    for (int i = 0; i < map_data->markers.rows; i++)
    {
        for (int j = 0; j < map_data->markers.cols; j++)
        {
            // skip if not in mask
            if (mask_8u.at<uchar>( i, j ) != (uchar) 0) {
                continue;
            }
            int pixel = map_data->markers.at<int>( i, j );
            if (pixel > 0 && pixel == marker_value) {
                // paint selected region
                map_data->marked_up_image->at<cv::Vec3b>( i, j ) = cv::Vec3b(255, 255, 150);
            }
        }
    }
    mask_8u.release();
}


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

// find normalized distance transform of image
cv::Mat
distance_finder(cv::Mat borders)
{
    cv::Mat distance;
    cv::distanceTransform( borders, distance, cv::DIST_L2, 3 );
    cv::normalize( distance, distance, 0.f, 1.f, cv::NORM_MINMAX );
    return distance;
}

// Trivial "black background only" background finder
cv::Mat*
make_background_mask(cv::Mat image)
{
    cv::Mat* mask = new cv::Mat();
    cv::inRange( image, cv::Scalar::all(0), cv::Scalar::all(0), *mask );
    return mask;
}

// created binary image of regions
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

// find drawable contours from distance transformation
std::vector<std::vector<cv::Point>>*
find_distance_contours(cv::Mat distance_transform)
{
    cv::threshold( distance_transform, distance_transform, 0.01f, 1.f, cv::THRESH_BINARY );

    cv::Mat distance_8U;
    distance_transform.convertTo( distance_8U, CV_8U );

    std::vector<std::vector<cv::Point>>* contours = new std::vector<std::vector<cv::Point>>();
    cv::findContours( distance_8U, *contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE );

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
std::vector<cv::Rect>*
draw_bounding_rects(std::vector<std::vector<cv::Point>> contours)
{
    std::vector<cv::Rect>* boundaries = new std::vector<cv::Rect>;
    *boundaries = std::vector<cv::Rect>( contours.size() );
    for (size_t i = 0; i < contours.size(); i++) {
        int ii = static_cast<int>( i );
        std::vector<cv::Point> contours_poly;
        cv::approxPolyDP( contours[ii], contours_poly, 3, true );
        (*boundaries)[ii] = cv::boundingRect( contours_poly );
        // bump up size of rects just a bit
        (*boundaries)[ii].x -= 5;
        (*boundaries)[ii].y -= 5;
        (*boundaries)[ii] += cv::Size( 10, 10 );
    }

    return boundaries;
}
