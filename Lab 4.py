#***********************************************************************************************************
#  *File Name           : Lab 4.py
#  * Description        : this program will read the data from serial port with which stm 303 is connected.
#  *                      then we will seperate data using ',' and then we will post required data into database.
#  *                      after that we will write get request to read data from database and then we will
#  *                      delete requeat to delete data from database.
#  * 
#  * Author             : Abhisha Bhesaniya              
#  * Date               : 24th March, 2020			 
#  **********************************************************************************************************

# include Library
import serial
import time
import requests

serialport = serial.Serial('/dev/ttyACM1',115200) #set the USB port, budrate and other stuff here 
print(serialport.name) # print the name of port you are connected with
serialport.reset_input_buffer() #reset the input buffer

while 1:
       
       out = serialport.readline() #read data untill line ends
       print(out) #print the received data
       
       fout = out.decode().split(",", 5) #split the data using ',' five times
       
       data={'No Of Sample':fout[1], 'temp':fout[2],} # make the formate of data you want to post in to database
       
       requests.put('http://esdserver:3000/api/c520d388d3438a8b12053f62c0deef6f/data',json=data) #put request
       r=requests.get('http://esdserver:3000/api/c520d388d3438a8b12053f62c0deef6f/data') #get request
       print(r.json())#print data retrived from database
       
       print() # just to add blank line
       print()
       
       requests.delete('http://esdserver:3000/api/c520d388d3438a8b12053f62c0deef6f/data') # delete request
       r=requests.get('http://esdserver:3000/api/c520d388d3438a8b12053f62c0deef6f/data')  # get request
       print(r.json()) #print data retrived from database
       
       time.sleep(1) #delay of 1 sec
       
