// affine.cpp : Affine Transformations
// Austin Hester CS642o april 2021
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "./include/affine.hpp"


cv::Mat
resize_affine(cv::Mat src, float scale)
{
    // if we have more than 8k in the future, then my only question is why? what's the point?
    assert( src.rows * scale <= 7680 && src.cols * scale <= 7680 );

    cv::Point2f src_points[3];
    src_points[0] = cv::Point2f( 0.f, 0.f );
    src_points[1] = cv::Point2f( src.cols - 1.f, 0.f );
    src_points[2] = cv::Point2f( 0.f, src.rows - 1.f );

    cv::Point2f dst_points[3];
    dst_points[0] = cv::Point2f( 0.f, 0.f );
    dst_points[1] = cv::Point2f( src.cols * scale, 0.f );
    dst_points[2] = cv::Point2f( 0.f, src.rows * scale );

    cv::Mat warp_mat = cv::getAffineTransform( src_points, dst_points );

    cv::Mat warp_dst = cv::Mat::zeros( src.rows * scale, src.cols * scale, src.type() );

    cv::warpAffine( src, warp_dst, warp_mat, warp_dst.size() );

    warp_mat.release();
    return warp_dst;
}
