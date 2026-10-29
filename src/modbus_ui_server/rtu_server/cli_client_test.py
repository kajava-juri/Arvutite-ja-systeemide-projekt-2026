#!/usr/bin/env python
# -*- coding: utf_8 -*-
"""
 Modbus TestKit: Implementation of Modbus protocol in python

 (C)2009 - Luc Jean - luc.jean@gmail.com
 (C)2009 - Apidev - http://www.apidev.fr

 This is distributed under GNU LGPL license, see license.txt
"""

import serial
import sys

import modbus_tk
import modbus_tk.defines as cst
from modbus_tk import modbus_rtu

PORT = '/dev/ttyS11' # 'COMXXX' on Windows
# PORT = None
BAUDRATE = 9600
PARITY = serial.PARITY_NONE
STOPBITS = 2

SLAVE_ID = 10
BLOCK = 'ui'
START_ADDR = 0
NUM_REGS = 20
#PORT = '/dev/ttyp5'

def main():
    """main"""
    logger = modbus_tk.utils.create_logger("console")

    try:
        #Connect to the slave
        master = modbus_rtu.RtuMaster(
            serial.Serial(PORT, baudrate=BAUDRATE, parity=PARITY, bytesize=8, stopbits=STOPBITS)
        )
        master.set_timeout(5.0)
        master.set_verbose(True)
        logger.info("connected")

        logger.info(master.execute(SLAVE_ID, cst.READ_HOLDING_REGISTERS, 0, 3))

        #send some queries
        #logger.info(master.execute(1, cst.READ_COILS, 0, 10))
        #logger.info(master.execute(1, cst.READ_DISCRETE_INPUTS, 0, 8))
        #logger.info(master.execute(1, cst.READ_INPUT_REGISTERS, 100, 3))
        #logger.info(master.execute(1, cst.READ_HOLDING_REGISTERS, 100, 12))
        #logger.info(master.execute(1, cst.WRITE_SINGLE_COIL, 7, output_value=1))
        # logger.info(master.execute(SLAVE_ID, cst.WRITE_SINGLE_REGISTER, 0, output_value=54))
        #logger.info(master.execute(1, cst.WRITE_MULTIPLE_COILS, 0, output_value=[1, 1, 0, 1, 1, 0, 1, 1]))
        #logger.info(master.execute(1, cst.WRITE_MULTIPLE_REGISTERS, 100, output_value=xrange(12)))

        while True:
            cmd = sys.stdin.readline()
            args = cmd.split(' ')

            if cmd.find('quit') == 0:
                sys.stdout.write('bye-bye\r\n')
                break

            elif args[0] == 'write_single':
                slave_id = SLAVE_ID
                address = int(args[1])
                value = int(args[2])
                logger.info(master.execute(slave_id, cst.WRITE_SINGLE_REGISTER, address, output_value=value))

    except modbus_tk.modbus.ModbusError as exc:
        logger.error("%s- Code=%d", exc, exc.get_exception_code())

if __name__ == "__main__":
    main()