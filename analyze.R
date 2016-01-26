#!/bin/
# author: cchuang
# description: analyze the efficiency of cityhall's elevators
# setwd("D:/cityhall-elevator-survey");source("analyze.R")
#library(doParallel)
library(plyr)
library(foreach)
library(xts)
library(ggplot2)
library(MASS)
library(magrittr)
library(reshape2)

#cl <- makeCluster(detectCores())
#registerDoParallel(cl)

#######################################
# Global Variables
cpt.data <- data.frame()

#target.time1 <- 1451444819
#target.time2 <- 0
target.time1 <- 1452052326
target.time2 <- 86400
target.time <- as.POSIXct(target.time1 + target.time2, origin="1970-01-01")

# Load data
if (!exists("orig.data")) {
	orig.data <- read.csv(sprintf("data/%d_%08d.csv", target.time1, target.time2), header = FALSE, skip=1, stringsAsFactors=FALSE)
	names(orig.data) <- c("Car", "Time", "Fl", "Event", "Val")
	# The panel is changed at this time. 
	if ((target.time1 + target.time2) ==  1451444819) {
		orig.data <- subset(orig.data, (Time < 2816410 | Time > 2816510))
	}
}

nrow.data <- ddply(orig.data, .(Car, Fl, Event), RmTvEntries, start_time=target.time1 + target.time2)

#RmTvEntries(subset(orig.data, Event=="REQUP" & Fl==1 & Car=="NC1"))
# cpt.data <- cpt.data[with(cpt.data, order(Time)),]

###########################################################
# Waiting Time
wt.data <- data.frame()
ddply(cpt.data, .(Car, Fl, Event), CountTimeDuration, out.data=wt.data)

wt.data <- within(wt.data, { 
	Duration <- as.numeric(Duration)
	Fl <- as.factor(Fl)
	CarGrp <- character(length = length(Car))
	CarGrp[Car == "NC1" | Car == "NC2" | Car == "NC3"] <- "CG1"
	CarGrp[Car == "NC1-3"] <- "CG1B"
	CarGrp[Car == "NC4" | Car == "NC5"] <- "CG2"
	CarGrp[Car == "NC4-5"] <- "CG2B"
	CarGrp[Car == "NC6"] <- "CG3"
	CarGrp[Car == "NC6~"] <- "CG3B"
})

#MergeCarChEvents(wt.data) 
#stop("Stop for debugging")

wt.data2 <- subset(wt.data, Event=="REQUP" | Event=="REQDOWN" | Event=="REQSTOP" | Event=="DOORISOPEN")

#p <- ggplot(wt.data2, aes(Fl, Duration)) 
#print(p + geom_boxplot() + facet_grid(CarGrp ~ Event))

out.width <- 16
out.height <- 10
#############################################################
# Sum by each single car

if (FALSE) {
	# Boxplot
	p <- ggplot(subset(wt.data2, Event != "REQSTOP" & Event != "DOORISOPEN"), aes(Fl, Duration)) 
	print(p + geom_boxplot(aes(colour=CarGrp)) + facet_grid(Event ~ .) + labs(y="Waiting Time (in seconds)"))
	ggsave(sprintf("boxplot_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)

	# Histogram for counting how many times the buttons are pressed. 
	p <- ggplot(subset(wt.data2, Event != "DOORISOPEN"), aes(Fl)) 
	print(p + stat_count(aes(fill=CarGrp), position="dodge") + facet_grid(Event ~ .))
	ggsave(sprintf("hist_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)

	# Histogram for how long the car is waiting. 
	p <- ggplot(subset(wt.data2, Event == "DOORISOPEN" & (Fl==12 | Fl==11 | Fl==10 | Fl==9 | Fl==8 | Fl==7)), aes(Duration)) 
	#p <- ggplot(subset(wt.data2, Event == "DOORISOPEN" & (as.numeric(Fl)>3)), aes(Duration)) 
	print(p + geom_histogram(aes(fill=CarGrp), position="dodge") + facet_grid(CarGrp ~ Fl))
	ggsave(sprintf("hist2_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)

	p <- ggplot(subset(wt.data2, Event == "REQDOWN" & (Fl==12 | Fl==11 | Fl==10 | Fl==9 | Fl==8 | Fl==7 | Fl==1)), aes(Duration, color=Fl)) 
	print(p + stat_ecdf() + facet_grid(CarGrp ~ .) + labs(x="Waiting Time (in seconds)", title="CDF of Waiting Time (Down)"))
	ggsave(sprintf("ecdf_d1_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)

	p <- ggplot(subset(wt.data2, CarGrp != "CG3" & Event == "REQDOWN" & (Fl==12 | Fl==11 | Fl==10 | Fl==9 | Fl==8 | Fl==7 | Fl==1)), aes(Duration, color=Fl)) 
	print(p + stat_ecdf() + facet_grid(CarGrp ~ .) + labs(x="Waiting Time (in seconds)", title="CDF of Waiting Time (Down)"))
	ggsave(sprintf("ecdf_d2_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)

	p <- ggplot(subset(wt.data2, Event == "REQUP" & (Fl==2 | Fl==1)), aes(Duration, color=Fl)) 
	print(p + stat_ecdf() + facet_grid(CarGrp ~ .) + labs(x="Waiting Time (in seconds)", title="CDF of Waiting Time (Up)"))
	ggsave(sprintf("ecdf_u1_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)

	p <- ggplot(subset(wt.data2, CarGrp != "CG3" & Event == "REQUP" & ( Fl==2 | Fl==1)), aes(Duration, color=Fl)) 
	print(p + stat_ecdf() + facet_grid(CarGrp ~ .) + labs(x="Waiting Time (in seconds)", title="CDF of Waiting Time (Up)"))
	ggsave(sprintf("ecdf_u2_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)
}


# Show wrong results
# subset(wt.data2, Event=="DOORISOPEN" & Duration < 2 )

###################################################
# Multi call
if (FALSE) {
	hf.data <- subset(wt.data2, (Fl==11 | Fl==12))
	res <- ddply(subset(hf.data, Event=="REQDOWN"), .(CarGrp),  CountMultiSummoning, full.data=hf.data, event="REQDOWN")
	print(res)

	hf.data <- subset(wt.data2, (Fl==1 ))
	res <- ddply(subset(hf.data, Event=="REQUP"), .(CarGrp),  CountMultiSummoning, full.data=hf.data, event="REQUP")
	print(res)
}
#stopCluster(cl)

###################################################
# Analysis against car group
if (TRUE) {
	wt.data3 <- subset(wt.data, CarGrp=="CG1B" | CarGrp=="CG2B")

	p <- ggplot(subset(wt.data3), aes(Fl, Duration)) 
	print(p + geom_boxplot(aes(colour=CarGrp)) + facet_grid(Event ~ .) + labs(y="Waiting Time (in seconds)"))
	ggsave(sprintf("boxplotB_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)

	p <- ggplot(subset(wt.data3, Event == "REQUP" & Fl==1), aes(Duration, color=CarGrp)) 
	print(p + stat_ecdf() + labs(x="Waiting Time (in seconds)", title="CDF of Waiting Time (Up)"))
	ggsave(sprintf("ecdf_uB_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)
	p <- ggplot(subset(wt.data3, Event == "REQDOWN" & (Fl==12 | Fl==11)), aes(Duration, color=CarGrp)) 
	print(p + stat_ecdf() + facet_grid(Fl ~ .) + labs(x="Waiting Time (in seconds)", title="CDF of Waiting Time (Down)"))
	ggsave(sprintf("ecdf_dB_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)

	hf.data <- subset(wt.data3, (Fl==11 | Fl==12))
	res <- ddply(subset(hf.data, Event=="REQDOWN"), .(CarGrp),  CountMultiSummoning, full.data=hf.data, event="REQDOWN")
	print(res)

	hf.data <- subset(wt.data3, (Fl==1 ))
	res <- ddply(subset(hf.data, Event=="REQUP"), .(CarGrp),  CountMultiSummoning, full.data=hf.data, event="REQUP")
	print(res)

	fit <- ddply(wt.data3, .(CarGrp, Fl, Event), 
		  function(x){
			  fit <- fitdistr(x$Duration, "exponential")
			  return(data.frame(est=fit$estimate, sd=fit$sd, n=fit$n))
		  }) %>% subset(n>3)
	fit %<>% within({
		lower <- est - 1.95 * sd
		upper <- est + 1.95 * sd
	})
	p <- ggplot(fit, aes(Fl, est, color=CarGrp, label=sprintf("%.2f", 1.0/est))) + 
		 geom_errorbar(aes(ymin=lower, ymax=upper))
	print(p + facet_grid(Event ~ .) + 
		  geom_label() + annotate("text", label="Numbers in the labels are means of the fitted model, which is 1/lambda", x = 5, y = 0.01, size = 5, color="red") + 
		  labs(x="Floor", y=expression(lambda), title="Exponential Fit by ML")) 
}

