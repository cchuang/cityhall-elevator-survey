/**
 * Elevator Video Recognizer
 * Maintainer: cc.huang@iis.sinica.edu.tw
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
//#include <opencv/cv.h>
//#include <opencv/highgui.h>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "ElevStat.h"
#include "ElevSetStat.h"

using namespace cv;	
using namespace std;

struct FileList {
	ifstream list;
	time_t	timestamp;
};

static int OpenNewCap(VideoCapture &cap, FileList &in_list) {
	string	in_file_name;
	in_list.list >> in_file_name;
	cap.open(in_file_name);
	if (!cap.isOpened()) {
		cerr << "Cannot open the video file " << in_file_name << endl;
		return -1;
	}
	in_list.timestamp = std::stoi(in_file_name.substr(in_file_name.size()-23, 10));
	in_list.timestamp += std::stoi(in_file_name.substr(in_file_name.size()-12, 8));
	//adjust timezone manually
	in_list.timestamp += 8 * 60 * 60;
	cerr << in_file_name <<":"<<in_list.timestamp << endl << std::asctime(std::gmtime(&in_list.timestamp)) << endl;
	return 0;
}

int ReadOneFrameByN(int n, VideoCapture &cap, Mat& frame, FileList &in_list) {
	for (int i = 0; i < n; i ++) {
		if (cap.isOpened()) {
			cap >> frame; 
		} else {
			if (OpenNewCap(cap, in_list) < 0)	return -1;
			cap >> frame;
		}
		if (frame.empty()) {
			if (OpenNewCap(cap, in_list) < 0)	return -1;
			cap >> frame;
		}
	}
	return 0;
}

#define SIZE_ELEVS_STAT 2
void InitElevStat(ElevSetStat *elevs_stat) {
	for (int i = 0; i < SIZE_ELEVS_STAT; i ++) {
		elevs_stat[i].SetElev(Point(76, 97), 14, "NC1");
		elevs_stat[i].SetElev(Point(199, 101), 14, "NC2");
		elevs_stat[i].SetElev(Point(323, 101), 14, "NC3");
		elevs_stat[i].SetElev(Point(520, 100), 14, "NC4");
		elevs_stat[i].SetElev(Point(645, 99), 14, "NC5");
		elevs_stat[i].SetElev(Point(844, 101), 14, "NC6");
		elevs_stat[i].SetElevGrp(Point(4, 100), 14, "NC1-3");
		elevs_stat[i].SetElevGrp(Point(448, 100), 14, "NC4-5");
		elevs_stat[i].SetElevGrp(Point(771, 103), 14, "NC6~");
	}
}

bool IsPanX1(Mat frame) {
	// very noive approach. check the color of X1 button. when it turns white, it's not on X1. 
	Vec3b p, pbar;
	p = frame.at<Vec3b>(Point(155, 703)); // [192, 192, 192]
	//pbar = frame.at<Vec3b>(Point(256, 703)); //[239, 239, 239] 
	//cout << p << " " << pbar << endl;
	return (p[0] < 215);
}

int DetectEvents(struct FileList &in_list) {
	int	num_frames = 0;
	int	result;
	Mat prev_frame, curr_frame;
	ElevSetStat	*elevs_stat = new ElevSetStat [SIZE_ELEVS_STAT];
	ElevSetStat	*curr_es, *prev_es, *tmp_es;
	VideoCapture	cap;

	InitElevStat(elevs_stat);
	curr_es = elevs_stat;
	prev_es = elevs_stat + 1;

	//TODO: As long as the original file is split into small pieces (each one
	//is in 2 hours), we are facing a problem that it's possible we got an event
	//at the boundary in time of two files.  Moreover, it's also possilbe that 
	//we will have duplicated events at the boundaries. 
	//So, check it out before you analyze the data. 
	
	result = ReadOneFrameByN(6, cap, curr_frame, in_list); // down sampling
	while(result >= 0) {
		Mat diff;
		int num_nonzeros = 0; 
		bool is_event = false;

		if ((num_frames != 0) && IsPanX1(curr_frame)) {
#if 0
			imshow("Current frame", curr_frame);
			waitKey(10);
#endif

			absdiff(prev_frame, curr_frame, diff);
			cvtColor(diff, diff, COLOR_BGRA2GRAY);

			//Scalar s = sum(diff);
			//cout << "Sum of the frame difference is " << s << endl;
			num_nonzeros = countNonZero(diff);
			double avg_px_ch = sum(diff)[0]/num_nonzeros;
			/*
			cout << cap.get(CAP_PROP_POS_MSEC) << ": " 
				<< num_frames << ": # of none zeros: " << num_nonzeros << 
				" avg px change " << avg_px_ch <<endl;
				*/

			// in the most noisy frames, we got the ratio around 2. 
			// in the REAL event frames, it's about 20s or more. 
			// that's why we choose 10 as the threshold. 
			is_event = ((num_nonzeros != 0) && (avg_px_ch > 10));

			if (is_event) {
				int	result;
				result = curr_es->RecogStat(curr_frame, 
						in_list.timestamp + cap.get(CAP_PROP_POS_MSEC)/1000);
				if (result == 0) {
					//curr_es->Show();
					int	num_lines = curr_es->ShowDiff(prev_es);
#if 0
					if (num_lines > 0) {
						imshow("Frame", curr_frame);
						waitKey(0);
					}
#endif
					// swap curr_es and prev_es
					tmp_es = curr_es;
					curr_es = prev_es;
					prev_es = tmp_es;
				}
#if 0
				imshow("Difference", diff);
				waitKey(0);
#endif
			}
		} else if (num_frames == 0) {
			// Assumption: the first frame must be on the panel we are interested in. 
			prev_es->RecogStat(curr_frame, cap.get(CAP_PROP_POS_MSEC));
		}

		if (IsPanX1(curr_frame)) {
			prev_frame = curr_frame.clone();
		}
		result = ReadOneFrameByN(6, cap, curr_frame, in_list);
		num_frames ++;
#if 0
		if (num_frames > 12) {
			break;
		}
#endif
	}

	delete [] elevs_stat;
	return 0;
}

int main(int argc, const char** argv) {
	struct FileList in_file_list;

	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " <target file list>" << endl;
		cerr << "\tOpenCV " << CV_VERSION << endl;
		cerr << "\tTesseract-OCR " << TESSERACT_VERSION_STR << endl;
		return -1;
	}

	in_file_list.list.open(argv[1]);
	//ifstream in_file_list(argv[1]);
	if (!in_file_list.list.is_open()) {
		cerr << "File list (" << argv[1] << ") is not present." << endl;
		return -1;
	}
#if 0
	string in_file;

	in_file_list >> in_file;
	
	cout << "Input file list: " << in_file << endl;

	VideoCapture cap(in_file);
	if (!cap.isOpened()) {
		cout << "Cannot open the video file " << endl;
		return -1;
	}
#endif

	return DetectEvents(in_file_list);
}

