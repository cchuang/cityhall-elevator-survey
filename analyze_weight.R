#!/bin/
# author: cchuang
# description: analyze the efficiency of cityhall's elevators
# setwd("D:/cityhall-elevator-survey");source("analyze_weight.R")
#library(doParallel)
library(plyr)
library(foreach)
library(xts)
library(ggplot2)
library(MASS)
library(magrittr)
library(reshape2)
source("analysis_func.R")

#cl <- makeCluster(detectCores())
#registerDoParallel(cl)
out.list <- read.csv("out_list.txt", header=FALSE, stringsAsFactors=FALSE)
l <- lapply(out.list[[1]], read.csv, header = FALSE, stringsAsFactors=FALSE)
orig.data <- do.call("rbind", l)

#orig.data <- read.csv("data/out_2015-12-30.csv", header = FALSE, stringsAsFactors=FALSE)
names(orig.data) <- c("Car", "Time", "Fl", "Event", "Val")
orig.data$Time <- as.POSIXct(orig.data$Time - 8 *60 *60, origin="1970-01-01")

# TODO: STOPSERVICE is buggy
# TODO: REQOPEN output lousy data
#cpt.data <- subset(orig.data, Time < "2015-12-30 13:00:00" & Event != "LOCDIR" & Event != "WEIGHT" & Event != "STOPSERVICE" & Event != "REQOPEN")

# NC6 usually get over 100% weight because of parsing error by OCR, which is a bug and should be fixed in the future. 
orig.data2 <- within(orig.data, {
	while (any(Event=="WEIGHT" & Val > 100 & Car == "NC6")) {
		Val[Event=="WEIGHT" & Val > 100 & Car == "NC6"] <- Val[Event=="WEIGHT" & Val > 100 & Car == "NC6"]/10
	}
})

p <- ggplot(subset(orig.data2, Event == "WEIGHT" & Val != -99 & Time < "2015-12-30 13:00:00"), aes(Time, Val, color=Car)) 
print(p + geom_line() + labs(y="Weight Percentage"))

weight <- subset(orig.data2, Event=="WEIGHT" & Val != -99) 
ww <- data.frame()
ddply(weight, .(Car), CountNoPpLv, out.data=ww)

p <- ggplot(subset(ww, Car!="NC1"), aes(time.interval, no.p.lv, color=Car)) 
print(p + geom_line() + labs(y="# of People Left"))

#stop("stopped manually")

