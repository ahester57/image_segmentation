// map_helper.cpp : This file contains the helper functions for the main
// Austin Hester CS642o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

#include "./include/map_helper.hpp"

#include "./include/affine.hpp"
#include "./include/bitwise_porter_duff_ops.hpp"
#include "./include/canny.hpp"
#include "./include/rectangle.hpp"
#include "./include/region_of_interest.hpp"
#include "./include/segmentation.hpp"

#define DEBUG 1

#if DEBUG
    #include <opencv2/highgui/highgui.hpp>
    #include "./include/string_helper.hpp"
#endif


// perform segmentation using canny edges, thresholding, and distance transform
void
segment(MapData* map_data, int hsv_plane)
{
    // canny edge detection, returning contour map
    cv::Mat canny_edges = draw_color_canny_contours( map_data->whole_map, hsv_plane ); // for usa.png, saturation is best to use imo

    // create bordered map
    cv::Mat binary_image = create_binary_image_from_canny_edges( canny_edges, map_data->map_mask );
    canny_edges.release();

    // distance transform on thresholded
    cv::Mat distance_transform = distance_finder( binary_image );
    binary_image.release();

    // find contours of distance transform
    map_data->contours = find_distance_contours( distance_transform );

    // find boundaries of the contours
    map_data->boundaries = draw_bounding_rects( map_data->contours );

    // create markers for foreground objects // aka "markers"
    map_data->markers = draw_contours_as_markers( map_data->contours, distance_transform.size() );
    distance_transform.release();

    // apply watershed algorithm
    cv::watershed( map_data->whole_map, map_data->markers );

#if DEBUG > 1
    cv::Mat markers_8U;
    map_data->markers.convertTo( markers_8U, CV_8U );
    cv::bitwise_and( markers_8U, map_data->map_mask, markers_8U );
    cv::imshow( "Markers 8U", markers_8U );
    markers_8U.release();
#endif

    // create new marked_up_image (the one we click on)
    map_data->marked_up_image = cv::Mat::zeros( map_data->markers.size(), CV_8UC3 );

    // draw original map back on
    draw_in_states( map_data );

}

void
select_region(MapData* map_data, int marker_value)
{

    // zero-out region of interest
    map_data->marked_up_image = cv::Mat::zeros( map_data->whole_map.size(), map_data->whole_map.type() );

    // draw original map back on
    draw_in_states( map_data );

    // highlight selected region
    // draw_in_roi( map_data, marker_value );

    // get bounding rect
    cv::Rect bounding_rect = map_data->boundaries[marker_value - 1];

    // extract the ROI
    cv::Mat region_only = extract_selected_region( map_data, marker_value );

    // double the size
    region_only = resize_affine( region_only, 2.f );

    // copy the region to map_data
    region_only.copyTo( map_data->region_of_interest );
    region_only.release();

    // double size of rect of roi
    bounding_rect = center_and_double_rect( bounding_rect, map_data->marked_up_image.size() );

    // place enlarged roi in marked up image
    map_data->marked_up_image = paint_region_over_map( map_data, bounding_rect );

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

    // paint real map over the region's mask
    drawn_contour = paint_map_atop_region( map_data, marker_value, drawn_contour );

    // grab the ROI
    // get bounding rectangle from auxillary array
    drawn_contour = extract_roi_safe( drawn_contour, map_data->boundaries[marker_value - 1] );

    return drawn_contour;
}

// paint real map atop the region of interest's mask
cv::Mat
paint_map_atop_region(MapData* map_data, int marker_value, cv::Mat drawn_contour)
{
    // create single channel mask
    cv::Mat map_mask_8u;
    map_data->map_mask.convertTo( map_mask_8u, CV_8U );

    // create 3 channel contour
    cv::Mat contour_8u3;
    drawn_contour.convertTo( contour_8u3, CV_8UC3 );
    cv::cvtColor( contour_8u3, contour_8u3, cv::COLOR_GRAY2BGR );

    // paint region using porter duff
    cv::Mat painted_region;
    try {
        painted_region = bitwise_i1_atop_i2(
            map_data->whole_map,
            contour_8u3,
            map_mask_8u,
            drawn_contour
        );
    } catch (std::string &str) {
        std::cerr << "ERROR : paint_map_atop_region" << std::endl;
        std::cerr << "Error: " << str << std::endl;
    } catch (cv::Exception &e) {
        std::cerr << "ERROR : paint_map_atop_region" << std::endl;
        std::cerr << "Error: " << e.msg << std::endl;
    } catch (std::runtime_error &re) {
        std::cerr << "ERROR : paint_map_atop_region" << std::endl;
        std::cerr << "Error: " << re.what() << std::endl;
    }

    if (painted_region.empty()) {
        map_data->whole_map.copyTo( painted_region );
    }

    map_mask_8u.release();
    contour_8u3.release();
    return painted_region;
}

// paint the region of interest over the whole_map
cv::Mat
paint_region_over_map(MapData* map_data, cv::Rect bounding_rect)
{
    // create single channel mask
    cv::Mat map_mask_8u;
    map_data->map_mask.convertTo( map_mask_8u, CV_8U );

    // create 3 channel roi mask
    cv::Mat roi_8u;
    map_data->region_of_interest.convertTo( roi_8u, CV_8U );
    cv::cvtColor( roi_8u, roi_8u, cv::COLOR_BGR2GRAY );

    // pad roi
    cv::Mat padded_roi = make_border_from_size_and_rect(map_data->region_of_interest, map_mask_8u.size(), bounding_rect);

    // pad roi mask
    cv::Mat padded_roi_mask = make_border_from_size_and_rect(roi_8u, map_mask_8u.size(), bounding_rect);
    // cv::Mat::zeros( map_mask_8u.size(), map_mask_8u.type() );
    // cv::Mat center_roi_mask = padded_roi_mask( cv::Rect(
    //     bounding_rect.x,
    //     bounding_rect.y,
    //     roi_8u.size().width,
    //     roi_8u.size().height
    // ));
    // roi_8u.copyTo( center_roi_mask );
    // center_roi_mask.copyTo (padded_roi_mask( bounding_rect ) );
    // cv:imshow( "padd_mask", padded_roi_mask );

#if DEBUG
    std::cout << cv_type_to_str( map_mask_8u ) << " :: " << cv_type_to_str( padded_roi_mask ) << std::endl;
    std::cout << map_mask_8u.size() << " :: " << map_mask_8u.size() << std::endl;
#endif

    // paint the region of interest over the map
    cv::Mat painted_map;
    try {
        painted_map = bitwise_i1_in_i2(
            padded_roi,
            map_data->marked_up_image,
            padded_roi_mask,
            padded_roi_mask
        );
    } catch (std::string &str) {
        std::cerr << "ERROR : paint_map_atop_region" << std::endl;
        std::cerr << "Error: " << str << std::endl;
    } catch (cv::Exception &e) {
        std::cerr << "ERROR : paint_map_atop_region" << std::endl;
        std::cerr << "Error: " << e.msg << std::endl;
    } catch (std::runtime_error &re) {
        std::cerr << "ERROR : paint_region_over_map" << std::endl;
        std::cerr << "Error: " << re.what() << std::endl;
    }

    if (painted_map.empty()) {
        map_data->marked_up_image.copyTo( painted_map );
    }

    map_mask_8u.release();
    roi_8u.release();
    padded_roi.release();
    padded_roi_mask.release();

    return painted_map;
}

// surround roi with black border before placing over map
cv::Mat
make_border_from_size_and_rect(cv::Mat image, cv::Size target_size, cv::Rect rect)
{
    // math to confuse you
    int top = rect.y;
    int bottom = target_size.height - rect.y - rect.height;
    if (top < 0 && bottom >= 0) {
        bottom -= top;
    } else if (top >= 0 && bottom < 0) {
        top += bottom;
    }
    if (bottom < 0 && top >= 0) {
        top -= bottom;
    } else if (bottom >= 0 && top < 0) {
        bottom += top;
    }

    // seems to be necessary to avoid weird stuff from happening on the sides
    int left = rect.x;
    int right = target_size.width - rect.x - rect.width;
    if (left < 0 && right >= 0) {
        right -= left;
    } else if (left >= 0 && right < 0) {
        left += right;
    }
    if (right < 0 && left >= 0) {
        left -= right;
    } else if (right >= 0 && left < 0) {
        right += left;
    }

#if DEBUG
    std::cout << top << " " << bottom << " " << left << " " << right << std::endl;
#endif

    // avoid zeros
    top = top < 0 ? 0 : top;
    bottom = bottom < 0 ? 0 : bottom;
    left = left < 0 ? 0 : left;
    right = right < 0 ? 0 : right;

    cv::Mat padded_image;
    cv::copyMakeBorder(
        image,
        padded_image,
        top,
        bottom,
        left,
        right,
        cv::BORDER_CONSTANT,
        cv::Scalar(0)
    );

    return padded_image;
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
