// segment_helper.hpp : This file contains functions useful for segmentation
// Austin Hester CS542o apr 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef SEGMENT_HELPER_H
#define SEGMENT_HELPER_H

#include <opencv2/core/core.hpp>

#include <vector>


int wait_key();

cv::Mat extract_roi_safe(cv::Mat image, cv::Rect rect);

void equalize_image(cv::Mat* image);

cv::Mat distance_finder(cv::Mat borders);

cv::Mat make_background_mask(cv::Mat image);

std::vector<std::vector<cv::Point>> find_distance_contours(cv::Mat distance_transform);

cv::Mat draw_contour_as_marker(std::vector<std::vector<cv::Point>> contours, cv::Size canvas_size, int marker_value);

cv::Mat draw_contours_as_markers(std::vector<std::vector<cv::Point>> contours, cv::Size canvas_size);

std::vector<cv::Rect> draw_bounding_rects(std::vector<std::vector<cv::Point>> contours);

#endif
