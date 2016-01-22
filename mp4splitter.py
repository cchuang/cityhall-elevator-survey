#!/usr/bin/python
# author: cchuang.tw@gmail.com

import sys
import os
import re
import StringIO
import subprocess

def main(infile, time_period, start_time):
    ffm = "/usr/ffmpeg/bin/ffmpeg.exe"
    
    devnull = open(os.devnull, "w+")
    
    print infile
    # Determine how long the video is. (len_in_sec)
    p = subprocess.Popen([ffm, "-i", infile], stderr=subprocess.PIPE)
    info = StringIO.StringIO(p.communicate()[1])
    len_in_sec = 0
    for line in info: 
        m = re.search("Duration: +([0-9:\.]+)", line)
        if m: 
            time_fmt = re.match("([0-9]+):([0-9]+):([0-9]+)", m.group(1))
            len_in_sec = int(time_fmt.group(1)) * 60 * 60 +  \
                    int(time_fmt.group(2)) * 60 + int(time_fmt.group(3)) + 1
    
    info.close()
    
    while start_time < len_in_sec: 
        subprocess.call([ffm, "-y", "-i", infile, \
                "-vcodec", "copy", "-ss", str(start_time), "-t", str(time_period), \
                "out_{:08d}.mp4".format(start_time)], stderr=devnull)
        start_time += time_period
    
    devnull.close() 

if __name__ == "__main__": 
    if len(sys.argv) < 3:  
        print "Usage: " + sys.argv[0] + " <infile> <time segment size in seconds> [<start time>]"
        exit()

    if len(sys.argv) > 3:  
        start_time = int(sys.argv[3])
    else:
        start_time = 0

    main(sys.argv[1], int(sys.argv[2]), start_time)


