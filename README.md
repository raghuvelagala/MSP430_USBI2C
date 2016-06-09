# MSP430_USBI2C

Contains code for retrieving data (Temperature, Pressure, Humidity, Lux) from the appropriate sensors and
displays it on a an LCD through I2C while also sending data through USB to a Python app on the PC for plotting. 

	- Resources/lab8 - Contains project files for use with Code Composer Studio (Eclipse based IDE) for
	                   executing the code on thr microcontroller (used examples from MSP430 Tutorials).
	- lab8.py  - This is the python application for the PC to parse data recieved through USB and 
	             for plotting it using the matplotlib library (Python 3.5).

