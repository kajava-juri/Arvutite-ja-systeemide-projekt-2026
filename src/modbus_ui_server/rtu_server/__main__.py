#!/usr/bin/env python
# -*- coding: utf_8 -*-

# for simulating and testing: replace <user> with your username that has permission to access the port
# sudo socat -d -d PTY,rawer,link=/dev/ttyS10,user=<user> PTY,rawer,link=/dev/ttyS11

from rtu_server.server import ModbusRtuUIServer
import serial
import argparse


PORT = '/dev/ttyUSB0' # 'COMXXX' on Windows
# PORT = None
BAUDRATE = 9600
PARITY = serial.PARITY_NONE
STOPBITS = 2

SLAVE_ID = 10
BLOCK = 'ui'
START_ADDR = 0
NUM_REGS = 20

def parity_from_string(parity_str):
    if parity_str == 'none':
        return serial.PARITY_NONE
    elif parity_str == 'even':
        return serial.PARITY_EVEN
    elif parity_str == 'odd':
        return serial.PARITY_ODD
    else:
        raise ValueError(f"Invalid parity value: {parity_str}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("-p", "--port", help="Serial port to use (default: /dev/ttyUSB0)", default=PORT)
    parser.add_argument("-b", "--baudrate", help="Baud rate (default: 9600)", type=int, default=BAUDRATE)
    parser.add_argument("--parity", help="Parity (default: none)", choices=['none', 'even', 'odd'], default='none')
    parser.add_argument("--stopbits", help="Stop bits (default: 2)", type=int, choices=[1, 2], default=STOPBITS)
    parser.add_argument("--slave-id", help="Slave ID (default: 10)", type=int, default=SLAVE_ID)

    args = parser.parse_args()

    rtu_server = ModbusRtuUIServer(
        port=args.port,
        slave_id=args.slave_id,
        block=BLOCK,
        start_addr=START_ADDR,
        num_regs=NUM_REGS,
        baudrate=args.baudrate,
        parity=parity_from_string(args.parity),
        stopbits=args.stopbits
    )
    rtu_server.start()