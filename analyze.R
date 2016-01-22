#!/bin/
# author: cchuang
# description: analyze the efficiency of cityhall's elevators
# setwd("D:/tpc.elev");source("analyze.R")
#library(doParallel)
library(plyr)
library(foreach)
library(xts)
library(ggplot2)

#cl <- makeCluster(detectCores())
#registerDoParallel(cl)

#######################################
# Global Variables
cpt.data <- data.frame()

########################################
# Functions

# default started at 2015-12-30 11:00:00
RmTvEntries <- function(x, start_time=1451444400) {
	# sorting
	x <- x[ with(x, order(Time)), ]
	rl.res <- rle(x$Val)
	cumsum(rl.res$lengths)
	# discard the first and the last sample due to incomplete record period. 
	edge <- c(cumsum(rl.res$lengths)[-length(rl.res$lengths)] + 1) 
	x.tmod <- x[edge, ]
	x.tmod$Tick <- x.tmod$Time
	x.tmod$Time <- as.POSIXct(x.tmod$Tick/1000 + start_time, origin="1970-01-01")

	# load/store global variable
	cpt.data <- get("cpt.data", envir = .GlobalEnv) 
	cpt.data <- rbind(cpt.data, x.tmod) 
	assign("cpt.data", cpt.data, envir = .GlobalEnv) 

	return(nrow(x))
}

CountTimeDuration <- function(x, out.data=data.frame()) {
	# sorting
	x <- x[ with(x, order(Time)), ]
	if (x$Val[1] != 1) {
		x <- x[-1,]
	}
	if (nrow(x) %% 2 == 1) {
		x <- x[-nrow(x),]
	}
	if (nrow(x) == 0) {
		return(0)
	}
	
	# Even rows of x
	x.even <- x[seq(2, nrow(x), 2),]
	# Odd rows of x
	x.odd <- x[seq(1, nrow(x), 2),]
	browser("The events are NOT arranged as 1 0 1 0...", expr=(x.even$Val!=0 || x.odd$Val!=1))

	x.odd$Duration <- x.even$Time - x.odd$Time
	x.odd$Val <- NULL
	eval(substitute(out.data<-rbind(out.data, data.frame(x.odd))), envir=.GlobalEnv)
	return(nrow(x.odd))
}

CountMultiSummoning <- function(x, full.data, event) {
	y <- list(cnt=nrow(x), unchanged=0, multi.waiting=0, uc.rate=0)
	curr.grp <- unique(x$CarGrp)
	cooldown <- vector()
	browser("Must have only one group", expr=(length(curr.grp)!=1))

	full.grps <- unique(full.data$CarGrp)
	other.grps <- full.grps[full.grps != curr.grp]
	y[full.grps] <- 0
	for (i in 1:nrow(x)) {
		xrow <- x[i,]
		# drop those samples that are 2nd press
		# 	we don't drop this because the number looks too small 
		# 	and there are more circumstances that we can still count it
		#focus.rows <- subset(full.data, Event == event 
		#	& CarGrp != xrow$CarGrp
		#	& Fl == xrow$Fl
		#	& Time < xrow$Time)
		#focus.rows <- focus.rows[(xrow$Time - focus.rows$Time) < focus.rows$Duration, ]
		#if (nrow(focus.rows) != 0) {
		#	next
		#}

		focus.rows <- subset(full.data, Event == event 
			& CarGrp != xrow$CarGrp
			& Fl == xrow$Fl
			& Time > xrow$Time)
		focus.rows <- focus.rows[(focus.rows$Time - xrow$Time) < xrow$Duration, ]
		if (nrow(focus.rows) != 0) {
			cooldown[length(cooldown) + 1] <- min(focus.rows$Time - xrow$Time)

			focus.grps <- unique(focus.rows$CarGrp)
			for (grp in focus.grps) {
				y[[grp]] <- y[[grp]] + 1 
			}
			if (all((focus.rows$Time + focus.rows$Duration) >= (xrow$Time + xrow$Duration))) {
				y$unchanged <- y$unchanged + 1
			}
		}

	}
	y$multi.waiting <- length(cooldown)
	y$uc.rate <- y$unchanged / y$multi.waiting
	y$cd.median <- median(cooldown)
	y$cd.mean <- mean(cooldown)
	return(data.frame(y))
}
#ddply(subset(hf.data, Event=="REQDOWN"), .(CarGrp),  CountMultiSummoning, full.data=hf.data, event="REQDOWN")

target.time1 <- 1451444819
target.time2 <- 0
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
ddply(subset(cpt.data, Event=="REQUP" | Event=="REQDOWN" | Event=="REQSTOP" | Event=="DOORISOPEN"), 
	  .(Car, Fl, Event), CountTimeDuration, out.data=wt.data)

wt.data2 <- within(wt.data, { 
	Duration <- as.numeric(Duration)
	Fl <- as.factor(Fl)
	CarGrp <- character(length = length(Car))
	CarGrp[Car == "NC1" | Car == "NC2" | Car == "NC3"] <- "CG1"
	CarGrp[Car == "NC4" | Car == "NC5"] <- "CG2"
	CarGrp[Car == "NC6"] <- "CG3"
})

#p <- ggplot(wt.data2, aes(Fl, Duration)) 
#print(p + geom_boxplot() + facet_grid(CarGrp ~ Event))

out.width <- 16
out.height <- 10
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


# Show wrong results
# subset(wt.data2, Event=="DOORISOPEN" & Duration < 2 )

###################################################
# Multi summoning
hf.data <- subset(wt.data2, (Fl==11 | Fl==12))
res <- ddply(subset(hf.data, Event=="REQDOWN"), .(CarGrp),  CountMultiSummoning, full.data=hf.data, event="REQDOWN")
print(res)

hf.data <- subset(wt.data2, (Fl==1 ))
res <- ddply(subset(hf.data, Event=="REQUP"), .(CarGrp),  CountMultiSummoning, full.data=hf.data, event="REQUP")
print(res)
#stopCluster(cl)

