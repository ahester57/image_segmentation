// canny.cpp : Canny Edge Detections
// Austin Hester CS642o april 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "./include/canny.hpp"


// draw contours of canny edge detection
cv::Mat
draw_canny_contours(cv::Mat image)
{
    // blur image
    cv::Mat canny_output;
    cv::GaussianBlur( image, canny_output, cv::Size( 3, 3 ), 0.5f );

    // compute canny edges
    cv::imshow(" Canny Input Image", canny_output );
    cv::Canny( canny_output, canny_output, 75, 200 );

    // find the contours to draw
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours( canny_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

    // draw the contours
    cv::Mat canvas = cv::Mat::zeros( canny_output.size(), CV_8U );
    for (size_t i = 0; i < contours.size(); i++) {
        cv::drawContours( canvas, contours, i, cv::Scalar(255), 2, cv::LINE_8, hierarchy, 0 );
    }

    // cv::imshow(" Canny Output Image", canvas );
    canny_output.release();
    return canvas;
}
