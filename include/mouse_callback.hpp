
// mouse_callback.hpp : Mouse Callback Stuff
// Austin Hester CS642o april 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#ifndef MOUSE_CALLBACK_H
#define MOUSE_CALLBACK_H

#include <opencv2/core/core.hpp>

#include <vector>
#include <iostream>

std::vector<std::string> split (std::string s, char delim);
std::string cv_type_to_str(cv::Mat img);

#endif
