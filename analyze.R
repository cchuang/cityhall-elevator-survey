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
source("analysis_func.R")

#cl <- makeCluster(detectCores())
#registerDoParallel(cl)

#######################################
# Global Variables
cpt.data <- data.frame()

# Load data
out.list <- read.csv("out_list.txt", header=FALSE, stringsAsFactors=FALSE)
l <- lapply(out.list[[1]], read.csv, stringsAsFactors=FALSE)
orig.data <- do.call("rbind", l)

orig.data$time <- as.POSIXct(orig.data$time, origin="1970-01-01")

###########################################################
# Waiting Time
wt.data <- data.frame()
d_ply(subset(orig.data, id %in% c("NC1-3", "NC4-5", "NC6~") & event %in% c("REQ_UP", "REQ_DOWN")), .(id, para, event), CountTimeDuration, out.data=wt.data)
wt.data <- wt.data[ with(wt.data, order(time)), ]
stop("debugging")
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
	p <- p + stat_ecdf() + facet_grid(Fl ~ .) + labs(x="Waiting Time (in seconds)", title="CDF of Waiting Time (Down)")
	print(p)
	ggsave(sprintf("ecdf_dB_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)

	# multiple call
	hf.data <- subset(wt.data3, (Fl==11 | Fl==12))
	res <- ddply(subset(hf.data, Event=="REQDOWN"), .(CarGrp),  CountMultiSummoning, full.data=hf.data, event="REQDOWN")
	print(res)

	hf.data <- subset(wt.data3, (Fl==1 ))
	res <- ddply(subset(hf.data, Event=="REQUP"), .(CarGrp),  CountMultiSummoning, full.data=hf.data, event="REQUP")
	print(res)

	# model fiting
	fit <- ddply(wt.data3, .(CarGrp, Fl, Event), 
		  function(x){
			  #fit <- fitdistr(x$Duration, "exponential")
			  fit <- fitdistr(x$Duration, "lognormal")
			  p.value <- ks.test(x$Duration, "plnorm", fit$estimate)$p.value
			  return(data.frame(meanlog=fit$estimate[1], sdlog=fit$estimate[2], 
								meanlog.sd=fit$sd[1], sdlog.sd=fit$sd[2], 
								n=fit$n, p.value=p.value))
		  }) %>% subset(n>3)
	fit %<>% within({
		lower <- meanlog - 1.95 * sdlog
		upper <- meanlog + 1.95 * sdlog
	})

	p <- ggplot(fit, aes(Fl, meanlog, color=CarGrp, label=sprintf("%.2f", exp(meanlog)))) + 
		 geom_errorbar(aes(ymin=lower, ymax=upper))
	print(p + facet_grid(Event ~ .) + 
		  geom_label() + annotate("text", label="Numbers in the labels are the medians of the fitted model", x = 5, y = 0.005, size = 5, color="red") + 
		  labs(x="Floor", y=expression(mu), title="Log-normal Distribution Fitted by ML")) 
	ggsave(sprintf("fit_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)

	fit.est.12 <- subset(fit, Fl==12)[,c("meanlog", "sdlog")]
	p <- ggplot(subset(wt.data3, Event == "REQDOWN" & Fl==12), aes(Duration, color=CarGrp)) 
	p <- p + stat_ecdf() + labs(x="Waiting Time (in seconds)", title="CDF of Waiting Time (Down)")
	p <- p + 
		stat_function(fun = plnorm, args = fit.est.12[1,], color = "blue") + 
		stat_function(fun = plnorm, args = fit.est.12[2,], color = "blue") 
	print(p)
	ggsave(sprintf("ecdf_dB_%s.png", format(target.time, "%F_%H%M")), width=out.width, height=out.height)
}

