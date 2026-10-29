#!/usr/bin/env python
# -*- coding: utf_8 -*-
"""
 Modbus TestKit: Implementation of Modbus protocol in python

 (C)2009 - Luc Jean - luc.jean@gmail.com
 (C)2009 - Apidev - http://www.apidev.fr

 This is distributed under GNU LGPL license, see license.txt
"""

# for simulating and testing: replace <user> with your username that has permission to access the port
# udo socat -d -d PTY,rawer,link=/dev/ttyS10,user=<user> PTY,rawer,link=/dev/ttyS11

from rtu_server.server import ModbusRtuUIServer
import serial



PORT = '/dev/ttyS10' # 'COMXXX' on Windows
# PORT = None
BAUDRATE = 9600
PARITY = serial.PARITY_NONE
STOPBITS = 2

SLAVE_ID = 10
BLOCK = 'ui'
START_ADDR = 0
NUM_REGS = 20

if __name__ == "__main__":
    rtu_server = ModbusRtuUIServer(
        port=PORT,
        slave_id=SLAVE_ID,
        block=BLOCK,
        start_addr=START_ADDR,
        num_regs=NUM_REGS,
        baudrate=BAUDRATE,
        parity=PARITY,
        stopbits=STOPBITS
    )
    rtu_server.start()