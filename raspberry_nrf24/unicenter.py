#!/usr/bin/python
# raspberry pi nrf24l01 hub
# more details at http://blog.riyas.org
# Credits to python port of nrf24l01, Joao Paulo Barrac & maniacbugs original c library
# ======================= changes on may 1,2015=======================
#added csv write..and parses multiple data elements (Temp, press, humidity)
#result goes to a file called temp.csv in the same place as this file

from nrf24 import NRF24
import time
from toinflux import updatedata
from time import gmtime, strftime
from datetime import datetime
ISOTIMEFORMAT="%Y-%m-%dT%XZ"



pipes = [[0xf0, 0xf0, 0xf0, 0xf0, 0xe1], [0xf0, 0xf0, 0xf0, 0xf0, 0xd2]]

radio = NRF24()
radio.begin(0, 0,25,18) #set gpio 25 as CE pin
radio.setRetries(15,15)
radio.setPayloadSize(32)
radio.setChannel(0x4c)
radio.setDataRate(NRF24.BR_250KBPS)
radio.setPALevel(NRF24.PA_MAX)
radio.setAutoAck(1)
radio.openWritingPipe(pipes[0])
radio.openReadingPipe(1, pipes[1])
radio.openReadingPipe(2, [0xf0,0xf0,0xf0,0xc3])
radio.startListening()
radio.stopListening()

radio.printDetails()
radio.startListening()


def extract(raw_string, start_marker, end_marker):
    start = raw_string.index(start_marker) + len(start_marker)
    end = raw_string.index(end_marker, start)
    return raw_string[start:end]
while True:
    pipe = [0]
    while not radio.available(pipe, True):
        time.sleep(1000/1000000.0)
    recv_buffer = []
    radio.read(recv_buffer)
    out = ''.join(chr(i) for i in recv_buffer)
    now=time.strftime( ISOTIMEFORMAT, time.localtime() )
    if "T" in out:
    	temp=extract(out,'T','T')
    	hum=extract(out,'H','H')
    	pm25=extract(out,'S','S')
    	pm10=extract(out,'B','B')
	updatedata(now,'temp_info','1',temp)
	updatedata(now,'hum_info','1',hum)
	updatedata(now,'pm25_info','1',pm25)
	updatedata(now,'pm10_info','1',pm10)
    elif "G" in out:
    	gas=extract(out,'G','G')
    	co=extract(out,'C','C')
	updatedata(now,'gas_info','1',gas)
	updatedata(now,'co_info','1',co)
