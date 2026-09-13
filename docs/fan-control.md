
The actuator in the pressure control loop. Common computer fans (12/24 V).

**Control methods:**
- **Voltage control** - regulate supply voltage to set fan speed. Works on 3-pin fans.
- **PWM control** - ~25 kHz logic-level PWM to a 4-pin fan's control pin. PWM pin is usually 5 V logic.

**Pin configurations:**
- **2-pin** - power, ground
- **3-pin** - power, ground, tach (RPM feedback).
- **4-pin** - adds PWM control pin.