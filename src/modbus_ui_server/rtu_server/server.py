import sys

import modbus_tk
import modbus_tk.defines as cst
from modbus_tk import modbus_rtu
from modbus_tk.hooks import install_hook
from modbus_tk.modbus import ModbusBlock
import serial
from .registers import REGISTER_MAP
import time

class ModbusRtuUIServer:
    def __init__(self, port, slave_id, block, start_addr, num_regs, baudrate=9600, parity=serial.PARITY_NONE, stopbits=2):
        self.port = port
        self.baudrate = baudrate
        self.parity = parity
        self.stopbits = stopbits
        self.slave_id = slave_id
        self.block = block
        self.start_addr = start_addr
        self.num_regs = num_regs

        install_hook("modbus.ModbusBlock.setitem", lambda fct: self.handle_set_item(fct))

        self.logger = modbus_tk.utils.create_logger(name="console", record_format="%(message)s")
        self.server = modbus_rtu.RtuServer(serial.Serial(self.port, baudrate=self.baudrate, parity=self.parity, bytesize=8, stopbits=self.stopbits))

    def handle_set_item(self, fct: tuple[ModbusBlock, any, any]) -> None:
        (block, slice, value) = fct
        """Hook to handle after receiving a response from the Modbus master."""
        self.logger.info(f"Received slice: {slice}, value: {value}")
        self.logger.info(f"Block: {block.starting_address}, Size: {block.size}")

    def start(self):
        try:
            self.logger.info("running...")
            self.logger.info("enter 'quit' for closing the server")

            self.server.start()

            slave = self.server.add_slave(self.slave_id)
            slave.add_block(self.block, cst.HOLDING_REGISTERS, self.start_addr, self.num_regs)

            while True:
                # raw = slave.get_values(self.block, self.start_addr, self.num_regs)
                # sys.stdout.write('done: values read: %s\r\n' % (str(raw)))
                # decoded = self.decode(raw)
                # sys.stdout.write('done: values read: %s\r\n' % (str(decoded)))
                time.sleep(1)

        except Exception as e:
            self.logger.error(e)
        finally:
            if self.server.is_alive():
                self.server.stop()

    def decode(self, raw_values):
        """Decode raw values from the Modbus server into a dictionary of register names and their corresponding values."""
        decoded_values = {}
        for name, register in REGISTER_MAP.items():
            offset = register.offset_addr
            scale = register.scale
            signed = register.signed
            unit = register.unit

            if offset < len(raw_values):
                raw_value = raw_values[offset]
                if signed:
                    # Convert to signed integer if necessary
                    if raw_value >= 32768:
                        raw_value -= 65536
                decoded_value = raw_value * scale
                decoded_values[name] = (decoded_value, unit)
            else:
                decoded_values[name] = (None, unit)  # Handle out-of-bounds access

        return decoded_values