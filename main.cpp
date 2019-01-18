#include<WinSock2.h>
#include<WS2tcpip.h.>
#include <iostream>
//ZED includes
#include <zed/Camera.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#include<conio.h>           // may have to modify this line if not using Windows
#include <fstream>
#include<Windows.h>
#include<string>
#include<time.h>
#pragma comment(lib,"Ws2_32.lib")
#define PORT "1234"
#define BUFFERSIZE 512
#include <chrono>
// may have to modify this line if not using Windows



///////////////////////////////////////////////////////////////////////////////////////////////////
int main() {

	using namespace cv;
	using namespace std;
	ofstream myfile;
	myfile.open("3d.txt");

	//########################### IPC INTIALIZATION ##############################//
	// init
	struct addrinfo *result = NULL, ref;
	WSADATA wsdata;

	WSAStartup(MAKEWORD(2, 2), &wsdata); // Intialization of Server
	// Setting up address
	ZeroMemory(&ref, sizeof(ref));
	ref.ai_family = AF_INET;
	ref.ai_socktype = SOCK_STREAM;
	ref.ai_protocol = IPPROTO_TCP;
	ref.ai_flags = AI_PASSIVE;
	getaddrinfo("127.78.67.34", PORT, &ref, &result);


	//################# CREATE CLIENT SOCKET #########//

	SOCKET Ani = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	connect(Ani, result->ai_addr, result->ai_addrlen);
	int status = 1;
	std::string sendMsg;
	std::string signD;
	std::string signX;
	std::string signY;
	std::string conc;

	int c = 0;


	sl::zed::Camera* zed = new sl::zed::Camera(sl::zed::VGA);
	sl::zed::InitParams params;
	params.mode = sl::zed::PERFORMANCE;
	params.unit = sl::zed::MILLIMETER; // Viewer scaled for processing METERs
	params.verbose = true;
	params.coordinate = sl::zed::RIGHT_HANDED | sl::zed::APPLY_PATH;
	sl::zed::ERRCODE err = zed->init(params);

	if (err != sl::zed::SUCCESS)
	{
		std::cout << "ZED error";
		delete zed;
		return -1;
	}


	int width = zed->getImageSize().width;
	int height = zed->getImageSize().height;
	int depthG = 0;
	int disparityG = 0;
	int low_h = 0, low_s = 175, low_v = 182;
	int high_h = 100, high_s = 255, high_v = 255;


	int pts = 0;
	int L = 0;
	int R = 1;
	int cX = 0;
	int cY = 0;
	int cXR = 0;
	int cYR = 0;
	double fx_d = 1.0 / 350.118;
	double fy_d = 1.0 / 350.118;
	double cx_d = 335.157;
	double cy_d = 190.084;

	double xaxis = 0;
	double yaxis = 0;

	vector<vector<Point> > contours;
	vector<vector<Point> > Mcenter;
	vector<vector<Point> > contoursR;
	vector<Vec4i> hierarchy;
	vector<Vec4i> hierarchyR;



	std::cout << width;
	std::cout << height;

	cv::Mat image(height, width, CV_8UC4, 1);
	cv::Mat imageHSV(height, width, CV_8UC4, 1);
	cv::Mat imageHSV_THRESH(height, width, CV_8UC4, 1);


	cv::Mat imageR(height, width, CV_8UC4, 1);
	cv::Mat imageRHSV(height, width, CV_8UC4, 1);
	cv::Mat imageRHSV_THRESH(height, width, CV_8UC4, 1);


	cv::Mat depth(height, width, CV_8UC4, 1);
	
	cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ImageHSV", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ImageHSV_THRESH", cv::WINDOW_AUTOSIZE);


	cv::namedWindow("ImageR", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ImageRHSV", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ImageRHSV_THRESH", cv::WINDOW_AUTOSIZE);

	

	cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);

	cv::Size displaySize(360, 202);
	cv::Mat imageDisplay(displaySize, CV_8UC4);
	cv::Mat imageDisplayHSV(displaySize, CV_8UC4);
	cv::Mat imageDisplayHSV_THRESH(displaySize, CV_8UC4);


	cv::Mat imageDisplayR(displaySize, CV_8UC4);
	cv::Mat imageDisplayRHSV(displaySize, CV_8UC4);
	cv::Mat imageDisplayRHSV_THRESH(displaySize, CV_8UC4);


	cv::Mat depthDisplay(displaySize, CV_8UC4);

	myfile << -500;
	myfile << "," << -500;
	myfile << ",";
	myfile << 1000;
	myfile << "\n";
	myfile << 0;
	myfile << "," << 0;
	myfile << ",";
	myfile << 0;
	myfile << "\n";
	int frame = 0;
	int x = 0;
	int cntr = 0;

	while (1 && (status != SOCKET_ERROR))
	{

		int t = 0;
		int tR = 0;
		int largest_area = 0;
		int largest_contour_index = 0;
		int largest_areaR = 0;
		int largest_contour_indexR = 0;
		// Grab frame and compute depth in FILL sensing mode
		if (!zed->grab(sl::zed::SENSING_MODE::STANDARD))
		{
			sl::zed::Mat left = zed->retrieveImage(sl::zed::SIDE::LEFT);
			memcpy(image.data, left.data, width*height * 4 * sizeof(uchar));

			sl::zed::Mat right = zed->retrieveImage(sl::zed::SIDE::RIGHT);
			memcpy(imageR.data, right.data, width*height * 4 * sizeof(uchar));
			
			sl::zed::Mat depthmap = zed->normalizeMeasure(sl::zed::MEASURE::DISPARITY);
			memcpy(depth.data, depthmap.data, width*height * 4 * sizeof(uchar));
			

			std::string savingName =  std::to_string(++cntr) + ".jpg";
			cv::imwrite(savingName, image);

					
		
		
			//std::cout << "\n";

			/*
			
			cv::resize(image, imageDisplay, displaySize);
			cv::imshow("Image", imageDisplay);

			cv::resize(imageR, imageDisplayR, displaySize);
			cv::imshow("ImageR", imageDisplayR);
			
			cv::resize(depth, depthDisplay, displaySize);
			cv::imshow("Depth", depthDisplay);
			
	*/

				//cv::waitKey(1);
		

		}
		frame++;
		printf("\nFRAME = ");
		printf("%d\n", frame);
		pts++;

		}
	



	delete zed;

	return(0);


}


/*

////###################################### Original code 16/4/2017 , Functions: Image processing , Depth , Multiprocessing , Current FPS : 30 FPS ###########################//
#include<WinSock2.h>
#include<WS2tcpip.h.>
#include <iostream>
//ZED includes
#include <zed/Camera.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include<iostream>
#include<conio.h>           // may have to modify this line if not using Windows
#include <fstream>
#include<Windows.h>
#include<string>
#include<time.h>
#pragma comment(lib,"Ws2_32.lib")
#define PORT "1234"
#define BUFFERSIZE 512
#include <chrono>
// may have to modify this line if not using Windows



///////////////////////////////////////////////////////////////////////////////////////////////////
int main() {

	using namespace cv;
	using namespace std;
	ofstream myfile;
	myfile.open("3d.txt");
	
	//########################### IPC INTIALIZATION ##############################//
	// init
	struct addrinfo *result = NULL, ref;
	WSADATA wsdata;
	
	WSAStartup(MAKEWORD(2, 2), &wsdata); // Intialization of Server
	// Setting up address
	ZeroMemory(&ref, sizeof(ref));
	ref.ai_family = AF_INET;
	ref.ai_socktype = SOCK_STREAM;
	ref.ai_protocol = IPPROTO_TCP;
	ref.ai_flags = AI_PASSIVE;
	getaddrinfo("127.78.67.34", PORT, &ref, &result);
	

	//################# CREATE CLIENT SOCKET #########//

	SOCKET Ani = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	connect(Ani, result->ai_addr, result->ai_addrlen);
	int status = 1;
	std::string sendMsg;
	std::string signD;
	std::string signX;
	std::string signY;
	std::string conc;

	int c = 0;
	

	sl::zed::Camera* zed = new sl::zed::Camera(sl::zed::VGA);
	sl::zed::InitParams params;
	params.mode = sl::zed::PERFORMANCE;
	params.unit = sl::zed::MILLIMETER; // Viewer scaled for processing METERs
	params.verbose = true;
	params.coordinate = sl::zed::RIGHT_HANDED | sl::zed::APPLY_PATH;
	sl::zed::ERRCODE err = zed->init(params);

	if (err != sl::zed::SUCCESS)
	{
		std::cout << "ZED error";
		delete zed;
		return -1;
	}


	int width = zed->getImageSize().width;
	int height = zed->getImageSize().height;
	int depthG = 0;
	int disparityG = 0;
	int low_h = 0, low_s = 79, low_v = 190;
	int high_h = 28, high_s = 255, high_v = 255;

	int pts = 0;
	int L = 0;
	int R = 1;
	int cX = 0;
	int cY = 0;
	int cXR = 0;
	int cYR = 0;
	double fx_d = 1.0 / 350.118;
	double fy_d = 1.0 / 350.118;
	double cx_d = 335.157;
	double cy_d = 190.084 ;

	double xaxis = 0;
	double yaxis = 0;

	vector<vector<Point> > contours;
	vector<vector<Point> > Mcenter;
	vector<vector<Point> > contoursR;
	vector<Vec4i> hierarchy;
	vector<Vec4i> hierarchyR;



	std::cout << width;
	std::cout << height;

	cv::Mat image(height, width, CV_8UC4, 1);
	cv::Mat imageHSV(height, width, CV_8UC4, 1);
	cv::Mat imageHSV_THRESH(height, width, CV_8UC4, 1);


	cv::Mat imageR(height, width, CV_8UC4, 1);
	cv::Mat imageRHSV(height, width, CV_8UC4, 1);
	cv::Mat imageRHSV_THRESH(height, width, CV_8UC4, 1);


	cv::Mat depth(height, width, CV_8UC4, 1);
	
	cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ImageHSV", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ImageHSV_THRESH", cv::WINDOW_AUTOSIZE);


	cv::namedWindow("ImageR", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ImageRHSV", cv::WINDOW_AUTOSIZE);
	cv::namedWindow("ImageRHSV_THRESH", cv::WINDOW_AUTOSIZE);



	cv::namedWindow("Depth", cv::WINDOW_AUTOSIZE);

	cv::Size displaySize(720, 404);
	cv::Mat imageDisplay(displaySize, CV_8UC4);
	cv::Mat imageDisplayHSV(displaySize, CV_8UC4);
	cv::Mat imageDisplayHSV_THRESH(displaySize, CV_8UC4);


	cv::Mat imageDisplayR(displaySize, CV_8UC4);
	cv::Mat imageDisplayRHSV(displaySize, CV_8UC4);
	cv::Mat imageDisplayRHSV_THRESH(displaySize, CV_8UC4);
	

	cv::Mat depthDisplay(displaySize, CV_8UC4);
	
	myfile << -500;
	myfile << "," << -500;
	myfile << ",";
	myfile << 1000;
	myfile << "\n";
	myfile << 0;
	myfile << "," << 0;
	myfile << ",";
	myfile << 0;
	myfile << "\n";
	int frame = 0;
	int depthLAST = 0;
	int xlast = 0;
	int ylast = 0;
	while (1 && (status != SOCKET_ERROR))
	{    
		
		int t = 0;
		int tR = 0;
		int largest_area = 0;
		int largest_contour_index = 0;
		int largest_areaR = 0;
		int largest_contour_indexR = 0;
		// Grab frame and compute depth in FILL sensing mode
		if (!zed->grab(sl::zed::SENSING_MODE::STANDARD))
		{    
			sl::zed::Mat left = zed->retrieveImage(sl::zed::SIDE::LEFT);
			memcpy(image.data, left.data, width*height * 4 * sizeof(uchar));

			sl::zed::Mat right = zed->retrieveImage(sl::zed::SIDE::RIGHT);
			memcpy(imageR.data, right.data, width*height * 4 * sizeof(uchar));
			
			sl::zed::Mat depthmap = zed->normalizeMeasure(sl::zed::MEASURE::DISPARITY);
			memcpy(depth.data, depthmap.data, width*height * 4 * sizeof(uchar));
			
		
			
			rectangle(image, // Right rect
				Point(475,0),
				Point(672,376),
				Scalar(0, 0, 0),
				-1,
				0);


			rectangle(image, // Upper rect
				Point(220, 0),
				Point(475, 100),
				Scalar(0, 0, 0),
				-1,
				0);
			rectangle(image,    // Lower rect
				Point(220, 275),
				Point(475, 376),
				Scalar(0, 0, 0),
				-1,
				0);
			rectangle(image, // Left rect
				Point(0, 0),
				Point(220, 376),
				Scalar(0, 0, 0),
				-1,
				0);
//################ Image R occulsion ################
			rectangle(imageR,
				Point(475, 0),
				Point(672, 376),
				Scalar(0, 0, 0),
				-1,
				0);
			rectangle(imageR,
				Point(220, 0),
				Point(475, 100),
				Scalar(0, 0, 0),
				-1,
				0);
			rectangle(imageR,
				Point(220, 280),
				Point(475, 376),
				Scalar(0, 0, 0),
				-1,
				0);
			rectangle(imageR,
				Point(0, 0),
				Point(220, 376),
				Scalar(0, 0, 0),
				-1,
				0);
             
		
			cvtColor(image, imageHSV, CV_BGR2HSV);
			cv::resize(imageHSV, imageDisplayHSV, displaySize);
		    cv::imshow("ImageHSV", image);

			cvtColor(imageR, imageRHSV, CV_BGR2HSV);
			cv::resize(imageRHSV, imageDisplayRHSV, displaySize);
			//cv::imshow("ImageRHSV", imageDisplayRHSV);
			
			inRange(imageHSV, cv::Scalar(low_h, low_s, low_v), cv::Scalar(high_h, high_s, high_v), imageHSV_THRESH);
			inRange(imageRHSV, cv::Scalar(low_h, low_s, low_v), cv::Scalar(high_h, high_s, high_v), imageRHSV_THRESH);

			dilate(imageHSV, imageHSV, Mat(), Point(-1, -1), 2, 1, 1);
			dilate(imageRHSV, imageRHSV, Mat(), Point(-1, -1), 2, 1, 1);
			//	cv::dilate(imageHSV_THRESH, imageHSV_THRESH, Mat(), Point(-1, -1), 2, 1, 1);
			//	cv::dilate(imageRHSV_THRESH, imageRHSV_THRESH, Mat(), Point(-1, -1), 2, 1, 1);


			cv::findContours(imageHSV_THRESH, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
			cv::findContours(imageRHSV_THRESH, contoursR, hierarchyR, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));


			Mat drawing = Mat::zeros(imageHSV_THRESH.size(), CV_8UC3);
			Mat drawingR = Mat::zeros(imageRHSV_THRESH.size(), CV_8UC3);


			for (size_t i = 0; i < contours.size(); i++)
			{

				drawContours(drawing, contours, (int)i, (255, 255, 255), 2, 8, hierarchy, 0, Point());

			}


			for (int i = 0; i< contours.size(); i++) // iterate through each contour. 
			{
			

				double a = contourArea(contours[i], false);  //  Find the area of contour
				if (a>largest_area){
					t = 1;
					largest_area = a;
					largest_contour_index = i;                //Store the index of largest contour
				}

			}


				for (size_t i = 0; i < contoursR.size(); i++)
				{
					drawContours(drawingR, contoursR, (int)i, (255, 255, 255), 2, 8, hierarchyR, 0, Point());

				}


				for (int i = 0; i< contoursR.size(); i++) // iterate through each contour. 
				{
					
					double a = contourArea(contoursR[i], false);  //  Find the area of contour
					if (a>largest_areaR){
						tR = 1;
						largest_areaR = a;
						largest_contour_indexR = i;                //Store the index of largest contour

					}
				}
						vector<Moments> mu(contours.size());
						vector<Moments> mu2(contoursR.size());

				
						if (contours.size()>0){
			      mu[0] = moments(contours[largest_contour_index], false);
				  cX = mu[0].m10 / mu[0].m00;
				  cY = mu[0].m01 / mu[0].m00;
				  
				  std::cout << "Cx :";
				  std::cout << cX;
				  std::cout << " Cy :";
				  std::cout << cY;
				 

						}

						if (contoursR.size()>0){

			      mu2[0] = moments(contoursR[largest_contour_indexR], false);
				  cXR = mu2[0].m10 / mu2[0].m00;
				  cYR = mu2[0].m01 / mu2[0].m00;
				  
				  std::cout << "CxR :";
				  std::cout << cXR;
				  std::cout << " CyR :";
				  std::cout << cYR;
				  
				
						}
						

						if (contours.size()>0 && contoursR.size()>0)
						{
							disparityG = abs(cX - cXR);
							depthG = (zed->getParameters()->LeftCam.fx * zed->getParameters()->baseline) / (-disparityG);

							if (abs(depthG) < 2000)
							{
							
							xaxis = -(float)((cX - cx_d) * -depthG * fx_d);
							yaxis = (float)((cY - cy_d) * -depthG * fy_d);
							
							std::cout << " Depth :";
							std::cout <<(int) depthG;
							std::cout << " X :";
							std::cout << (int) xaxis;
							std::cout << " Y :";
							std::cout << (int) yaxis;
							
							std::string depthG_S = std::to_string((int)abs(depthG)+1000);
							std::string xaxis_S = std::to_string((int)abs(xaxis)+1000);
							std::string yaxis_S = std::to_string((int)abs(yaxis)+1000);

							if (depthG > 0)
							{
								signD = "P";
							}
							else
							{
								signD = "N";
							}
							if (xaxis > 0)
							{
								signX = "P";
							}
							else
							{
								signX = "N";
							}
							if (yaxis > 0)
							{
								signY = "P";
							}
							else
							{
								signY = "N";
							}


							xlast = (int)abs(xaxis) + 1000;
							ylast = (int)abs(yaxis) + 1000;
							sendMsg = "S";
							conc = sendMsg;
							//status = send(Ani, sendMsg.c_str(), sendMsg.length(), 0);

							//status = send(Ani, signD.c_str(), signD.length(), 0);
							
							//status = send(Ani, signX.c_str(), signX.length(), 0);

							//status = send(Ani, signY.c_str(), signY.length(), 0);

							conc = conc + signD+signX+signY;
							sendMsg = "Z";
							//status = send(Ani, sendMsg.c_str(), sendMsg.length(), 0);
							conc = conc+sendMsg;

							sendMsg = depthG_S;
							// = send(Ani, sendMsg.c_str(), sendMsg.length(), 0);
							conc = conc + sendMsg;

							sendMsg = "X";
							//status = send(Ani, sendMsg.c_str(), sendMsg.length(), 0);
							conc = conc + sendMsg;

							sendMsg = xaxis_S;
							//status = send(Ani, sendMsg.c_str(), sendMsg.length(), 0);
							conc = conc + sendMsg;

							sendMsg = "Y";
							//status = send(Ani, sendMsg.c_str(), sendMsg.length(), 0);
							conc = conc + sendMsg;

							sendMsg = yaxis_S;
							//status = send(Ani, sendMsg.c_str(), sendMsg.length(), 0);
							conc = conc + sendMsg;

							sendMsg = "E";
							//status = send(Ani, sendMsg.c_str(), sendMsg.length(), 0);
							conc = conc + sendMsg;
							//std::cout << conc << '\n';

							status = send(Ani, conc.c_str(), conc.length(), 0);
							
							myfile << xaxis;
							myfile << "," << yaxis;
							myfile << ",";
							myfile << depthG;
							myfile << "\n";
							
							}
						}
						//std::cout << "\n";

					cv::imshow("ContoursR", drawingR);
					cv::imshow("Contours", drawing);

					
					cv::imshow("ImageHSV_THRESH", imageHSV_THRESH);
					cv::imshow("ImageRHSV_THRESH", imageRHSV_THRESH);
					
					
					cv::resize(image, imageDisplay, displaySize);
					cv::imshow("Image", imageDisplay);

					cv::resize(imageR, imageDisplayR, displaySize);
					cv::imshow("ImageR", imageDisplayR);
					
					cv::resize(depth, depthDisplay, displaySize);
					cv::imshow("Depth", depthDisplay);
					
						
					cv::waitKey(1);
				


				}
				frame++;
				printf("\nFRAME = ");
				printf("%d\n", frame);
				pts++;
				if (pts > 200){
					std::cout << "FILE CLOSED :";
					myfile.close();
				}
			}
			     
			
			shutdown(Ani, SD_SEND);
			closesocket(Ani);
			WSACleanup;
			
		
					delete zed;
					
					return(0);


}

*/

/*
#include<WinSock2.h>
#include<WS2tcpip.h.>
#include<stdio.h>
#include<Windows.h>
#include<string>
#include<iostream>
#pragma comment(lib,"Ws2_32.lib")
#define PORT "1234"
#define BUFFERSIZE 512
struct addrinfo *result = NULL, ref;
int main()
{   // init
WSADATA wsdata;
WSAStartup(MAKEWORD(2, 2), &wsdata); // Intialization of Server
// Setting up address
ZeroMemory(&ref, sizeof(ref));
ref.ai_family = AF_INET;
ref.ai_socktype = SOCK_STREAM;
ref.ai_protocol = IPPROTO_TCP;
ref.ai_flags = AI_PASSIVE;
getaddrinfo("127.78.67.34", PORT, &ref, &result);


//################# CREATE CLIENT SOCKET #########//

SOCKET Ani = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
connect(Ani, result->ai_addr, result->ai_addrlen);
int status=1;
std::string sendMsg;
int c = 0;
while (status!=SOCKET_ERROR)
{
if (c == 0) { sendMsg = "1"; }
else if (c == 1) { sendMsg = "2"; }
else if (c == 2) { sendMsg = "3"; }
else if (c == 3) { sendMsg = "4"; }

c++;
if (c == 4) { c = 0; }
status = send(Ani, sendMsg.c_str(), sendMsg.length(),0);
Sleep(1000);
}
shutdown(Ani, SD_SEND);
closesocket(Ani);
WSACleanup;
return 0;
}

*/



