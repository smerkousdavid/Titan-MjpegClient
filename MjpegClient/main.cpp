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
