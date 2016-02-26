# cityhall-elevator-survey
Analyze the cityhall-elevators efficiency by the video of the control panel

## vdorcg
An application recognizes the details and status of the elevator system of the city-hall by OpenCV and Tesseract-OCR. It will output events in CSV files which are divided into pieces by day automatically. 
### Input
Merely a list of files with time stamps (sum of this two number, since UNIX Epoch) as their files. Must be listed chronically. 
```
D:\cityhall-elevator-survey\video\chunks\1451444819_00000000.mp4
D:\cityhall-elevator-survey\video\chunks\1451444819_00007200.mp4
```

### Ouput
CSV files like this

```
id,time,floor,direction,event,para,para2
NC4,1451444819,4,-1,WEIGHT,8,
NC4,1451444819,4,-1,OPENING,4,
NC6,1451444819,5,1,ARRIVING,5,
```

#### ids
| id   | Type        |Affected Cars| 
|------|-------------|-------------|
|NC1   | Regular car |NC1 |
|NC2   | Regular car |NC2 |
|NC3   | Regular car |NC3 |
|NC4   | Regular car |NC4 |
|NC5   | Regular car |NC5 |
|NC6   | Regular car |NC6 |
|NC1-3 | Car group   |NC1, NC2, NC3|
|NC4-5 | Car group   |NC4, NC5|
|NC6-  | Car group   |NC6|

#### time
Time stamps (seconds since UNIX Epoch 1970-01-01) of this event. 

#### floor
The current position of this car. 1st floor is 1, 2nd floor is 2, B1 is -1 and so on. 

#### direction
|Direction|Description|
|---------|-----------|
|1        |The car is going up.|
|-1       |The car is going down.|
|0        |The car will stay here till a new request is arrival.|

#### Events, Para, Para2
| Event    | Description | Parameter 1 | Parameter 2 |
|----------|-------------|-------------|-------------|
| REQ_STOP | A list of the pressed floor buttons on the car panel. | A colon ( : ) separated list. | NULL |
| REQ_UP   | The UP button in the hall of N'th floor has been pressed. | N | Set (1) or reset (0) |
| REQ_DOWN | The DOWN button in the hall of N'th floor has been pressed. | N | Set (1) or reset (0) |
| OPENING  | The door of this car is opening. | N, generally it and the floor field are the same. | NULL |
| OPENED   | The door of this car has been fully opened. | N, generally it and the floor field are the same. | NULL |
| CLOSING  | The door of this car is closing. | N, generally it and the floor field are the same. | NULL |
| CLOSED   | The door of this car has been fully closed. | N, generally it and the floor field are the same. | NULL |
| LEAVING  | The car is leaving N'th floor.  | N | NULL |
| ARRIVING | The car is arriving N'th floor. | N | NULL |
| WEIGHT   | The loading of this car in percent. The rated full load is 1350 kg. | Percentage (e.g. 40) | NULL |
| REQ_OPEN | The 'OPEN' button on the car panel is pressed. | Set (1) or reset (0) | NULL |
| STOP_SRV | The car is out of service. | Set (1) or reset (0) | NULL |
| UNKNOWN_MOVING | The car's status is unknown. | N | NULL |


