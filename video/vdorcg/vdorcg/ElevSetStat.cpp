#include "ElevSetStat.h"
#include "ElevStat.h"
#include <vector>
#include <opencv2/core.hpp>

using namespace std;

int ElevSetStat::SetElev(cv::Point anchor, int num_floors, std::string name) {
	elevs_stat.push_back(new ElevStat(anchor, num_floors));
	elevs_stat.back()->name = name;
	elevs_stat.back()->SetType(TYPE_GENEARL_CAR);
	return 0;
}

int	ElevSetStat::SetElevGrp(cv::Point anchor, int num_floors, std::string name) {
	elevgs_stat.push_back(new ElevStat(anchor, num_floors));
	elevgs_stat.back()->name = name;
	elevgs_stat.back()->SetType(TYPE_CAR_GROUP);
	return 0;
}

int ElevSetStat::RecogStat(cv::Mat frame, double msec) {
	for (int i = 0; i < (int) elevs_stat.size(); i ++) {
		elevs_stat.at(i)->RecogStat(frame, msec);
		//elevs_stat.at(i)->Show();
	}
	for (int i = 0; i < (int) elevgs_stat.size(); i ++) {
		elevgs_stat.at(i)->RecogStat(frame, msec);
		//elevgs_stat.at(i)->Show();
	}
	return 0;
}

ElevSetStat::ElevSetStat() {
}

ElevSetStat::~ElevSetStat() {
	for (int i = 0; i < (int) elevs_stat.size(); i ++) {
		delete elevs_stat.at(i);
	}
	for (int i = 0; i < (int) elevgs_stat.size(); i ++) {
		delete elevgs_stat.at(i);
	}
}

