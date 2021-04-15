// map_helper.hpp : Helper Functions
// Austin Hester CS542o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef MAP_HELPER_H
#define MAP_HELPER_H

#include <opencv2/core/core.hpp>

#include <vector>


typedef struct {
    std::string window_name;
    cv::Mat whole_map;
    cv::Mat map_mask;
    cv::Mat region_of_interest;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Rect> boundaries;
    cv::Mat markers;
    cv::Mat marked_up_image;
} MapData;


void segment(MapData* map_data, int hsv_plane = 2);

void higlight_selected_region(MapData* map_data, int marker_value);

cv::Mat extract_selected_region(MapData* map_data, int marker_value);

cv::Mat paint_map_atop_region(MapData* map_data, int marker_value, cv::Mat drawn_contour);

cv::Mat paint_region_over_map(MapData* map_data, cv::Rect bounding_rect);

cv::Mat make_border_from_size_and_rect(cv::Mat image, cv::Size target_size, cv::Rect rect);

void draw_in_states(MapData* map_data);

void draw_in_roi(MapData* map_data, int marker_value);

#endif
