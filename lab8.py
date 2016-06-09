#!/usr/bin/python
##########################################
# Copyright 2014 George Mason University #
##########################################
import os
import serial
import time
import sys
import numpy
import matplotlib.pyplot as plt


''' GLOBAL VARIABLES '''
comPort = 'COM8' #change value to actual comPort Example -> 'COM6'
data1 = ['1', '2', '3', '4', '5', '6']
data2 = ['6', '5', '4', '3', '2', '1']
snapShotFile = "lab8Output.png"
snapShotFile = "I2C_Pressure.png"

''' Clears Screen '''
def clearScreen():
  os.system( [ 'clear', 'cls' ][ os.name == 'nt' ] )
  
''' comPort: comport to connect to returns: PySerial object '''
def serialConnect(comPort):
	sys.stdout.write("\n\tOpening Serial Port for Communication\n")
	ser = serial.Serial(comPort, baudrate= 9600, timeout=0)
	ser.close()
	ser.open()
	return ser

''' hexString: string from MSP430 returns: byte array '''
def convertToByteArray(hexString):
	#hexString = ''.join( hexString.split(" ") )
	hexBytes = []
	for dataByte in range(0, len(hexString), 2):
		hexBytes.append( hexString[dataByte:dataByte+2])		
	return hexBytes

''' dataToPlot: data from sensors. Each row corresponds to a sensor. Each column corresponds to value fro
the sensor returns: plot output in png file '''
	
def plotTrace(dataToPlot, plotType, outputFilename):
	figs = plt.figure()	
	figs.suptitle('Measured Data', fontsize=14, fontweight='bold')		
	plt.hold(False)
	plt.clf()
	tiledPlotCounter = 0
	dataToPlot = numpy.transpose(dataToPlot)
	tiledPlotCounter = 0
	temp, tracesToPrint = numpy.shape(dataToPlot)
	if(plotType == 'OVERLAY'):
		sys.stdout.write("\tPlotting data - Overlayed Format\n")	
		plt.plot(dataToPlot)
		plt.ylabel('Sensor Value')
		plt.xlabel('Time')
		plt.title('Processed Data')
		plt.grid(b=True, which='Major', color = 'b', linestyle = '--')
	elif(plotType == 'TILED'):
		sys.stdout.write("\tPlotting traces - Tiled Format\n")
		fig,axes = plt.subplots(tracesToPrint, 1, sharex= True, sharey = True)
		fig.text(0.54, 0.02, 'Time', ha='center', va='center', fontsize=14)
		#fig.text(0.02, 0.53, 'Value', ha='center', va='center', rotation='vertical', fontsize=14)
		plt.tight_layout()

		if (tracesToPrint == 1):
			axes.plot(dataToPlot)
		else:	
			for traceNo in range(0, tracesToPrint):
				axes[tiledPlotCounter].plot(dataToPlot[:,int(traceNo)])
				ylabel = "Sensor " + str(tiledPlotCounter+1)
				axes[tiledPlotCounter].set_ylabel(ylabel)
				plt.setp(axes[tiledPlotCounter].get_xticklabels(), visible=True)
				plt.setp(axes[tiledPlotCounter].get_yticklabels(), visible=True)				
				tiledPlotCounter += 1
			tiledPlotCounter -= 1
			plt.setp(axes[tiledPlotCounter].get_xticklabels(), visible=True)
	
	sys.stdout.write("\tSaving to %s file\n" % outputFilename)
	plt.savefig(outputFilename,dpi=100)
		
def main():
	clearScreen()
	ser = serialConnect(comPort)
	
	#I2C Pressure output
	ser.write(b'SENSOR_1\r')
	time.sleep(1)
	out = None
	while ser.inWaiting() > 0:
		out=ser.read(100) # Why 100? Should this also be user specified?
	if out == None:
		sys.stdout.write("\tNo output received from MSP430\n")
		sys.exit(1)		
	out= convertToByteArray(out)
	out = [float(x) for x in out]
	print(out) # Float Array
	dataToPlot = []
	dataToPlot.append(out)
	#dataToPlot = numpy.vstack((dataToPlot, data2))
	print(dataToPlot)
	plotTrace(dataToPlot, 'OVERLAY', "I2C_Pressure.png")
	print('\n')
	
	#I2C Temperature Output
	ser.write(b'SENSOR_2\r')
	time.sleep(1)
	out = None
	while ser.inWaiting() > 0:
		out=ser.read(100) # Why 100? Should this also be user specified?
	if out == None:
		sys.stdout.write("\tNo output received from MSP430\n")
		sys.exit(1)		
	out= convertToByteArray(out)
	out = [float(x) for x in out]
	print(out) # Float Array
	dataToPlot = []
	dataToPlot.append(out)
	#dataToPlot = numpy.vstack((dataToPlot, data2))
	print(dataToPlot)
	plotTrace(dataToPlot, 'OVERLAY', "I2C_Temperature.png")
	print('\n')
	
	#ADC LUX output
	ser.write(b'SENSOR_3\r')
	time.sleep(1)
	out = None
	while ser.inWaiting() > 0:
		out=ser.read(100) # Why 100? Should this also be user specified?
	if out == None:
		sys.stdout.write("\tNo output received from MSP430\n")
		sys.exit(1)		
	out= convertToByteArray(out)
	out = [float(x) for x in out]
	print(out) # Float Array
	dataToPlot = []
	dataToPlot.append(out)
	#dataToPlot = numpy.vstack((dataToPlot, data2))
	print(dataToPlot)
	plotTrace(dataToPlot, 'OVERLAY', "ADC_LUX.png")
	print('\n')
	
	#ADC Temp. (MSP430) output
	ser.write(b'SENSOR_4\r')
	time.sleep(1)
	out = None
	while ser.inWaiting() > 0:
		out=ser.read(100) # Why 100? Should this also be user specified?
	if out == None:
		sys.stdout.write("\tNo output received from MSP430\n")
		sys.exit(1)		
	out= convertToByteArray(out)
	out = [float(x) for x in out]
	print(out) # Float Array
	dataToPlot = []
	dataToPlot.append(out)
	#dataToPlot = numpy.vstack((dataToPlot, data2))
	print(dataToPlot)
	plotTrace(dataToPlot, 'OVERLAY', "ADC(MSP430)_TEMP.png")
	print('\n')

if __name__ == '__main__':
	main()	
