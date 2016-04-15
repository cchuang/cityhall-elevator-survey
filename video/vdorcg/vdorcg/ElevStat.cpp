
#include "ElevStat.h"
#include <stdio.h>
#include <iomanip>
#include <string>
#ifdef __GNUC__
#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#define REDUCE_AVG	CV_REDUCE_AVG
#define REDUCE_SUM	CV_REDUCE_SUM
#else
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#endif
#include <tesseract/baseapi.h>
#include <tesseract/genericvector.h>


using namespace std;
using namespace cv;

static tesseract::TessBaseAPI *g_ocr;

#define COLOR_WHITE 3 
#define COLOR_RED   2
#define COLOR_GREEN 1
#define COLOR_BLUE  0
#define COLOR_GRAY  -1
#define COLOR_BIAS_FACTOR  0.04
static int CompareDouble (const void * a, const void * b)
{
  return (int) ( *(double*)b - *(double*)a );
}

static int	DtctSgnfctColor(cv::Mat frame, bool debug) {
	Vec3d p;
	Mat	ycbcrframe;
	Mat sum_row = Mat::zeros(1, frame.cols, CV_64FC3);
	Mat sum = Mat::zeros(1, 1, CV_64FC3);

	cvtColor(frame, ycbcrframe, COLOR_BGR2YCrCb);
	cv::reduce(ycbcrframe, sum_row, 0, REDUCE_AVG, CV_64FC3);
	cv::reduce(sum_row, sum, 1, REDUCE_AVG, CV_64FC3);
	p = sum.at<Vec3d>(0, 0);
	if (debug) {
		cout << p << endl;
	}

	p[1] = p[1]/256 - 0.5;
	p[2] = p[2]/256 - 0.5;

	if (p[1] > COLOR_BIAS_FACTOR) {
		return COLOR_RED;
	} else if (p[2] > COLOR_BIAS_FACTOR) {
		return COLOR_BLUE;
	} else if (p[1] < -COLOR_BIAS_FACTOR && p[2] < -COLOR_BIAS_FACTOR) {
		return COLOR_GREEN;
	} else {
		return COLOR_GRAY;
	}
}

ElevStat::ElevStat(int x, int y, int num_floors, int highest) {
	ElevStat(cv::Point(x, y), num_floors, highest);
}


ElevStat::ElevStat(cv::Point ac, int num_floors, int highest) : anchor(ac) {
	SetNumFloors(num_floors, highest);
	if (g_ocr == NULL) {
		GenericVector<STRING>	*vars_vec = new GenericVector<STRING>();
		GenericVector<STRING>	*vars_values = new GenericVector<STRING>();
		vars_vec->push_back("tessedit_char_whitelist");
		vars_values->push_back("0123456789BCNS-");
		g_ocr = new tesseract::TessBaseAPI();
		g_ocr->Init(NULL, "eng", tesseract::OEM_DEFAULT, NULL, NULL, vars_vec, vars_values, false);
		delete vars_vec;
		delete vars_values;
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

int ElevStat::SetNumFloors(int n, int highest) {
	int	floor = highest;	
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

int	ElevStat::RecogStat(cv::Mat frame, double ts_out){
	ts = ts_out;
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
		wh_floor = NA_CODE;
		weight_percent = NA_CODE;
		up_down = NA_CODE;
		req_open = false;
		stop_service = false;
	}

	for (int i=0; i < (int)floors_stat.size(); i ++) {
		floors_stat.at(i)->RecogStat(frame);
	}

	if (type == TYPE_GENERAL_CAR) {
		int	wh_fl_alt = RecogElevFloorByFloor();
		if (wh_fl_alt != ERR_CODE) {
			if (wh_floor == ERR_CODE) {
				wh_floor = wh_fl_alt;
			} else if (abs(wh_floor - wh_fl_alt) > 1) {
				cerr << name << " has 2 different location: " << wh_floor << " and " << wh_fl_alt << endl;
				wh_floor = wh_fl_alt;
			} else if (wh_floor != wh_fl_alt) {
				//cerr << name << " has 2 different location: " << wh_floor << " and " << wh_fl_alt << endl;
				//wh_floor = wh_fl_alt;
			}
		}
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

int	ElevStat::WriteRow(std::ostream &outfile, std::string event, std::string param1) {
	return WriteRow(outfile, event, param1, std::string(""));
}

int	ElevStat::WriteRow(std::ostream &outfile, std::string event) {
	return WriteRow(outfile, event, std::string(""), std::string(""));
}

int	ElevStat::WriteRow(std::ostream &outfile, std::string event, int param1) {
	std::string out_par = (param1 == ERR_CODE) ? std::string("ERROR") : std::to_string(param1);
	return WriteRow(outfile, event, out_par);
}

int	ElevStat::WriteRow(std::ostream &outfile, std::string event, int param1, bool param2) {
	std::string out_par = (param1 == ERR_CODE) ? std::string("ERROR") : std::to_string(param1);
	return WriteRow(outfile, event, out_par, std::to_string(param2));
}

int	ElevStat::WriteRow(std::ostream &outfile, std::string event, bool param1) {
	return WriteRow(outfile, event, std::to_string(param1));
}

int	ElevStat::WriteRow(std::ostream &outfile, std::string event, std::string param1, std::string param2) {
	std::string floor(std::to_string(wh_floor));
	std::string dir(std::to_string(up_down));
	if (wh_floor == NA_CODE || wh_floor == ERR_CODE) {
		floor = std::string("NA");
	} 
	if (up_down == NA_CODE) {
		dir = std::string("NA");
	}
	outfile << std::fixed;
	outfile << name << "," << std::setprecision(2) << ts << "," << floor << "," << dir << "," << event << "," << param1 << "," << param2 << endl; 
	return 0;
}

int	ElevStat::WriteRow(std::ostream &outfile, double ts, std::string event) {
	std::string floor(std::to_string(wh_floor));
	std::string dir(std::to_string(up_down));
	if (wh_floor == NA_CODE) {
		floor = std::string("NA");
	} 
	if (up_down == NA_CODE) {
		dir = std::string("NA");
	}
	outfile << std::fixed;
	outfile << name << "," << std::setprecision(2) << ts << "," << floor << "," << dir << "," << event << ",," << endl; 
	return 0;
}

bool ElevStat::CompReqStop(ElevStat *other) {
	if (type != TYPE_GENERAL_CAR) {
		return false;
	} 

	bool result = false;
	for (int i=0; i < (int) floors_stat.size(); i ++) {
		FloorStat *me, *he;
		me = GetFS(i);
		he = other->GetFS(i);
		result |= (me->req_stop != he->req_stop);
	}

	return result;
}

std::string ElevStat::WriteReqStop(void) {
	if (type != TYPE_GENERAL_CAR) {
		return std::string("");
	} 

	bool	any_stop = false;
	std::string result ("\"");
	for (int i=0; i < (int) floors_stat.size(); i ++) {
		FloorStat *me;
		me = GetFS(i);
		if (me->req_stop) {
			any_stop = true;
			result.append(std::to_string(me->floor));
			result.append(":");
		}
	}
	if (any_stop) {
		result.pop_back();
	}
	result.push_back('\"');
	return result;
}

int	ElevStat::Show(ElevStat *other, std::ostream &outfile) {
	int	result = 0;
	if (type == TYPE_GENERAL_CAR) {
		if (weight_percent != other->weight_percent) {
			result ++;
			WriteRow(outfile, "WEIGHT", weight_percent);
		}
		if (req_open != other->req_open) {
			result ++;
			WriteRow(outfile, "REQ_OPEN", req_open);
		}
		if (stop_service != other->stop_service) {
			result ++;
			WriteRow(outfile, "STOP_SRV", stop_service);
		}
	}

	if (CompReqStop(other)) {
		result ++;
		WriteRow(outfile, "REQ_STOP", WriteReqStop());
	}

	for (int i=0; i < (int) floors_stat.size(); i ++) {
		FloorStat *me, *he;
		me = GetFS(i);
		he = other->GetFS(i);
		if ((type == TYPE_GENERAL_CAR) || (type == TYPE_CAR_GROUP)) {
			if (me->req_up != he->req_up) {
				result ++;
				WriteRow(outfile, "REQ_UP", me->floor, me->req_up);
			}
			if (me->req_down != he->req_down) {
				result ++;
				WriteRow(outfile, "REQ_DOWN", me->floor, me->req_down);
			}
		}

		if (type == TYPE_GENERAL_CAR) {
			if (me->door_sstat != he->door_sstat) {
				result ++;
				if (he->door_sstat == DOOR_SSTAT_CLOSED && me->door_sstat == DOOR_SSTAT_OPEN_H) {
					WriteRow(outfile, "OPENING");
				} else if (he->door_sstat == DOOR_SSTAT_OPEN_H && me->door_sstat == DOOR_SSTAT_OPEN) {
					WriteRow(outfile, "OPENED");
				} else if (he->door_sstat == DOOR_SSTAT_OPEN && me->door_sstat == DOOR_SSTAT_OPEN_H) {
					WriteRow(outfile, "CLOSING");
				} else if (he->door_sstat == DOOR_SSTAT_OPEN_H && me->door_sstat == DOOR_SSTAT_CLOSED) {
					WriteRow(outfile, "CLOSED");
				} else if (he->door_sstat == DOOR_SSTAT_NOT_HERE && me->door_sstat == DOOR_SSTAT_CLOSED) {
					WriteRow(outfile, "ARRIVING");
				} else if (he->door_sstat == DOOR_SSTAT_CLOSED && me->door_sstat == DOOR_SSTAT_NOT_HERE) {
					// ignore LEAVING
				} else if (he->door_sstat == DOOR_SSTAT_NOT_HERE && me->door_sstat == DOOR_SSTAT_OPEN_H) {
					WriteRow(outfile, (ts + other->ts)/2, "ARRIVING");
					WriteRow(outfile, "OPENING");
				} else if (he->door_sstat == DOOR_SSTAT_CLOSED && me->door_sstat == DOOR_SSTAT_OPEN) {
					WriteRow(outfile, (ts + other->ts)/2, "OPENING");
					WriteRow(outfile, "OPENED");
				} else if (he->door_sstat == DOOR_SSTAT_OPEN_H && me->door_sstat == DOOR_SSTAT_NOT_HERE) {
					WriteRow(outfile, (ts + other->ts)/2, "CLOSED");
					// ignore LEAVING
				} else if (he->door_sstat == DOOR_SSTAT_OPEN && me->door_sstat == DOOR_SSTAT_CLOSED) {
					WriteRow(outfile, (ts + other->ts)/2, "CLOSING");
					WriteRow(outfile, "CLOSED");
				} else {
					WriteRow(outfile, "UNKNOWN_MOVING", std::to_string(he->door_sstat), std::to_string(me->door_sstat));
				}
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
	int	floor = ERR_CODE;
	Rect roi(anchor + trans_floor_text_box, size_floor_text_box);
	const char* ocr_out = RecogRectText(frame, roi, 4, false);
	// ocr_out: formatted as <char>0A0A. e.g. B2 is 0x42320A0A
	
	if (strlen(ocr_out) > 0) {
		if (ocr_out[0] == 'B' && strlen(ocr_out) > 1 && ocr_out[1] <= '9' && ocr_out[1] >= '0') {
			floor = -std::stoi(ocr_out + 1);
		} else if (ocr_out[0] <= '9' && ocr_out[0] >= '0') {
			floor = std::stoi(ocr_out);
		}
	} 
	//cout << name << ":" <<floor << endl;
	delete ocr_out;

	return floor;
}

int	ElevStat::RecogElevFloorByFloor(void) {
	int	present_position = ERR_CODE;
	bool	has_found = false;
	for (int i=0; i < (int)floors_stat.size(); i ++) {
		if(floors_stat.at(i)->car_is_here) {
			if (!has_found) {
				present_position = floors_stat.at(i)->floor;
				has_found = true;
			} else {
				cerr << "One elevator at different floors at the same time" << endl;
			}
		}
	}

	return	present_position;
}

int	ElevStat::RecogWeight(cv::Mat frame) {
	int	weight = ERR_CODE;
	Rect roi(anchor + trans_weight_box, size_weight_box);


	const char* ocr_out = RecogRectText(frame, roi, 3, false);
	//cout << name << ":len " << strlen(ocr_out) << ":" << ocr_out << endl;
	if (strlen(ocr_out) > 0) {
		if (ocr_out[0] <= '9' && ocr_out[0] >= '0') {
			weight = std::stoi(ocr_out);
		} 
	} 
#if 0
	Mat subframe = frame(Rect(anchor + trans_name_text_box, size_name_text_box)).clone();
	cout << name << endl;
	imshow("Weight", subframe);
	waitKey(0);
#endif
	delete ocr_out;

	return weight;
}

int ElevStat::VerifyName(cv::Mat frame) {
	Rect roi(anchor + trans_name_text_box, size_name_text_box);

	const char* ocr_out = RecogRectText(frame, roi, 3, false);
	std::string ocr_result(ocr_out);
	delete ocr_out;

#if 0
	Mat subframe = frame(roi).clone();
	cout << name << ": " << ocr_result << endl;
	imshow("NAME", subframe);
	waitKey(0);
#endif
	// nasty approach. temporally method. 
	if (ocr_result.size() >= 3) {
		if (ocr_result.compare(1, 1, "0") == 0) {
			ocr_result[1] = 'C';
		}
		if (ocr_result.compare(2, 1, "S") == 0) {
			ocr_result[2] = '5';
		}
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
	} else if (indicator < -100) {
		up_down = GOING_DOWN;
	} else {
		up_down = GOING_STOP;
	}

#if 0
	if (up_down == GOING_STOP && ts == 1451444915) {
	cout << row_diff << endl;
	cout << "Result: " << up_down << endl;

	imshow("detect", subframe);
	waitKey(0);
	}
#endif
}

void ElevStat::DetectDoorOpen(cv::Mat frame) {
	Rect roi(anchor + trans_open_box, size_open_box);
	Mat subframe = frame(roi).clone();
	if (DtctSgnfctColor(subframe, false) == COLOR_GREEN) {
		req_open = true;
	} else {
		req_open = false;
	}

#if 0
	if (req_open) {
		cout << "REQ OPEN: " << req_open << endl;
		imshow("Detect Service", subframe);
		waitKey(0);
	}
#endif
}

void ElevStat::DetectService(cv::Mat frame) {
	Rect roi(anchor + trans_service_box, size_service_box);
	Mat subframe = frame(roi).clone();

	if (DtctSgnfctColor(subframe, false) == COLOR_RED) {
		stop_service = true;
	} else {
		stop_service = false;
	}

#if 0
	cout << "Stop service: " << stop_service << endl;
	imshow("Detect Service", subframe);
	waitKey(0);
#endif
}

