// segment_helper.hpp : Helper Functions
// Austin Hester CS542o mar 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef SEGMENT_HELPER_H
#define SEGMENT_HELPER_H

#include <opencv2/core/core.hpp>

#include <vector>
#include <iostream>

int wait_key();
cv::Mat distance_finder(cv::Mat borders);
cv::Mat make_background_mask(cv::Mat image);
cv::Mat create_bordered_map(cv::Mat canny_edges, cv::Mat mask);

#endif
