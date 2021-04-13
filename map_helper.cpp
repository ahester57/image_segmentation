// map_helper.cpp : This file contains the helper functions for the main
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "./include/affine.hpp"
#include "./include/bitwise_porter_duff_ops.hpp"
#include "./include/map_helper.hpp"
#include "./include/segment_helper.hpp"


void
higlight_selected_region(MapData* map_data, int marker_value)
{

    // zero-out region of interest
    map_data->marked_up_image = cv::Mat::zeros( map_data->whole_map.size(), map_data->whole_map.type() );

    // draw original map back on
    draw_in_states( map_data );

    // highlight selected region
    draw_in_roi( map_data, marker_value );

    // draw the bounding rect of the selected region
    cv::rectangle(
        map_data->marked_up_image,
        map_data->boundaries[marker_value - 1],
        cv::Scalar::all(255),
        2
    );

    // draw the region seperately
    cv::Mat region_only = extract_selected_region( map_data, marker_value );
    region_only.copyTo( map_data->region_of_interest );
    region_only.release();

}

// extract selected region from contours
cv::Mat
extract_selected_region(MapData* map_data, int marker_value)
{
    // draw contours at given marker_value
    cv::Mat drawn_contour = draw_contour_as_marker(
        map_data->contours,
        map_data->marked_up_image.size(),
        marker_value
    );

    drawn_contour = paint_region_over( map_data, marker_value, drawn_contour );

    // get bounding rectangle from auxillary array
    cv::Rect contour_bounds = map_data->boundaries[marker_value - 1];

    // grab the ROI
    drawn_contour = extract_roi_safe( drawn_contour, contour_bounds );

    // double the size
    drawn_contour = resize_affine( drawn_contour, 2.f );

    return drawn_contour;
}

// paint the selected region over with whole_map
cv::Mat
paint_region_over(MapData* map_data, int marker_value, cv::Mat drawn_contour)
{
    // create single channel mask
    cv::Mat map_mask_8u;
    map_data->map_mask.convertTo( map_mask_8u, CV_8U );

    // create 3 channel contour
    cv::Mat contour_8u3;
    drawn_contour.convertTo( contour_8u3, CV_8UC3 );
    cv::cvtColor( contour_8u3, contour_8u3, cv::COLOR_GRAY2BGR );

    // paint region using porter duff
    cv::Mat painted_region = bitwise_i1_atop_i2( map_data->whole_map, contour_8u3, map_mask_8u, drawn_contour);

    map_mask_8u.release();
    contour_8u3.release();
    return painted_region;
}

// draw original states back onto marked_up_image
void
draw_in_states(MapData* map_data)
{
    // create single channel mask
    cv::Mat mask_8u;
    map_data->map_mask.convertTo( mask_8u, CV_8U );

    // fill in states
    for (int i = 0; i < map_data->markers.rows; i++)
    {
        for (int j = 0; j < map_data->markers.cols; j++)
        {
            int pixel = map_data->markers.at<int>( i, j );
            if (pixel > 0 && pixel <= static_cast<int>(map_data->contours.size())) {
                map_data->marked_up_image.at<cv::Vec3b>( i, j ) = map_data->whole_map.at<cv::Vec3b>( i, j );
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
    map_data->map_mask.convertTo( mask_8u, CV_8U );

    // fill in selected region with 255, 255, 150
    for (int i = 0; i < map_data->markers.rows; i++)
    {
        for (int j = 0; j < map_data->markers.cols; j++)
        {
            // skip if not in mask
            if (mask_8u.at<uchar>( i, j ) == (uchar) 0) {
                continue;
            }
            int pixel = map_data->markers.at<int>( i, j );
            if (pixel > 0 && pixel == marker_value) {
                // paint selected region
                map_data->marked_up_image.at<cv::Vec3b>( i, j ) = cv::Vec3b(255, 255, 150);
            }
        }
    }
    mask_8u.release();
}
