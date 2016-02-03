
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
		vars_values->push_back("0123456789BCNS-");
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
	int	verify_result;
	if (type == TYPE_GENERAL_CAR) {
		verify_result = VerifyName(frame);
		if (verify_result != 0) {
			return verify_result;
		}
		wh_floor = RecogElevFloor(frame);
		weight_percent = RecogWeight(frame);
		DetectDirection(frame);
		DetectDoorOpen(frame);
		DetectService(frame);
		//cout<<name << ":" << wh_floor << ":" << weight_percent << endl;
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
	if (type == TYPE_GENERAL_CAR) {
		cout << name << "," << msec << "," << wh_floor << ",LOCDIR," << up_down << endl;
		cout << name << "," << msec << "," << wh_floor << ",WEIGHT," << weight_percent << endl;
		cout << name << "," << msec << "," << wh_floor << ",REQOPEN," << req_open << endl;
		cout << name << "," << msec << "," << wh_floor << ",STOPSERVICE," << stop_service << endl;
	}
	for (int i=0; i < (int) floors_stat.size(); i ++) {
		if ((type == TYPE_GENERAL_CAR) || (type == TYPE_CAR_GROUP)) {
			cout << name << "," << msec << "," << floors_stat.at(i)->floor << ",REQUP," << floors_stat.at(i)->req_up << endl;
			cout << name << "," << msec << "," << floors_stat.at(i)->floor << ",REQDOWN," << floors_stat.at(i)->req_down << endl;
		}

		if (type == TYPE_GENERAL_CAR) {
			cout << name << "," << msec << "," << floors_stat.at(i)->floor << ",REQSTOP," << floors_stat.at(i)->req_stop << endl;
			cout << name << "," << msec << "," << floors_stat.at(i)->floor << ",DOORISOPEN," << floors_stat.at(i)->door_is_opening << endl;
			cout << name << "," << msec << "," << floors_stat.at(i)->floor << ",CARISHERE," << floors_stat.at(i)->car_is_here << endl;
		}
	}
	return 0;
}

// TODO: it's a very nasty implementation...
int	ElevStat::ShowDiff(ElevStat *other) {
	int	result = 0;
	if (type == TYPE_GENERAL_CAR) {
		if ((wh_floor != other->wh_floor) || (up_down != other->up_down)) {
			result ++;
			cout << name << "," << msec << "," << wh_floor << ",LOCDIR," << up_down << endl;
		}
		if ((wh_floor != other->wh_floor) || (weight_percent != other->weight_percent)) {
			result ++;
			cout << name << "," << msec << "," << wh_floor << ",WEIGHT," << weight_percent << endl;
		}
		if ((wh_floor != other->wh_floor) || (req_open != other->req_open)) {
			result ++;
			cout << name << "," << msec << "," << wh_floor << ",REQOPEN," << req_open << endl;
		}
		if ((wh_floor != other->wh_floor) || (stop_service != other->stop_service)) {
			result ++;
			cout << name << "," << msec << "," << wh_floor << ",STOPSERVICE," << stop_service << endl;
		}
	}
	for (int i=0; i < (int) floors_stat.size(); i ++) {
		FloorStat *me, *he;
		me = GetFS(i);
		he = other->GetFS(i);
		if ((type == TYPE_GENERAL_CAR) || (type == TYPE_CAR_GROUP)) {
			if (me->req_up != he->req_up) {
				result ++;
				cout << name << "," << msec << "," << me->floor << ",REQUP," << me->req_up << endl;
			}
			if (me->req_down != he->req_down) {
				result ++;
				cout << name << "," << msec << "," << me->floor << ",REQDOWN," << me->req_down << endl;
			}
		}

		if (type == TYPE_GENERAL_CAR) {
			if (me->req_stop != he->req_stop) {
				result ++;
				cout << name << "," << msec << "," << me->floor << ",REQSTOP,"    << me->req_stop << endl;
			}
			if (me->door_is_opening != he->door_is_opening) {
				result ++;
				cout << name << "," << msec << "," << me->floor << ",DOORISOPEN," << me->door_is_opening << endl;
			}
			if (me->car_is_here != he->car_is_here) {
				result ++;
				cout << name << "," << msec << "," << me->floor << ",CARISHERE,"  << me->car_is_here << endl;
			}
		}
	}
	return result;
}

FloorStat *ElevStat::GetFS(int i) {
	return floors_stat.at(i);
}

char *ElevStat::RecogRectText(Mat frame, Rect roi, int ratio, bool debug) {
	cv::Mat subframe = frame(roi).clone();
	cv::Mat resize_frame, laplace, sharp;

	cvtColor(subframe, subframe, COLOR_BGRA2GRAY);
	cv::resize(subframe, resize_frame, Size(), ratio, ratio, cv::INTER_CUBIC);
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
	const char* ocr_out = RecogRectText(frame, roi, 4, true);
	// ocr_out: formatted as <char>0A0A. e.g. B2 is 0x42320A0A
	string ocr_string (ocr_out);
	std::replace(ocr_string.begin(), ocr_string.end(), '\n', ' ') ;
	//cout << name << ": real text " << ocr_string << endl;
	if (strlen(ocr_out) > 0) {
		if (ocr_out[0] == 'B') {
			floor = -std::stoi(ocr_out + 1);
		} else {
			floor = std::stoi(ocr_out);
		}
	} 
	//cout << name << ":" <<floor << endl;
	delete ocr_out;

	return floor;
}

int	ElevStat::RecogWeight(cv::Mat frame) {
	int	weight = -99;
	Rect roi(anchor + trans_weight_box, size_weight_box);

	const char* ocr_out = RecogRectText(frame, roi, 3, false);
	//cout << name << ":len " << strlen(ocr_out) << ":" << ocr_out << endl;
	if (strlen(ocr_out) > 0) {
		weight = std::stoi(ocr_out);
	} 
	delete ocr_out;

	return weight;
}

int ElevStat::VerifyName(cv::Mat frame) {
	Rect roi(anchor + trans_name_text_box, size_name_text_box);

	const char* ocr_out = RecogRectText(frame, roi, 3, true);
	std::string ocr_result(ocr_out);
	delete ocr_out;

	// nasty approach. temporally method. 
	if ((ocr_result.compare(0, 2, "N0") == 0) || 
			(ocr_result.compare(0, 2, "S0") == 0)) {
		ocr_result[1] = 'C';
	}	

	//cout << name << ":" << name.size() << ":" << ocr_result.compare(0, name.size(), name) << endl;
	return ocr_result.compare(0, name.size(), name);
}

void ElevStat::DetectDirection(cv::Mat frame) {
	Rect roi(anchor + trans_updown_box, size_updown_box);
	Mat subframe = frame(roi).clone();
	Mat row_sum = Mat::zeros(subframe.rows, 1, CV_64F);
	Mat row_sum_lag = Mat::zeros(subframe.rows, 1, CV_64F);
	Mat row_diff = Mat::zeros(subframe.rows, 1, CV_64F);

	cvtColor(subframe, subframe, COLOR_BGRA2GRAY);
	subframe = 255 - subframe;
	cv::reduce(subframe, row_sum, 1, REDUCE_SUM, CV_64F);
	// delay
	row_sum(Rect(0, 1, row_sum.cols, row_sum.rows - 1)).copyTo(row_sum_lag(Rect(0, 0, row_sum.cols, row_sum.rows - 1)));
	// take the derivative
	row_diff = row_sum_lag - row_sum;
	// find the median
	cv::sort(row_diff, row_diff, SORT_EVERY_COLUMN); 
	double indicator = row_diff.at<double>((row_diff.rows/2), 0);

	if (indicator > 100) {
		up_down = GOING_UP;
	} else if (indicator < 100) {
		up_down = GOING_DOWN;
	} else {
		up_down = GOING_STOP;
	}

#if 0
	cout << up_down << endl;

	imshow("detect", subframe);
	waitKey(0);
#endif
}

static int CompareDouble (const void * a, const void * b)
{
  return (int) ( *(double*)b - *(double*)a );
}

static int	DtctSgnfctColor(cv::Mat frame) {
	Vec3d p;
	double tmp[3] = {0, 0, 0};
	double gap[2];
	int	i;
	Mat sum_row = Mat::zeros(1, frame.cols, CV_64FC3);
	Mat sum = Mat::zeros(1, 1, CV_64FC3);

	cv::reduce(frame, sum_row, 0, REDUCE_SUM, CV_64FC3);
	cv::reduce(sum_row, sum, 1, REDUCE_SUM, CV_64FC3);
	p = sum.at<Vec3d>(0, 0);

	for (i = 0; i < 3; i ++) {
		tmp[i] = p[i];
	}
	qsort(tmp, 3, sizeof(double), CompareDouble);
	gap[0] = tmp[0] - tmp[1];
	gap[1] = tmp[1] - tmp[2];
	//cout << "\t" << p << "\t" << gap[0] << "\t" << gap[1] << endl;
	// has significant gap, 20 is selected empirically. 
	if (gap[0] > 20.0 * gap[1]) {
		for (i = 0; i < 3; i ++) {
			if (tmp[0] == p[i]) {
				return i;
			}
		}
	} 
	return -1;
}

void ElevStat::DetectDoorOpen(cv::Mat frame) {
	Rect roi(anchor + trans_open_box, size_open_box);
	Mat subframe = frame(roi).clone();
	if (DtctSgnfctColor(subframe) == 1) {
		req_open = true;
	} else {
		req_open = false;
	}
}

void ElevStat::DetectService(cv::Mat frame) {
	Rect roi(anchor + trans_service_box, size_service_box);
	Mat subframe = frame(roi).clone();

#if 0
	std::string	tmp_name ("tmp" + name + ".bmp"); 
	imwrite(tmp_name, subframe);
	cout << name << ":" << "Service detection" << endl;
#endif

	if (DtctSgnfctColor(subframe) == 2) {
		stop_service = true;
	} else {
		stop_service = false;
	}
}

