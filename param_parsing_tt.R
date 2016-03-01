#!/bin/
# author: cchuang
# description: analyze the efficiency of cityhall's elevators
# setwd("D:/cityhall-elevator-survey");source("param_parsing_tt.R")
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

orig.data$time <- as.POSIXct(orig.data$time, origin="1970-01-01")

# Constant
# car group list
car.grp <- list("NC1-3"=c("NC1", "NC2", "NC3"), "NC4-5"=c("NC4", "NC5"), "NC6-"=c("NC6"))

travling.time <- data.frame(); d_ply(subset(orig.data, id %in% unlist(car.grp)), .(id), ComputeMovingTime, out=travling.time)
travling.time.1 <- subset(travling.time, dir!=0 & duration < 600)
write.csv(travling.time.1, "travling_time.csv")

