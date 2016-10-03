import serial
import struct
import Handshake

Handshake.executeHandshake()
print("Handshake Complete")

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=None)
#ser.open()

#ser.write("testing")

#messageLength = 0

#change global variable device name
pedo = 0.0
compass = 0.0
ultraCentre = 0.0
ultraBottom = 0.0
ultraRight = 0.0
ultraLeft = 0.0
infra = 0.0

locationReceivedFlag = 0
startingBuildingNo = 0
startingLvlNo = 0
startingNodeNo = 0
endingBuildingNo = 0
endingLevelNo = 0
endingNodeNo = 0

def checksumIsCorrect(forChecksum,messageLength):
        #print("forChecksum: %d"%forChecksum)
        #print("forChecksum length: %d"%len(forChecksum))
        checksum = 0
        for i in range (0,messageLength):
                compare = struct.unpack('B',forChecksum[i])[0]
                checksum = checksum ^ compare
        #print ("final checksum")
        #print (checksum)
        originalChecksum = struct.unpack('B',forChecksum[messageLength])[0]
        #print ("original checksum")
        #print (originalChecksum)
        print (checksum == originalChecksum)
        if (checksum == originalChecksum):
                return True
        else:
                return False

def pollData():
        try:

                global pedo
                global compass
                global ultraCentre
                global ultraBottom
                global ultraRight
                global ultraLeft
                global infra

                global locationReceivedFlag
                global startingBuildingNo
                global startingLvlNo
                global startingNodeNo
                global endingBuildingNo
                global endingLevelNo
                global endingNodeNo


                pedoTemp = 0.0
                compassTemp = 0.0
                ultraCentreTemp = 0.0
                ultraBottomTemp = 0.0
                ultraRightTemp = 0.0
                ultraLeftTemp =0.0
                infraTemp = 0.0
                
                locationReceivedFlagTemp = 0.0
                startingBuildingNoTemp = 0.0
                startingLvlNoTemp = 0.0
                startingNodeNoTemp = 0.0
                gendingBuildingNoTemp = 0.0
                endingLevelNoTemp = 0.0
                endingNodeNoTemp = 0.0
                #print("Reading..");
                flag = 0
                messageLength = 0
                #rv = []
                while(1):
                        if (flag == 0):
                                rv = []
                                ch = ""
                                forChecksum = []
                                ch=(ser.read())
                                print(ch)
                                messageLength = struct.unpack('B',ch)[0]
                                flag = 1
                                print("messagelength: %d"%messageLength)

                        ch=(ser.read())
                        print(ch)
                        rv.append(ch)
                        forChecksum.append(ch)
                        if(rv[len(rv)- 1] != '' or rv[len(rv)-1]!='\r'):
                                if (rv[len(rv)-1].isalpha() and len(forChecksum)!=(messageLength+1)):
                                        deviceCode = rv[len(rv)-1]
                                        #print("device detected")
                                        reconstruct = ""
                                        #print(deviceCode)
                                        for x in range (0,4):
                                                ch=(ser.read())
                                                reconstruct += ch
                                                forChecksum.append(ch)

                                        value = struct.unpack('f',reconstruct)[0]
                                        #print("reconstructed value: %d"%value)
                                        if(deviceCode == 'A'):
                                                pedoTemp = value
                                        elif(deviceCode == 'B'):
                                                compassTemp = value
                                        elif(deviceCode == 'D'):
                                                ultraCentreTemp = value
                                        elif(deviceCode == 'E'):
                                                ultraBottomTemp = value
                                        elif(deviceCode == 'F'):
                                                ultraRightTemp = value
                                        elif(deviceCode == 'G'):
                                                ultraLeftTemp = value
                                        elif(deviceCode == 'H'):
                                                infraTemp = value
                                        #else:
                                        if(locationReceivedFlag == 0): #saves process wont check after location is keyed in 1 time
                                                if(deviceCode == 'U'):
                                                        startingBuildingNoTemp = value
                                                elif(deviceCode == 'V'):
                                                        startingLvlNoTemp = value
                                                elif(deviceCode == 'W'):
                                                        startingNodeNoTemp = value
                                                elif(deviceCode == 'X'):
                                                        endingBuildingNoTemp = value
                                                elif(deviceCode == 'Y'):
                                                        endingLevelNoTemp = value
                                                elif(deviceCode == 'Z'):
                                                        endingNodeNoTemp = value
                                                        locationReceivedFlag = 1
                                        rv.append(value)
                        #print("Chksum: %d" %len(forChecksum))
                        #print("Message: %d" %messageLength)
                        if(len(forChecksum)==(messageLength+1)):
                                #print("Getting checksum")
                                isCorrect = checksumIsCorrect(forChecksum,messageLength)
                                if(isCorrect == True):
                                        pedo = pedoTemp
                                        compass = compassTemp
                                        ultraCentre = ultraCentreTemp
                                        ultraBottom = ultraBottomTemp 
                                        ultraRight = ultraRightTemp
                                        ultraLeft = ultraLeftTemp
                                        infra = infraTemp
                                        startingBuildingNo = startingBuildingNoTemp
                                        startingLvlNo = startingLvlNoTemp
                                        startingNodeNo = startingNodeNoTemp
                                        endingBuildingNo = endingBuildingNoTemp
                                        endingLevelNo = endingLevelNoTemp
                                        endingNodeNo = endingNodeNoTemp
                                        ser.write('1')
                                elif(isCorrect == False):
                                        ser.write('0')
                                
                                        
                                flag = 0
                                #print(isCorrect)
                                print("Device A: %f"%pedo)
                                print("Device B: %f"%compass)
                                print("Device D: %f"%ultraCentre)
                                print("Device E: %f"%ultraBottom)
                                print("Device F: %f"%ultraRight)
                                print("Device G: %f"%ultraLeft)
                                print("Device H: %f"%infra)
                                print("pos 1 : %f"%startingBuildingNo)
                                print("pos 2 : %f"%startingLvlNo)
                                print("pos 3 : %f"%startingNodeNo)
                                print("pos 4 : %f"%endingBuildingNo)
                                print("pos 5 : %f"%endingLevelNo)
                                print("pos 6 : %f"%endingNodeNo)
        except KeyboardInterrupt:
                ser.close()

pollData()
