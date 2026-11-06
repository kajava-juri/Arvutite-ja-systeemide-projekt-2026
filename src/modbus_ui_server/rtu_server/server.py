"""
Modbus RTU UI Server.

Based on the RTU slave example from Modbus TestKit (modbus_tk):
 (C)2009 - Luc Jean - luc.jean@gmail.com
 (C)2009 - Apidev - http://www.apidev.fr
Distributed under the GNU LGPL, see LICENSE-LGPL.txt

Modified 2026 by Jüri Kajava: restructured into the ModbusRtuUIServer class,
added request logging hooks and register decoding via REGISTER_MAP.
"""

from collections.abc import Callable
from dataclasses import dataclass

import modbus_tk
import modbus_tk.defines as cst
from modbus_tk import modbus_rtu
from modbus_tk.hooks import install_hook
from modbus_tk.modbus import ModbusBlock
import serial
from .registers import REGISTER_MAP
import struct

@dataclass
class RegisterUpdate:
    name: str
    value: int | float | None
    unit: str

UpdateCallback = Callable[[RegisterUpdate], None]

def to_int16(raw: int) -> int:
    return raw - 0x10000 if raw & 0x8000 else raw

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
        self._name_by_offset = {reg.offset_addr: name for name, reg in REGISTER_MAP.items()}

        self._callbacks = []

        # install_hook("modbus_rtu.RtuServer.after_read", self.handle_raw_request);
        # install_hook("modbus.Slave.handle_write_single_register_request", self.handle_write_single_register)
        install_hook("modbus.ModbusBlock.setitem", self.handle_set_item)

        self.logger = modbus_tk.utils.create_logger(name="console", record_format="%(message)s")
        self.server = modbus_rtu.RtuServer(serial.Serial(self.port, baudrate=self.baudrate, parity=self.parity, bytesize=8, stopbits=self.stopbits),
                                           error_on_missing_slave=False)

    def handle_raw_request(self, args):
        """Full frame from the wire: [addr][PDU...][CRC lo][CRC hi]"""
        server, adu = args
        if len(adu) < 4:
            return  # partial/garbage frame
        addr = adu[0]
        pdu = adu[1:-2]
        self.logger.info(f"RX addr=0x{addr:02X} pdu={pdu.hex(' ')} crc={adu[-2:].hex(' ')}")

    def handle_write_single_register(self, args):
        slave, request_pdu = args
        func, reg_addr, value = struct.unpack(">BHH", request_pdu[:5])
        self.logger.info(
            f"FC06 slave={slave._id} (0x{slave._id:02X}) "
            f"pdu={request_pdu.hex(' ')} -> reg 0x{reg_addr:04X} = 0x{value:04X} ({value})"
        )

    def handle_set_item(self, fct: tuple[ModbusBlock, any, any]) -> None:
        (block, slice, value) = fct
        """Hook to handle after receiving a response from the Modbus master."""
        # self.logger.info(f"Received slice: {slice}, value: {value}")
        # self.logger.info(f"Block: {block.starting_address}, Size: {block.size}")

        register_name = self._name_by_offset.get(slice)
        if register_name is not None:
            reg_info = REGISTER_MAP[register_name]
            if (reg_info.signed):
                value = to_int16(value)
            scaled_value = value * reg_info.scale
            update = RegisterUpdate(name=register_name, value=scaled_value, unit=reg_info.unit)
            self._notify_callbacks([update])

    def get_registers(self, start_addr: int, num_regs: int) -> dict[str, tuple[int | float | None, str]]:
        """Get the current values of the registers in the specified range."""
        raw_values = self.server.get_slave(self.slave_id).get_values(self.block, start_addr, num_regs)
        return self.decode(raw_values)

    def start(self):
        try:
            self.logger.info("running...")

            self.server.start()

            slave = self.server.add_slave(self.slave_id)
            slave.add_block(self.block, cst.HOLDING_REGISTERS, self.start_addr, self.num_regs)

            # while True:
            #     raw = slave.get_values(self.block, self.start_addr, self.num_regs)
            #     # sys.stdout.write('done: values read: %s\r\n' % (str(raw)))
            #     decoded = self.decode(raw)
            #     sys.stdout.write('done: values read: %s\r\n' % (str(decoded)))
            #     time.sleep(1)

        except Exception as e:
            self.logger.error(e)

    def stop(self):
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

    def _notify_callbacks(self, updates: list[RegisterUpdate]):
        """Notify all subscribed callbacks with the provided updates."""
        for callback in self._callbacks:
            for update in updates:
                try:
                    callback(update)
                except Exception as e:
                    self.logger.error(f"Error in callback {callback}: {e}")

    def subscribe(self, callback: UpdateCallback):
        """Subscribe to register updates."""

        def unsubscribe():
            if callback in self._callbacks:
                self._callbacks.remove(callback)

        self._callbacks.append(callback)
        return unsubscribe