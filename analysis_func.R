#!/bin/
# author: cchuang.tw@gmail.com
# description: functions of analyzis the efficiency of cityhall's elevators
# setwd("D:/cityhall-elevator-survey");source("analysis_func.R")

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

# one x event endded when y event were happening
HasIntersection <- function(x, y, delay=0) {
	end.time <- x$Time + x$Duration + delay
	return(y[which(end.time >= y$Time & end.time <= (y$Time + y$Duration))[1],])
}

Farseer <- function(x, y, back=5, forward=60) {
	end.time <- x$Time + x$Duration
	y <- subset(y, Event != "CARISHERE" & CarGrp == x$CarGrp & Fl == x$Fl)
	return(subset(y, y$Time >= end.time - back & y$Time <= end.time + forward))
}

MergeCarChEvents <- function(x) {
	x <- x[ with(x, order(Time)), ]
	out <- data.frame()
	# input must have time duration info
	count <- 0
	for (i in 1:nrow(x)) {
		r <- x[i,]
		if (!((r$Event == "REQUP") | (r$Event == "REQDOWN"))) {
			next
		} 
		
		delay <- 0
		#c0 <- HasIntersection(r, subset(x, Event=="DOORISOPEN" &
		#		   CarGrp==r$CarGrp & Fl==r$Fl), delay=delay - 1)
		arrival <- HasIntersection(r, subset(x, Event=="DOORISOPEN" &
				   CarGrp==r$CarGrp & Fl==r$Fl), delay=5)
		call <- HasIntersection(r, subset(x, Event==r$Event &
				   CarGrp==r$CarGrp & Fl==r$Fl & Car!=r$Car), delay=1)
		#its <- HasIntersection(r, subset(x, 
		#			(Event==r$Event | Event=="DOORISOPEN") & 
		#			CarGrp==r$CarGrp & Fl==r$Fl), delay=delay)
		its <- rbind(arrival, call)
		its <- its[min(its$Time), ]

		r$end.time <- r$Time + r$Duration
		r$follow.by <- its$Event
		browser()
		#if (all(is.na(its))) {
			#if (!all(is.na(c))) {
				#print("====================================")
				#print(r)
				#print(its)
				#count <- count + 1
				#if (count > 10) break
			#}
			out <- rbind(out, r)
		#}
	}
	return(out)
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

CountNoPpLv <- function(x, out.data=data.frame()) {
	# sorting
	x <- x[ with(x, order(Time)), ]

	xp <- data.frame(Car=x$Car, est.no.p = x$Val / 5)# max load 20 people, 
	xp$est.no.p.l <- xp$est.no.p - c(xp$est.no.p[-1], NA) # no. of people leave
	xp$est.no.p.l[xp$est.no.p.l < 0] <- 0

	#d <- timeBasedSeq("2010-05-24 0000/2010-05-30 0000/H")
	start.date <- format(min(x$Time), "%Y-%m-%d")
	end.date <- format(max(x$Time + 24 * 60 *60), "%Y-%m-%d")
	d <- timeBasedSeq(paste0(c(start.date, " 0000/", end.date, " 0000/H"), collapse=""))

	#xp$time.interval <- d[findInterval(x$Time, d)]
	xp$time.interval <- findInterval(x$Time, d)
	res <- ddply(xp, .(Car, time.interval), summarize, no.p.lv=sum(est.no.p.l, na.rm=TRUE))
	res$time.interval <- d[res$time.interval] 

	eval(substitute(out.data<-rbind(out.data, data.frame(res))), envir=.GlobalEnv)
	return(nrow(xp))
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

ComputeMovingTime <- function(x, out=data.frame()) {
	# sorting
	x <- x[ with(x, order(time)), ]

	open <- subset(x, event=="OPENING")
	closed <- subset(x, event=="CLOSED")

	# remove duplicated rows in time
	rl.res <- rle(as.character(closed$time))
	closed <- closed[cumsum(rl.res$lengths), ]

	closed.dir <- xts(as.numeric(closed$direction), order.by = closed$time)

	if (nrow(open) %% 2 == 1) {
		open <- open[-nrow(open),]
	}
	# Even rows of open
	print(head(x))
	open.even <- open[seq(2, nrow(open), 2),]
	# Odd rows of open
	open.odd <- open[seq(1, nrow(open), 2),]

	mt <- data.frame(car = open.odd$id, 
					 src = open.odd$floor, 
					 dst = open.even$floor, 
					 start.time = open.odd$time, 
					 duration = open.even$time - open.odd$time,
					 close.time = open.even$time, 
					 dir = 0, req.open=FALSE)

	# the elevator is going down or going up or just stop
	# it's curcial that we have to rule out stop cases because this's not a moving elevator. 
	time.seg <- paste0(open.odd$time, '/', open.even$time)
	mt.rm.list <- vector()
	for (i in 1:nrow(mt)) {
		dir <- closed.dir[time.seg[i]]
		if (length(dir) == 1) {
			mt$dir[i] <- dir[1]
			mt$close.time[i] <- index(dir)[1]
		} else if (length(dir) > 1) {
			# why multiple items returned? the xts comparison is closed. and we do need so due to poor resolution of time_t. 
			if (mt$start.time[i] == index(dir)[1]) {
				mt$dir[i] <- dir[2]
				mt$close.time[i] <- index(dir)[2]
			} else if (mt$start.time[i] + mt$duration[i] == index(dir)[2]){
				mt$dir[i] <- dir[1]
				mt$close.time[i] <- index(dir)[1]
			} 
		} else if (length(dir) == 0) {
			# these are exceptions such as someone interfered the control panel
			mt.rm.list[length(mt.rm.list) + 1] <- -i
		}
	}
	if (length(mt.rm.list) > 0) {
		mt <- mt[mt.rm.list,]
	}

	browser("mt == 0", expr=(nrow(mt)==0))

	########################################
	# Record that the OPEN button is pressed. 
	for (i in 1:nrow(mt)) {
		m <- mt[i,]
		xs <- subset(x, time >= m$start.time & time <= m$close.time & event == "REQ_OPEN" & para == "1")
		if (nrow(xs) != 0) {
			mt$req.open[i] <- TRUE
		} else {
			mt$req.open[i] <- FALSE
		}
	}

	eval(substitute(out<-rbind(out, mt)), envir=.GlobalEnv)

}

