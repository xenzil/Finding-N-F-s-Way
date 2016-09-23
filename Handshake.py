import serial
import time

ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=2)

def executeHandshake():
	if(ser.isOpen() == False):
		ser.open()

	while True:
		ser.write('1')
		#print("sending 1")
		ack = ser.read();
		#print(ack)
		if(ack == '2'):
			break
		#time.sleep(0.01)

	#time.sleep(0.001)
	#print("sending 3")

	ser.write('3')
	
	print("Handshake Complete\n")
		
