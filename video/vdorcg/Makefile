# Makefile for the windows solution
MSBUILD="/cygdrive/c/Program Files (x86)/MSBuild/14.0/Bin/MSBuild.exe"
SOLUTION="vdorcg.sln"
TARGET="Debug/vdorcg.exe"
PLATFORM="x86"

all: 
	@$(MSBUILD) $(SOLUTION) /v:q /p:Configuration=Release /p:Platform=$(PLATFORM) /maxcpucount:6 

debug:
	@$(MSBUILD) $(SOLUTION) /v:q /p:Configuration=Debug /p:Platform=$(PLATFORM) /maxcpucount:6 

show:
	@$(MSBUILD) $(SOLUTION) /detailedsummary

clean:
	@$(MSBUILD) $(SOLUTION) /t:clean /v:q /p:Configuration=Debug /p:Platform=$(PLATFORM) /maxcpucount:6 
	#@$(MSBUILD) $(SOLUTION) /t:clean /v:q /p:Configuration=Release /maxcpucount:6 
	
test:
	$(TARGET) "D:\tpc.elev\video\chunks\20160106_00000000.mp4"
#	$(TARGET) 

.PHONY: all debug show clean

