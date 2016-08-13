# Titan MjpegClient
A very fast MjpgServer library written in c++...
Created by: David Smerkous

## Intro
This is the main counterpart to the Titan MjpegServer but can also connect
to any other mjpeg stream. Here is the server: https://github.com/smerkousdavid/Titan-MjpegServer
This was built using gcc but should be compatible cross platform with the help of boost libs

## Requirements
The Titan MjpegClient **requires**

    * C++11 standards to be built in (gcc flag... -std=c++11)
    * Boost libraries 1.54.0 and up (Built in 55)
    * OpenCv 3.10

## Installation
Here are the steps to install the Titan MjpgClient
    1. Download libs: 


         git clone https://github.com/smerkousdavid/Titan-MjpegClient
    
    2. Copy mjpgclient.cpp and mjpgclient.h into your project:

	
	cd Titan-MjpegClient
	cp mjpgclient.cpp ~/myproject/src
	cp mjpgclient.h ~/myproject/src

    3. Add linkers:
	If building from source you must include all boost libs and all opencv libs (Windows can use world dll*)
        Example g++ build option:


        -s /usr/lib/x86_64-linux-gnu/libboost_math_tr1.so /usr/lib/x86_64-linux-gnu/libboost_system.so /usr/lib/x86_64-linux-gnu/libboost_iostreams.so /usr/lib/x86_64-linux-gnu/libboost_regex.so /usr/lib/x86_64-linux-gnu/libboost_signals.so /usr/lib/x86_64-linux-gnu/libboost_thread.so /usr/lib/x86_64-linux-gnu/libboost_locale.so /usr/lib/x86_64-linux-gnu/libboost_timer.so /usr/lib/x86_64-linux-gnu/libboost_atomic.so /usr/lib/x86_64-linux-gnu/libboost_chrono.so /usr/local/lib/libopencv_imgproc.so.3.1.0 /usr/local/lib/libopencv_core.so.3.1.0 /usr/local/lib/libopencv_imgcodecs.so.3.1.0 /usr/local/lib/libopencv_videoio.so.3.1.0 /usr/local/lib/libopencv_features2d.so.3.1.0 /usr/local/lib/libopencv_highgui.so.3.1.0 /usr/local/lib/libopencv_flann.so.3.1.0 /usr/local/lib/libopencv_objdetect.so.3.1.0 /usr/local/lib/libopencv_ml.so.3.1.0 /usr/local/lib/libopencv_shape.so.3.1.0 /usr/local/lib/libopencv_photo.so.3.1.0 /usr/local/lib/libopencv_calib3d.so.3.1.0 /usr/local/lib/libopencv_videostab.so.3.1.0 /usr/local/lib/libopencv_superres.so.3.1.0 /usr/local/lib/libopencv_stitching.so.3.1.0
    4. You're done:
	Just add the mjpgserver.h into your project

## Example
		
    #include "mjpgclient.h" //Include client lib header (OpenCv included)


    int main(int argc, char *argv[]) {
	//example stream http://localhost:8080/mjpg

        const char ip[] = "http://localhost"; //Ip base of stream
        int port = 8081; //Port of stream
        const char name[] = "mjpg"; //extension

        MjpgClient client(ip, port, name); //Get cap line

        int counts = 0; //Just a fps pull counter
        client.setDiscDim(640, 480); //Set disconnected image size
        client.setServerQuality(1); //Tell Titan MjpegServer to reduce quality of image to save bandwidth
        client.setServerFPS(35); //Tell Titan MjpegServer to set target fps to 35
        while(1) { //Run forever
            cv::Mat curframe = client.getFrameMat(); //Test Mat (Use: getFrame for byte string)
            cv::imshow("OK", curframe); //Process frame headers for showing
            cv::waitKey(1); //Process frame and show
            if(counts++ % 20 == 0) { //Print fps every 20 frames
                std::cout << "FPS: " << client.getFPS() << std::endl; //Print local calculated fps
            }
        }
        return 0;
    }

## License
**Look at license file and sources**
License: MIT License (MIT)
Copyright (c) 2016 David Smerkous

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software 
without restriction, including without limitation the rights to use, copy, modify, 
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial 
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 

