#!/bin/
# author: cchuang
# description: analyze the efficiency of cityhall's elevators
# setwd("D:/cityhall-elevator-survey");source("param_parsing.R")
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

names(orig.data) <- c("Car", "Time", "Fl", "Dir", "Event", "Val")
orig.data$Time <- as.POSIXct(orig.data$Time, origin="1970-01-01")

moving.time <- data.frame(); ddply(subset(orig.data, Event=="DOORSTAT" | Event=="DOORISOPEN" | Event=="REQOPEN"), .(Car), ComputeMovingTime, out=moving.time)
moving.time.1 <- subset(moving.time, dir!=0 & duration < 600)
write.csv(moving.time.1, "moving_time.csv")

stop("stopped manually")

p <- ggplot(subset(orig.data2, Event == "WEIGHT" & Val != -99 & Time < "2015-12-30 13:00:00"), aes(Time, Val, color=Car)) 
print(p + geom_line() + labs(y="Weight Percentage"))

weight <- subset(orig.data2, Event=="WEIGHT" & Val != -99) 
ww <- data.frame()
ddply(weight, .(Car), CountNoPpLv, out.data=ww)

p <- ggplot(subset(ww, Car!="NC1"), aes(time.interval, no.p.lv, color=Car)) 
print(p + geom_line() + labs(y="# of People Left"))

#stop("stopped manually")

