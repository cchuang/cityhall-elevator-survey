#include "ElevStat.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

FloorStat::FloorStat(int x, int y) {
	SetAnchor(x, y);
}

int	FloorStat::SetAnchor(int x, int y) {
	anchor = Point(x, y);
	return 0;
}

int	FloorStat::RecogStat(Mat	frame) {
	// It's CV_8UC3 
	Vec3b p;
	req_up = req_down = req_stop = door_is_opening = car_is_here = false;
	if (type == TYPE_GENEARL_CAR) {
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

ElevStat::ElevStat(int x, int y, int num_floors) {
	ElevStat(Point(x, y), num_floors);
}


ElevStat::ElevStat(Point ac, int num_floors) {
	SetAnchor(ac.x, ac.y);
	SetNumFloors(num_floors);
}

ElevStat::~ElevStat() {
	for (int i=0; i < (int) floors_stat.size(); i ++) {
		delete floors_stat.at(i);
	}
}

int ElevStat::SetAnchor(int x, int y) {
	anchor = Point(x, y);
	return 0;
}

int ElevStat::SetNumFloors(int n) {
	int	floor = FLOOR_START_AT;	
	for (int i=0; i < n; i ++) {
		floors_stat.push_back(
				new FloorStat(anchor.x + trans_floor_box.x, 
					anchor.y + trans_floor_box.y + (int)(FLOOR_HEIGHT * (float)i)));
		floors_stat.back()->floor = floor;
		floors_stat.back()->type = type;

		floor --;
		// we don't have floor zero (or ground floor). 
		if (floor == 0) {
			floor --;
		}
	}

#if 0
	for (int i = 0; i < n; i ++) {
		cout << floors_stat.at(i)->anchor.x << ", " << floors_stat.at(i)->anchor.y << endl;
	}
#endif
	return 0;
}

int	ElevStat::RecogStat(Mat frame, double dmsec){
	msec = dmsec;
	wh_floor = RecogElevFloor(frame);
	for (int i=0; i < (int)floors_stat.size(); i ++) {
		floors_stat.at(i)->RecogStat(frame);
	}
	return 0;
}

int ElevStat::SetType(int in_type) {
	type = in_type;
	for (int i=0; i < (int)floors_stat.size(); i ++) {
		floors_stat.at(i)->type = in_type;
	}
	return 0;
}

int	ElevStat::Show(){
	for (int i=0; i < (int) floors_stat.size(); i ++) {
		if ((type == TYPE_GENEARL_CAR) || (type == TYPE_CAR_GROUP)) {
			cout << name << "," << msec << "," << floors_stat.at(i)->floor << ",REQUP," << floors_stat.at(i)->req_up << endl;
			cout << name << "," << msec << "," << floors_stat.at(i)->floor << ",REQDOWN," << floors_stat.at(i)->req_down << endl;
		}

		if (type == TYPE_GENEARL_CAR) {
			cout << name << "," << msec << "," << floors_stat.at(i)->floor << ",REQSTOP," << floors_stat.at(i)->req_stop << endl;
			cout << name << "," << msec << "," << floors_stat.at(i)->floor << ",DOORISOPEN," << floors_stat.at(i)->door_is_opening << endl;
			cout << name << "," << msec << "," << floors_stat.at(i)->floor << ",CARISHERE," << floors_stat.at(i)->car_is_here << endl;
		}
	}
	return 0;
}

int	ElevStat::RecogElevFloor(Mat frame) {
	// By Teeseract console app. nasty implementation
	// TODO: use library directly
	//FILE *ocr;

	Point floor_box = anchor + trans_floor_text;
	Mat fl_txt = frame(Range(floor_box.y, floor_box.y + 23), Range(floor_box.x, floor_box.x + 24)).clone();
	String	tmp_name ("tmp" + name + ".bmp"); 
	//char* result = (char *)malloc(32);
	vector<uchar> buf;

	//cvtColor(fl_txt, fl_txt, COLOR_BGRA2GRAY);
	//cout << fl_txt.channels() << " " << fl_txt.depth() << endl;
	//imwrite(tmp_name, fl_txt);
	//imshow("Floor text", fl_txt);
	//waitKey(10);

	/*
	if ((ocr = _popen("C:\\Tesseract-OCR\\tesseract.exe D:\\noname.jpg stdout", "rb")) == NULL) {
		return -1;
	}
	*/
	//fread(result, 1, 32, ocr);
	//printf("%s", result);
	//_pclose(ocr);
	return 0;
}

