# Makefile for Ubuntu
TARGET := vdorcg
OBJS := ElevSetStat.o ElevStat.o FloorStat.o vdorcg.o
SRCS := $(OBJS:.o=.cpp)

CXXFLAGS := -std=c++0x -O3
LDFLAGS := -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video -ltesseract

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS) 

%.o : %.cpp 
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

debug:
	#@$(MSBUILD) $(SOLUTION) /v:q /p:Configuration=Debug /p:Platform=$(PLATFORM) /maxcpucount:6 

show:
	#$(MSBUILD) $(SOLUTION) /detailedsummary

clean:
	@rm $(OBJS) $(TARGET)
#	
test:
	TZ=CST-8; $(TARGET) test_input.txt
	#$(TARGET) "D:\cityhall-elevator-survey\video\chunks\1451444819_00000000.mp4" 
	#$(TARGET) "D:\cityhall-elevator-survey\video\chunks\1451444819_00043200.mp4" 

.PHONY: all debug show clean

