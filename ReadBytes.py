import serial
import struct
import Handshake

Handshake.executeHandshake()
print("Handshake Complete")

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=None)
#ser.open()

#ser.write("testing")

messageLength = 0
global deviceA
global deviceB
global deviceD
global deviceE
global deviceF
global deviceG
global deviceH
deviceA = 0.0
deviceB = 0.0
deviceD = 0.0
deviceE = 0.0
deviceF = 0.0
deviceG = 0.0
deviceH = 0.0

def checksumIsCorrect(forChecksum):
        #print("forChecksum: %d"%forChecksum)
        #print("forChecksum length: %d"%len(forChecksum))
        checksum = 0
        for i in range (0,messageLength):
                compare = struct.unpack('B',forChecksum[i])
                compare1 = int(compare[0])
                checksum = checksum ^ compare1
        #print ("final checksum")
        #print (checksum)
        originalChecksum = struct.unpack('B',forChecksum[messageLength])
        #print ("original checksum")
        #print (originalChecksum)
        originalChecksum1 = int(originalChecksum[0])
        print (checksum == originalChecksum1)
        if (checksum == originalChecksum1):
                ser.write('1')
        else:
                ser.write('0')


try:

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
                        messageLength = struct.unpack('B',ch)[0]
                        flag = 1
                        #print("messagelength: %d"%messageLength)
                

                ch=(ser.read())
		#print(ch)
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
                                        deviceA = value
                                elif(deviceCode == 'B'):
                                        deviceB = value
                                elif(deviceCode == 'D'):
                                        deviceD = value
                                elif(deviceCode == 'E'):
                                        deviceE = value
                                elif(deviceCode == 'F'):
                                        deviceF = value
                                elif(deviceCode == 'G'):
                                        deviceG = value
                                elif(deviceCode == 'H'):
                                        deviceH = value
                                rv.append(value)
                if(len(forChecksum)==(messageLength+1)):
                        #print("Getting checksum")
                        isCorrect = checksumIsCorrect(forChecksum)
                        flag = 0
                        #print(isCorrect)
                        print("Device A: %f"%deviceA)
                        print("Device B: %f"%deviceB)
                        print("Device D: %f"%deviceD)
                        print("Device E: %f"%deviceE)
                        print("Device F: %f"%deviceF)
                        print("Device G: %f"%deviceG)
                        print("Device H: %f"%deviceH)                        
               
except KeyboardInterrupt:
        ser.close()
                

