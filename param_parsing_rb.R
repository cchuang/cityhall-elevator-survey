#!/bin/
# author: cchuang
# description: parsing the parameter - riders' behaviors. 
# setwd("D:/cityhall-elevator-survey");source("param_parsing_rb.R")
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
l <- lapply(out.list[[1]], read.csv, stringsAsFactors=FALSE)
orig.data <- do.call("rbind", l)
#orig.data <- read.csv("data/out_2015-12-30.csv", stringsAsFactors=FALSE)

orig.data$time <- as.POSIXct(orig.data$time, origin="1970-01-01")
orig.data <- orig.data[ with(orig.data, order(time)), ]

# Constant
# car group list
car.grp <- list("NC1-3"=c("NC1", "NC2", "NC3"), "NC4-5"=c("NC4", "NC5"), "NC6-"=c("NC6"))

stop("stop manually")

car.g.req <- subset(orig.data, Event %in% c("REQUP", "REQDOWN") & Car %in% names(car.grp))
rb <- data.frame(); d_ply(car.g.req, .(Car, Fl, Event), CountTimeDuration, out.data=rb) 
rb$Dir <- NULL
names(rb) <- c("Car", "Time", "From", "Event", "WaitingTime")
# only get the data at noon. 
rb <- rb[format(rb$Time, "%H") == "12",]
rb$To <- 0
rb$Arrival <- rb$Time
#for (i in 1:nrow(rb)) {
for (i in 1:1) {
	print(rb[i,])
	dio <- subset(orig.data, Car %in% car.grp[[rb$Car[i]]] & 
		   	Event == "DOORISOPEN" & 
			Val == "1" &
			Fl == rb$From[i] & 
			Time >= (rb$Time[i] + rb$WaitingTime[i]) & 
			Time < (rb$Time[i] + rb$WaitingTime[i] + 60 *60)) 
	if (nrow(dio) >= 1) {
		# which car has been called. 
		car.called <- dio$Car[1]
		dio.spcf <- subset(orig.data, Car == car.called &
		   							Event == "DOORISOPEN" & 
									Val == "1" &
									Time >= (rb$Time[i] + rb$WaitingTime[i]) & 
									Time < (rb$Time[i] + rb$WaitingTime[i] + 60 *60)) 
		if (nrow(dio.spcf) >= 2) {
			next.door.open.t <- dio.spcf$Time[2] 
			floor.req <- subset(orig.data, Car == car.called &
								Time >= dio.spcf$Time[1] &
								Time <= dio.spcf$Time[2] &
								Event == "REQSTOP" & 
								Val == "1")
			if (nrow(floor.req) >= 1) {
				rb$To[i] <- floor.req$Fl[1]
			} 
			
			if (nrow(floor.req) >= 2){
				rb.new.rows <- data.frame()
				tmp <- rb[i,]
				for (j in 2:nrow(floor.req)) {
					tmp$To <- floor.req$Fl[j]
					rb.new.rows <- rbind(rb.new.rows, tmp)
				}
				rb <- rbind(rb.new.rows)
			}

			if (nrow(floor.req) == 0) {
				# nobody presses any floor button. 
			}
		} else {
			# ignore 
			print(paste0("Nothing found, the door is not opened. might be an empty car. ", rb[i]))
		}
	} else {
		# ignore 
		print(paste0("Nothing found", rb[i]))
	}
}
