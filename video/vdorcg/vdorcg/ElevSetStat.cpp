#include "ElevSetStat.h"
#include "ElevStat.h"
#include <vector>
#include <opencv2/core.hpp>

int ElevSetStat::SetElev(Point anchor, int num_floors, string name) {
	elevs_stat.push_back(new ElevStat(anchor, num_floors));
	elevs_stat.back()->name = name;
	elevgs_stat.back()->type = 1;
	return 0;
}

int	ElevSetStat::SetElevGrp(Point anchor, int num_floors, string name) {
	elevgs_stat.push_back(new ElevStat(anchor, num_floors));
	elevgs_stat.back()->name = name;
	elevgs_stat.back()->type = 2;
	return 0;
}

int ElevSetStat::RecogStat(Mat frame, double msec) {
	for (int i = 0; i < (int) elevs_stat.size(); i ++) {
		elevs_stat.at(i)->RecogStat(frame, msec);
		elevs_stat.at(i)->Show();
	}
	for (int i = 0; i < (int) elevgs_stat.size(); i ++) {
		elevgs_stat.at(i)->RecogStat(frame, msec);
		elevgs_stat.at(i)->Show();
	}
	return 0;
}

ElevSetStat::ElevSetStat() {
}

ElevSetStat::~ElevSetStat() {
	for (int i = 0; i < (int) elevs_stat.size(); i ++) {
		delete elevs_stat.at(i);
	}
}

