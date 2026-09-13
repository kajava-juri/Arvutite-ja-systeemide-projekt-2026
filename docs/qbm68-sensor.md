
## Interfaces

- **Analog** - voltage/current output proportional to pressure (0-10 V / 4-20 mA)
- **Modbus RTU (RS-485)** - digital read of pressure value from register

### Modbus RS-485

Modbus RS-485 uses differential signaling over twisted pair. For the differential signal to perform better, a [twisted pair](https://en.wikipedia.org/wiki/Twisted_pair) wiring is used. 
- data rate is up to 10 Mbit/s
	- and at lower speeds, distances can reach up to 1200 meters.[^rs485] 
- As a rule of thumb, the speed in bit/s multiplied by the length in meters should not exceed $10^8$. Thus, a 50-meter cable should not signal faster than 2 Mbit/s.

#### Bus settings

write here the bitrate, slave address and other info regarding the communication bus

#### Register map

For now refer to the datasheet [994020910_911913 N1910en1_QBM68xx.pdf](datasheets/994020910_911913_N1910en1_QBM68xx.pdf)

### Reading over analog

info on how to read over analog, for now refer to the datasheet [994020910_911913 N1910en1_QBM68xx.pdf](datasheets/994020910_911913_N1910en1_QBM68xx.pdf)


---
[^rs485]: [Wikipedia: RS-485](https://en.wikipedia.org/wiki/RS-485)
