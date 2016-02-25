#include "ElevSetStat.h"
#include "ElevStat.h"
#include <vector>
#include <opencv2/core.hpp>

using namespace std;

int ElevSetStat::SetElev(cv::Point anchor, int num_floors, int highest, std::string name) {
	elevs_stat.push_back(new ElevStat(anchor, num_floors, highest));
	elevs_stat.back()->name = name;
	elevs_stat.back()->SetType(TYPE_GENERAL_CAR);
	return 0;
}

int	ElevSetStat::SetElevGrp(cv::Point anchor, int num_floors, int highest, std::string name) {
	elevgs_stat.push_back(new ElevStat(anchor, num_floors, highest));
	elevgs_stat.back()->name = name;
	elevgs_stat.back()->SetType(TYPE_CAR_GROUP);
	return 0;
}

int ElevSetStat::Show(ElevSetStat *other, std::ostream &outfile) {
	int	result = 0;
	for (int i = 0; i < (int) elevs_stat.size(); i ++) {
		result += GetES(i)->Show(other->GetES(i), outfile);
	}
	for (int i = 0; i < (int) elevgs_stat.size(); i ++) {
		result += GetEGS(i)->Show(other->GetEGS(i), outfile);
	}
	return result;
}

ElevStat *ElevSetStat::GetES(int idx) {
	return elevs_stat.at(idx);
}

ElevStat *ElevSetStat::GetEGS(int idx) {
	return elevgs_stat.at(idx);
}

int ElevSetStat::RecogStat(cv::Mat frame, time_t ts) {
	int	result;
	for (int i = 0; i < (int) elevs_stat.size(); i ++) {
		result = elevs_stat.at(i)->RecogStat(frame, ts);
		if (result != 0) {
			return result;
		}
		//elevs_stat.at(i)->Show();
	}
	for (int i = 0; i < (int) elevgs_stat.size(); i ++) {
		result = elevgs_stat.at(i)->RecogStat(frame, ts);
		if (result != 0) {
			return result;
		}
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

