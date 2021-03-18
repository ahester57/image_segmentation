// cla_parse.cpp : Parse given command line arguments.
// Austin Hester CS542o nov 2020
// g++.exe (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0

#include <opencv2/core/core.hpp>

#include "./include/cla_parse.hpp"


// parse command line arguments
int
parse_arguments(
    int argc,
    const char** argv,
    std::string* input_image_filename,
    std::string* output_image_filename,
    bool* double_input_size,
    bool* blur_output,
    bool* equalize_output,
    bool* manual_mode
) {
    cv::String keys =
        "{@input_image    |<none>| Input Image}"
        "{@output_image   |<none>| Output Image}"
        "{double d        |      | Double Input Image Size}"
        "{blur b          |      | Blur Output Image}"
        "{equalize e      |      | Equalize Output Image}"
        "{manual m        |      | Manual Filter Mode}"
        "{help h          |      | Show Help Message}";

    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("h")) {
        parser.printMessage();
        return 0;
    }

    if (!parser.check()) {
        parser.printErrors();
        parser.printMessage();
        return -1;
    }

    try {
        *input_image_filename = (std::string) parser.get<std::string>(0).c_str();
        assert(input_image_filename->size() > 0);
    } catch (...) {
        std::cerr << "Failed to parse imagefile argument!:" << std::endl;
        return -1;
    }

    try {
        *output_image_filename = (std::string) parser.get<std::string>(1).c_str();
        if (output_image_filename->size() == 0) {
            *output_image_filename = "filtered_" + *input_image_filename;
        }
    } catch (...) {
        std::cerr << "Failed to parse templatefile argument!:" << std::endl;
        return -1;
    }

    try {
        *double_input_size = parser.has("d");
    } catch (...) {
        std::cerr << "Failed to parse double argument!:" << std::endl;
        return -1;
    }

    try {
        *blur_output = parser.has("b");
    } catch (...) {
        std::cerr << "Failed to parse blur argument!:" << std::endl;
        return -1;
    }

    try {
        *equalize_output = parser.has("e");
    } catch (...) {
        std::cerr << "Failed to parse equalize argument!:" << std::endl;
        return -1;
    }

    try {
        *manual_mode = parser.has("m");
    } catch (...) {
        std::cerr << "Failed to parse manual argument!:" << std::endl;
        return -1;
    }

    std::cout << std::endl << "Shortcuts:" << std::endl << "\tq\t- quit" << std::endl << std::endl;

    return 1;
}
