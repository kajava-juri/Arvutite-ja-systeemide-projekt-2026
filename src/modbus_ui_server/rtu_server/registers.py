from dataclasses import dataclass

@dataclass
class Register:
    offset_addr: int
    scale: float
    signed: bool
    unit: str

REGISTER_MAP: dict[str, Register] = {
    "pressure": Register(offset_addr=0, scale=1, signed=True, unit="Pa"),
    "setpoint": Register(offset_addr=1, scale=1, signed=True, unit="Pa"),
}