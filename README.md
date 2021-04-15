# Segmentation
### Austin Hester
### CS 6420 - Computer Vision
### UMSL SP2021, Prof. Sanjiv Bhatia

----
## Purpose

To segment images using thresholding...

----

## Task


----

#### Usage

```
Usage: map_segment.exe [params] input_image output_image

        -b, --blur
                Blur Output Image
        -e, --equalize
                Equalize Output Image
        -h, --help (value:true)
                Show Help Message
        -s, --scale (value:1.f)
                Scale Input Image Size

        input_image (value:images/usa.png)
                Input Image
        output_image (value:<none>)
                Output Image

```
----

#### Sad Make Stats
From a fresh `make destroy`.
```
map_helper.cpp:57:9: error: 'imshow' is not a member of 'cv'
     cv::imshow( "Markers 8U", markers_8U );
         ^~~~~~
make: *** [Makefile:19: map_helper.o] Error 1

real    8m30.558s
user    0m0.000s
sys     0m0.015s
```
----

https://github.com/ahester57/image_segmentation
