# Makefile for the windows solution
MSBUILD="/cygdrive/c/Program Files (x86)/MSBuild/14.0/Bin/MSBuild.exe"
SOLUTION="vdorcg.sln"
TARGET="Debug/vdorcg.exe"
#TARGET="Release/vdorcg.exe"
PLATFORM="x86"

all: 
	@$(MSBUILD) $(SOLUTION) /v:q /p:Configuration=Release /p:Platform=$(PLATFORM) /maxcpucount:6 

debug:
	@$(MSBUILD) $(SOLUTION) /v:q /p:Configuration=Debug /p:Platform=$(PLATFORM) /maxcpucount:6 

show:
	@$(MSBUILD) $(SOLUTION) /detailedsummary

clean:
	@$(MSBUILD) $(SOLUTION) /t:clean /v:q /p:Configuration=Debug /p:Platform=$(PLATFORM) /maxcpucount:6 
	@$(MSBUILD) $(SOLUTION) /t:clean /v:q /p:Configuration=Release /p:Platform=$(PLATFORM) /maxcpucount:6 
	
test:
	$(TARGET) "D:\cityhall-elevator-survey\video\chunks\1451444819_00000000.mp4" 
	#$(TARGET) "D:\cityhall-elevator-survey\video\chunks\1451444819_00043200.mp4" 

.PHONY: all debug show clean

