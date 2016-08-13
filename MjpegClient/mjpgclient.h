/**
    CS-11 Format
    File: mjpgclient.h
    Purpose: Capture/process mats and display on http stream

    @author David Smerkous
    @version 1.0 8/11/2016

    License: MIT License (MIT)
    Copyright (c) 2016 David Smerkous

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
    INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef MJPGCLIENT_H_
#define MJPGCLIENT_H_

#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
#include <ctype.h>

using namespace boost::asio;
using boost::asio::ip::tcp;

class MjpgClient {
    int max_retry = 20;
    int bad_count = 0;
    std::string ip = "http://localhost";
    std::string name = "mjpg";
    std::string addr = "http";
    int port = 8080;
    int disc_width = -1;
    int disc_height = -1;
    int real_fps = 0;
    cv::Mat last_frame;
    cv::Mat cur_frame;
    cv::Mat no_connection;
    int frames = 0;
    boost::chrono::high_resolution_clock::time_point start;


    public:
        //! MjpgClient constructor
        /*!
        This will start the mjpgserver on the specified port

        @param ip a const char of the ip ex: "http://localhost"
        @param port an integer of the stream port used ex: 8081
        @param name the extension type ex "mjpg"
        @return the MjpgClient object
        */
        MjpgClient(const char*, int, const char*);

        //! MjpgClient deconstructor
        /*!
        This will safely release the MjpgStream (No matter what stream)
        And all of the buffered images
        */
        ~MjpgClient(void);

        //!Default init method
        /*!
        Unlike the constructor this is the core that connects to the
        stream and is used in mainloop when trying to reconnect

        @param ip a const char of the ip ex: "http://localhost"
        @param port an integer of the stream port used ex: 8081
        @param name the extension type ex "mjpg"
        @param buf char array of the new compiled address
        */
        void init(const char*, int, const char*, char*);

        //!Gets the current frame
        /*!
        This is the main code for pulling which will check for any
        errors in the buffered images, and display a noconnection image
        if there is one

        @return A OpenCv Mat with the frame image (if failed it will be the no connection image)
        */
        cv::Mat getFrameMat(void);

        //!Gets the current frame byte string
        /*!
        This calls the above method to process the Mat into a byte string.
        Best methods are to save this to a file with a stream. Mainly will
        be used for our dashboard

        @return A byte string of the latest image
        */
        std::string getFrame(void);

        //!REST GET call to get fps from Titan MjpgServer
        /*!
        Note: Server must be Titan MjpgServer
        Returns server fps if failed fps will be 0

        @return an integer of the server real fps
        */
        int getServerFPS(void);

        //!REST POST call to set fps to Titan MjpgServer
        /*!
        Note: Server must be Titan MjpgServer
        Try to set the target fps of the server, careful not
        to eat up the entire bandwidth

        @param fps to set between 1 > ... or -1 to uncontrol
        @return a bool if it was completed or not
        */
        bool setServerFPS(int);

        //!REST GET call to get quality from Titan MjpgServer
        /*!
        Note: Server must be Titan MjpgServer
        Get the current jpeg quality of the server
        @return an int of the server set quality between 0 - 100 (-1 if unregulated aka 100)
        */
        int getServerQuality(void);

        //!REST POST call to set quality to Titan MjpgServer
        /*!
        Note: Server must be Titan MjpgServer
        Sets the servers jpeg quality

        @param quality an integer between 0 - 100 (-1 to unregulate aka 100)
        */
        bool setServerQuality(int);

        //!Sleep current thread for millis
        void sleep(int);

        //!Get current pull rate of camera (Not REST)
        int getFPS(void);

        //!Not implemented... (Depending on cv version)
        bool setFPS(int);

        //!Internal method to retrieve frame size (width, heigh)
        int* getResolution();

        //!REST POST call to set the resolution of the Titan MjpgServer
        /*!
        Note: Server must be Titan MjpgServer
        Sets the servers jpeg width and height
        Warning: Depending on new size this will add lag to the stream
        and also slowdown/eat more cpu to process

        Set the width and height to -1x-1 to unregulate the size

        @param width new width in pixels
        @param height new height in pixels
        @return a bool if completed or not
        */
        bool setServerResolution(int, int);

        //!REST GET call to get the resolution from Titan MjpgServer
        /*!
        Note: Server must be Titan MjpgServer
        Returns a size of two array of the jpeg sent image

        @return array { width, height }
        */
        int* getServerResolution(void);

        //!Internal method to change frame size
        /*!
        Sets the capture method size (May work or may not working)

        @param width new width in pixels
        @param height new height in pixels
        @return a bool if completed or not
        */
        bool setResolution(int, int);

        //!Set disconnect image size (Not same as set resolution)
        /*!
        Sets the noconnected return size

        @param width new width in pixels
        @param height new height in pixels
        @return a bool if completed or not
        */
        bool setDiscDim(int, int);

        //!Set the disconnect image path
        /*!
        Sets the noconnected image to a specified path
        if { @code setDiscDim } is already set then it will
        resize the image automatically

        @param path path to image
        @return a bool if completed or not
        */
        bool setDiscPath(const char*);

        //!REST GET call to get the amount of connection on the Titan MjpgServer
        /*!
        Note: Server must be Titan MjpgServer
        This won't ever be more than the max unless

        @return amount of clients connected to server (-1 to be unlimited)
        */
        int getServerConnections();

        //!REST POST call to set the max connections on the Titan MjpgServer
        /*!
        Note: Server must be Titan MjpgServer

        Sets the maximum allowed connections to connect to the server
        Obviously you could just get and add one to override it... But this
        is more of a safety for people trying to spam the lines and using browsers

        @return a bool if completed or not
        */
        bool setServerConnections(int);


    private:
        //!Global OpenCv capture method
        cv::VideoCapture mjpgcap;

        //!Private method to test connect stream
        int getLine(char*);

        //!Private method to make a GET request to the Titan MjpgServer
        void getReq(const char[], std::string *);

        //!Private method to make  POST request to the Titan MJpgServer
        bool postReq(const char[], std::string &);

        //!Private method to replace the replace function in strings (easier to use)
        bool replace(std::string&, const std::string&, const std::string&);

        //!Private method to check if a string contains any digits
        bool is_digits(const std::string&);

        //!Private method to see the digit length of any number
        template <class T>
        int numDigits(T);
};

#endif  // MJPGSERVER_H_
