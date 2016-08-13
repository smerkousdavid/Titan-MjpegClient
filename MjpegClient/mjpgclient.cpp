/**
    CS-11 Format
    File: mjpgclient.cpp
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
#include "mjpgclient.h"

int MjpgClient::getLine(char* ip) {
    try {
        cv::VideoCapture mjpgcap;
        mjpgcap.open(ip);
        int count_max = 0;
        while(!mjpgcap.isOpened()) {
            if(count_max++ > this->max_retry) {
                throw std::invalid_argument("stream invalid");
            }
        }
        this->mjpgcap = mjpgcap;
        return 0;
    } catch(std::exception& err) {
        std::cerr << "Failed to open mjpg stream..." << std::endl;
        return 1;
    }
}

template <class T>
int MjpgClient::numDigits(T number) {
    int digits = 0;
    if (number < 0) digits = 1;
    while (number) {
        number /= 10;
        digits++;
    }
    return digits;
}

bool MjpgClient::replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

bool MjpgClient::is_digits(const std::string &str) {
    for(int charn = 0; charn < str.length(); charn++) {
        char cur = str[charn];
        if(isdigit(cur)) {
            return true;
        }
    }
    return false;
}

bool MjpgClient::postReq(const char name[], std::string &request_body) {
    try {
        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        std::string tempstr(this->ip);
        std::stringstream st;
        st << this->port;
        this->replace(tempstr, "http://", "");
        tcp::resolver::query query(tempstr, st.str());
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "POST " << name << " HTTP/1.1\r\n";
        request_stream << "Host: " << this->ip << "\r\n";
        request_stream << "Accept: *//*\r\n";
        request_stream << "Content-Length: " << request_body.length() << "\r\n";
        request_stream << "Connection: close\r\n\r\n";
        request_stream << request_body << "\r\n";

        boost::asio::write(socket, request);
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
          std::cout << "Invalid response" << std::endl;
          return false;
        }
        if (status_code != 200) {
          std::cout << "Response returned with non 200 status code " << status_code << std::endl;
          return false;
        }
        return true;
    } catch (std::exception& e) {
        std::cerr << "Failed posting: " << e.what() << std::endl;
        return false;
    }
}


void MjpgClient::sleep(int millis) {
    boost::this_thread::sleep_for(boost::chrono::milliseconds(millis));
}

void MjpgClient::getReq(const char name[], std::string *response_full) {
    try {

        boost::asio::io_service io_service;

        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        std::string tempstr(this->ip);
        std::stringstream st;
        st << this->port;
        this->replace(tempstr, "http://", "");
        tcp::resolver::query query(tempstr, st.str());
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << name << " HTTP/1.1\r\n";
        request_stream << "Host: " << this->ip << "\r\n";
        request_stream << "Accept: *//*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        boost::asio::write(socket, request);
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
          std::cout << "Invalid response" << std::endl;
          return;
        }

        if (status_code != 200) {
          std::cout << "Response returned with non 200 status code " << status_code << std::endl;
          return;
        }

        boost::asio::read_until(socket, response, "\r\n\r\n");
        std::ostringstream ss;
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
        if (response.size() > 0) {
            ss << &response;
            std::string curline(ss.str());
            *response_full += curline.substr(curline.find_last_of("\r"));
        }
        boost::system::error_code error;
        while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error)) {
            ss << &response;
            std::string curline(ss.str());
            *response_full += curline.substr(curline.find_last_of("\r"));
        }
        if (error != boost::asio::error::eof)
          throw boost::system::system_error(error);
        }
    catch (std::exception& e) {
        std::cerr << "Failed get request: " << e.what() << std::endl;
    }
}

int MjpgClient::getServerFPS() {
    try {
        std::string response;
        getReq("fps", &response);
        return atoi(response.c_str());
    } catch(std::exception& e) {
        std::cerr << "Failed getting server fps: " << std::endl;
        return 0;
    }
}

int* MjpgClient::getServerResolution() {
    int dims[2] = {0, 0};
    try {
        std::string response;
        this->getReq("resolution", &response);
        std::string dim = response.substr(0, response.find("x"));
        dims[0] = atoi(dim.c_str());
        dim = response.substr(response.find("x") + 1);
        dims[1] = atoi(dim.c_str());
    } catch(std::exception& err) {
        std::cerr << "Error getting resolution from server" << std::endl;
    }
    return dims;
}

int MjpgClient::getFPS() {
    return this->real_fps;
}

bool MjpgClient::setServerFPS(int fps) {
    try {
        std::stringstream st;
        st << fps;
        std::string start = st.str();
        return postReq("fps", start);
    } catch(std::exception& e) {
        std::cerr << "Failed setting server fps" << std::endl;
        return false;
    }
}

bool MjpgClient::setServerResolution(int width, int height) {
    try {
        std::stringstream st;
        st << width << "x" << height;
        std::string start = st.str();
        return postReq("resolution", start);
    } catch(std::exception& e) {
        std::cerr << "Error setting server resolution" << std::endl;
        return false;
    }
}

bool MjpgClient::setFPS(int fps) {
    try {
        return this->mjpgcap.set(CV_CAP_PROP_FPS, fps);
    } catch(std::exception& e) {
        std::cerr << "Couldn't set local fps: " << e.what() << std::endl;
        return false;
    }
}

int MjpgClient::getServerQuality() {
    try {
        std::string response;
        getReq("quality", &response);
        return atoi(response.c_str());
    } catch(std::exception& e) {
        std::cerr << "Couldn't get server quality: " << e.what() << std::endl;
        return 0;
    }
}

int MjpgClient::getServerConnections() {
    try {
        std::string response;
        getReq("connections", &response);
        return atoi(response.c_str());
    } catch(std::exception& e) {
        std::cerr << "Couldn't get connections" << std::endl;
        return 0;
    }
}

bool MjpgClient::setServerConnections(int connections) {
    try {
        std::stringstream st;
        st << connections;
        std::string start = st.str();
        return postReq("connections", start);
    } catch(std::exception& e) {
        std::cerr << "Couldn't set connections" << std::endl;
        return false;
    }
}

int* MjpgClient::getResolution() {
    int dims[2] = {0, 0};
    try {
        dims[0] = this->mjpgcap.get(CV_CAP_PROP_FRAME_WIDTH);
        dims[1] = this->mjpgcap.get(CV_CAP_PROP_FRAME_HEIGHT);
    } catch(std::exception& err) {
        std::cerr << "Problem getting resolution" << std::endl;
    }
}

bool MjpgClient::setResolution(int width, int height) {
    try {
        bool good = true;
        if(!this->mjpgcap.set(cv::CAP_PROP_FRAME_WIDTH, width)) good = false;
        if(!this->mjpgcap.set(cv::CAP_PROP_FRAME_HEIGHT, width)) good = false;
        return good;
    } catch(std::exception& err) {
        std::cout << "Error setting new resolution" << std::endl;
        return false;
    }
}

bool MjpgClient::setServerQuality(int quality) {
    try {
        std::stringstream st;
        st << quality;
        std::string start = st.str();
        return postReq("quality", start);
    } catch(std::exception& e) {
        std::cerr << "Couldn't set server quality" << std::endl;
        return false;
    }
}

bool MjpgClient::setDiscDim(int width, int height) {
    try {
        cv::resize(this->no_connection, this->no_connection, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);
        return true;
    } catch(std::exception& err) {
        std::cerr << "Failed setting new no connection image" << std::endl;
        return false;
    }
}

bool MjpgClient::setDiscPath(const char* path) {
    try {
        this->no_connection = cv::imread(path);
        if(this->disc_width > 0 && this->disc_height > 0) this->setDiscDim(this->disc_width, this->disc_height);
        return true;
    } catch(std::exception& err) {
        return false;
    }
}

void MjpgClient::init(const char* ip, int port, const char* name, char* buf) {
    sprintf(buf, "%s:%d/%s", ip, port, name);
    if(getLine(buf) != 0)
        std::cerr << "Bad connection..." << std::endl;
    this->start = boost::chrono::high_resolution_clock::now();
}

MjpgClient::~MjpgClient() {
    try {
        this->mjpgcap.release();
    } catch(std::exception& safetyrelease) {}
}

MjpgClient::MjpgClient(const char* ip, int port, const char* name) {
    int addr_length = (sizeof(ip) + sizeof(name)) + this->numDigits(port);
    char *addr = new char[addr_length] ;
    this->max_retry = 20;
    try {
        this->no_connection = cv::imread("noconnection.jpg");
        this->cur_frame = this->no_connection;
        this->init(ip, port, name, addr);
    } catch(std::exception& badconnection) {
        std::cerr << "Initialization failed" << std::endl;
        sleep(100);
    }
    this->port = port;
    this->ip = ip;
    this->name = name;
    this->addr = addr;
    std::cout << "Mjpeg client init at addr: " << addr << std::endl;
}

cv::Mat MjpgClient::getFrameMat() {
    try {
        this->mjpgcap.read(this->cur_frame);

        if(this->cur_frame.empty()) {
            this->cur_frame = this->last_frame;

        if(this->cur_frame.empty())
            this->cur_frame = this->no_connection;
            if(this->bad_count++ > this->max_retry) {
                throw std::invalid_argument("Max empty frame limit... attempting to connect again");
            }
            sleep(50);
        }
    } catch(std::exception& badframe) {
        std::cerr << badframe.what() << std::endl;
        this->cur_frame = this->no_connection;
        if(this->bad_count++ > this->max_retry) {
                bad_count = 0;
                try {
                    this->mjpgcap.release();
                    this->init(this->ip.c_str(), this->port, this->name.c_str(), const_cast<char *>(reinterpret_cast<const char *>(this->addr.c_str())));
                } catch(std::exception& reinit) {
                    std::cerr << "Failed reinitializing stream..." << std::endl;
                }
        }
    }
    boost::chrono::high_resolution_clock::time_point now = boost::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->start).count();
    this->frames++;
    if(duration > 100 && frames > 30) {
        this->real_fps = (float) ((frames * 1000) / duration);
        this->start = now;
        this->frames = 0;
    }
    return this->cur_frame;
}

std::string MjpgClient::getFrame() {
    try {
        this->getFrameMat();
        std::vector<uchar> buff;
        cv::imencode(".jpg", this->cur_frame, buff);
        std::string content(buff.begin(), buff.end());
        return content;
    } catch(std::exception& err) {
        std::cerr << "Image pull error: " << err.what() << std::endl;
        return std::string();
    }
}
