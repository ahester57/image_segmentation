// segment_helper.hpp : Helper Functions
// Austin Hester CS542o mar 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef SEGMENT_HELPER_H
#define SEGMENT_HELPER_H

#include <opencv2/core/core.hpp>

#include <vector>


typedef struct {
    const std::string window_name;
    cv::Mat* whole_map;
    cv::Mat* map_mask;
    cv::Mat region_of_interest;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Rect> boundaries;
    cv::Mat markers;
    cv::Mat marked_up_image;
} MapData;


int wait_key();

void higlight_selected_region(MapData* map_data, int marker_value);

cv::Mat extract_selected_region(MapData* map_data, int marker_value);

cv::Mat extract_roi_safe(cv::Mat image, cv::Rect rect);

void draw_in_states(MapData* map_data);

void draw_in_roi(MapData* map_data, int marker_value);

void equalize_image(cv::Mat* image);

cv::Mat distance_finder(cv::Mat borders);

cv::Mat* make_background_mask(cv::Mat image);

cv::Mat create_bordered_map(cv::Mat canny_edges, cv::Mat mask);

std::vector<std::vector<cv::Point>> find_distance_contours(cv::Mat distance_transform);

cv::Mat draw_contour_as_marker(std::vector<std::vector<cv::Point>> contours, cv::Size canvas_size, int marker_value);

cv::Mat draw_contours_as_markers(std::vector<std::vector<cv::Point>> contours, cv::Size canvas_size);

std::vector<cv::Rect> draw_bounding_rects(std::vector<std::vector<cv::Point>> contours);

#endif
