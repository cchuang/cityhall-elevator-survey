
#include "ElevStat.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <tesseract/baseapi.h>
#include <tesseract/genericvector.h>

using namespace std;
using namespace cv;

FloorStat::FloorStat(int x, int y) {
	SetAnchor(x, y);
}

int	FloorStat::SetAnchor(int x, int y) {
	anchor = cv::Point(x, y);
	return 0;
}

int	FloorStat::RecogStat(cv::Mat	frame) {
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
	ElevStat(cv::Point(x, y), num_floors);
}


ElevStat::ElevStat(cv::Point ac, int num_floors) {
	SetAnchor(ac.x, ac.y);
	SetNumFloors(num_floors);
	if (g_ocr == NULL) {
		GenericVector<STRING>	*vars_vec = new GenericVector<STRING>();
		GenericVector<STRING>	*vars_values = new GenericVector<STRING>();
		vars_vec->push_back("tessedit_char_whitelist");
		vars_values->push_back("0123456789B");
		g_ocr = new tesseract::TessBaseAPI();
		g_ocr->Init(NULL, "eng", tesseract::OEM_DEFAULT, NULL, NULL, vars_vec, vars_values, false);
		//std::cout << g_ocr->Version() << endl;
	} 
}

ElevStat::~ElevStat() {
	for (int i=0; i < (int) floors_stat.size(); i ++) {
		delete floors_stat.at(i);
	}
}

int ElevStat::SetAnchor(int x, int y) {
	anchor = cv::Point(x, y);
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

int	ElevStat::RecogStat(cv::Mat frame, double dmsec){
	msec = dmsec;
	if (type == TYPE_GENEARL_CAR) {
		wh_floor = RecogElevFloor(frame);
		weight_percent = RecogWeight(frame);
		cout<<name << ":" << wh_floor << ":" << weight_percent << endl;
	} else {
		wh_floor = 0;
		weight_percent = 0;
	}
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

char *ElevStat::RecogRect(Mat frame, Rect roi, bool debug) {
	cv::Mat subframe = frame(roi).clone();
	cv::Mat resize_frame, laplace, sharp;

	cvtColor(subframe, subframe, COLOR_BGRA2GRAY);
	cv::resize(subframe, resize_frame, Size(), 3, 3, cv::INTER_CUBIC);
	cv::Laplacian(resize_frame, laplace, CV_8UC1);
	sharp = resize_frame - laplace - laplace - laplace;
	//sharp = resize_frame;
	//laplace = sharp;
	//cv::medianBlur(laplace, sharp, 5);

	g_ocr->SetImage((uchar*)sharp.data, sharp.size().width, sharp.size().height, sharp.channels(), sharp.step1());
	if (debug) {
		std::string	tmp_name ("tmp" + name + ".bmp"); 
		imwrite(tmp_name, sharp);
		//imshow("Weight", sharp);
		//waitKey(0);
	}

	g_ocr->Recognize(0);
	char* ocr_out = g_ocr->GetUTF8Text();
	return ocr_out;
}

int	ElevStat::RecogElevFloor(cv::Mat frame) {
	int	floor = -99;
	Rect roi(anchor + trans_floor_text_box, size_floor_text_box);
	const char* ocr_out = RecogRect(frame, roi, false);
	// ocr_out: formatted as <char>0A0A. e.g. B2 is 0x42320A0A
	if (strlen(ocr_out) > 0) {
		if (ocr_out[0] == 'B') {
			floor = -std::stoi(ocr_out + 1);
		} else {
			floor = std::stoi(ocr_out);
		}
	} 
	//cout << floor << endl;
	delete ocr_out;

	return floor;
}

int	ElevStat::RecogWeight(cv::Mat frame) {
	int	weight = -99;
	Rect roi(anchor + trans_weight_box, size_weight_box);

	const char* ocr_out = RecogRect(frame, roi, false);
	//cout << name << ":len " << strlen(ocr_out) << ":" << ocr_out << endl;
	if (strlen(ocr_out) > 0) {
		weight = std::stoi(ocr_out);
	} 
	delete ocr_out;

	return weight;
}

