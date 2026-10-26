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

import sys

import modbus_tk
import modbus_tk.defines as cst
from modbus_tk import modbus_rtu
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

def main():
    """main"""
    logger = modbus_tk.utils.create_logger(name="console", record_format="%(message)s")

    #Create the server
    server = modbus_rtu.RtuServer(serial.Serial(PORT, baudrate=BAUDRATE, parity=PARITY, bytesize=8, stopbits=STOPBITS))

    try:
        logger.info("running...")
        logger.info("enter 'quit' for closing the server")

        server.start()

        slave_1 = server.add_slave(SLAVE_ID)
        slave_1.add_block('ui', cst.HOLDING_REGISTERS, START_ADDR, NUM_REGS)
        while True:
            cmd = sys.stdin.readline()
            args = cmd.split(' ')

            if cmd.find('quit') == 0:
                sys.stdout.write('bye-bye\r\n')
                break

            elif args[0] == 'set_values':
                slave_id = SLAVE_ID
                address = int(args[1])
                values = []
                for val in args[2:]:
                    values.append(int(val))
                slave = server.get_slave(slave_id)
                slave.set_values(BLOCK, address, values)
                values = slave.get_values(BLOCK, address, len(values))
                sys.stdout.write('done: values written: %s\r\n' % (str(values)))

            elif args[0] == 'get_values':
                slave_id = SLAVE_ID
                address = int(args[1])
                length = int(args[2])
                slave = server.get_slave(slave_id)
                values = slave.get_values(BLOCK, address, length)
                sys.stdout.write('done: values read: %s\r\n' % (str(values)))

            else:
                sys.stdout.write("unknown command %s\r\n" % (args[0]))
    finally:
        server.stop()

if __name__ == "__main__":
    main()