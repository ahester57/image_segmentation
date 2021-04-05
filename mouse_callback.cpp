// mouse_callback.cpp : Mouse Callback Stuff
// Austin Hester CS642o april 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "./include/mouse_callback.hpp"


// draw zeros on image
void
mouse_callback_draw_zeros(int event, int x, int y, int d, void* userdata)
{
    cv::Mat* frequency_mask = (cv::Mat*) userdata;

    switch (event) {
        case cv::EVENT_LBUTTONUP:
            // push the new point
            // draw a circle mask at chosen points
            cv::circle( *frequency_mask, cv::Point2f( x, y ), 4, cv::Scalar(0), cv::FILLED );
            break;
    }
}


// assign mouse callbacks
void
init_callback(std::string window_name, cv::Mat* frequency_mask)
{
    cv::setMouseCallback( window_name, mouse_callback_draw_zeros, frequency_mask );
}
