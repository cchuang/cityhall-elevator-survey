#include "ElevSetStat.h"
#include "ElevStat.h"
#include <vector>
#include <opencv2/core.hpp>

#if 0
void InitElevStat(vector<ElevStat*>&	elev_stat_s) {
	elev_stat_s.push_back(new ElevStat(76, 97, 14));
	elev_stat_s.back()->name = "NC1";
	elev_stat_s.push_back(new ElevStat(199, 100, 14));
	elev_stat_s.back()->name = "NC2";
	elev_stat_s.push_back(new ElevStat(323, 101, 14));
	elev_stat_s.back()->name = "NC3";
	elev_stat_s.push_back(new ElevStat(520, 99, 14));
	elev_stat_s.back()->name = "NC4";
	elev_stat_s.push_back(new ElevStat(645, 99, 14));
	elev_stat_s.back()->name = "NC5";
	elev_stat_s.push_back(new ElevStat(844, 101, 14));
	elev_stat_s.back()->name = "NC6";
}

void feedElevStat(vector<ElevStat*>&	elev_stat_s, Mat frame, double msec) {
	for (int i = 0; i < elev_stat_s.size(); i ++) {
		elev_stat_s.at(i)->RecogStat(frame, msec);
		elev_stat_s.at(i)->Show();
	}
}

void freeElevStat(vector<ElevStat*>&	elev_stat_s) {
	for (int i = 0; i < elev_stat_s.size(); i ++) {
		delete elev_stat_s.at(i);
	}
}
#endif


int ElevSetStat::SetElev(Point anchor, int num_floors, string name) {
	elevs_stat.push_back(new ElevStat(anchor, num_floors));
	elevs_stat.back()->name = name;
	return 0;
}

int ElevSetStat::RecogStat(Mat frame, double msec) {
	for (int i = 0; i < (int) elevs_stat.size(); i ++) {
		elevs_stat.at(i)->RecogStat(frame, msec);
		elevs_stat.at(i)->Show();
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

