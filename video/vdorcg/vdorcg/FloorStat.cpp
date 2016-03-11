
#include "FloorStat.h"
#include <stdio.h>
#include <iomanip>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace std;
using namespace cv;

#define COLOR_WHITE 3 
#define COLOR_RED   2
#define COLOR_GREEN 1
#define COLOR_BLUE  0
#define COLOR_GRAY  -1
#define COLOR_BIAS_FACTOR  0.04

int	FloorStat::SetAnchor(int x, int y) {
	anchor = cv::Point(x, y);
	return 0;
}

int	FloorStat::RecogStat(cv::Mat	frame) {
	// It's CV_8UC3 
	Vec3b p;
	req_up = req_down = req_stop = door_is_opening = car_is_here = false;
	if (type == TYPE_GENERAL_CAR) {
		p = frame.at<Vec3b>(anchor + trans_up);
		req_up = (p[0] < 100 && p[1] > 235 && p[2] < 100);
		p = frame.at<Vec3b>(anchor + trans_down);
		req_down = (p[0] < 100 && p[1] > 235 && p[2] < 100);
		p = frame.at<Vec3b>(anchor + trans_stop);
		req_stop = (p[0] < 100 && p[1] > 235 && p[2] < 100);
		p = frame.at<Vec3b>(anchor + trans_door);
		door_is_opening = (p[0] > 235 && p[1] > 235 && p[2] > 235);
		p = frame.at<Vec3b>(anchor + trans_car);
		car_is_here = (p[0] > 220 && p[1] < 50 && p[2] < 50);
		DetectDoorStat(frame);
	} else if (type == TYPE_CAR_GROUP) {
		p = frame.at<Vec3b>(anchor + trans_ii_up);
		req_up = (p[0] < 100 && p[1] > 235 && p[2] < 100);
		//cout << floor << ":" << p << ":";
		p = frame.at<Vec3b>(anchor + trans_ii_down);
		req_down = (p[0] < 100 && p[1] > 235 && p[2] < 100);
		//cout << p << ":" << req_up << ":" << req_down << endl;
	}

	return 0;
}

static int	QuickColorRcg(Vec3b p) {
	if (p[0] > 235 && p[1] > 235 && p[2] > 235) {
		return COLOR_WHITE;
	} else if (p[0] > 200 && p[1] < 50 && p[2] < 50) {
		return COLOR_BLUE;
	} else {
		return COLOR_GRAY;
	}
}

void FloorStat::DetectDoorStat (cv::Mat frame) {
	Rect roi(anchor + trans_door_box, size_door_box);
	cv::Mat subframe = frame(roi).clone();
	/* Naive point approach */
	int	nsstat;
	int	color[2];
	color[0] = QuickColorRcg(subframe.at<Vec3b>(Point(2, 4)));
	color[1] = QuickColorRcg(subframe.at<Vec3b>(Point(7, 4)));
	if (color[0] == COLOR_WHITE && color[1] == COLOR_WHITE) {
		nsstat = DOOR_SSTAT_OPEN;
	} else if (color[0] == COLOR_BLUE && color[1] == COLOR_WHITE) {
		nsstat = DOOR_SSTAT_OPEN_H;
	} else if (color[0] == COLOR_BLUE && color[1] == COLOR_BLUE) {
		nsstat = DOOR_SSTAT_CLOSED;
	} else {
		nsstat = DOOR_SSTAT_NOT_HERE;
	}
	door_sstat = nsstat;

#if 0
	if (nsstat != door_sstat ) {
		cout << subframe.at<Vec3b>(Point(2, 4)) << "," << subframe.at<Vec3b>(Point(7, 4)) << "," << color[0] << "," << color[1] << endl;
		cout << sumd << "," << door_sstat << "," << nsstat << endl;
		imshow("door stat", subframe);
		waitKey(0);
	}
#endif
}

