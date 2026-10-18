## Fan switch: FQP30N06L
 
Load: 12 V fan, 0.1 A, PWM 25 kHz.
V_th = 1.0 ... 2.5 V (datasheet)    R_g = 220 Ohm    
R_thJA = 62.5 C/W (TO-220)          T_j(max) = 175 C
(R_DS(on)) = 35 mOhm (from datasheet)

V_load = V_supply - (I_load-R_DS(on)) 
V_load = 12 V - 0.0035 V = 11.9965 V 
 
### 1. Switching time
```
Total Gate Charge Qg: 15 nC
Ig(peak) = 3.3V/220 = 15mA
Ig(average) = 15ma/2 = 7.5mA
Switching time(on) = 15nC/7.5mA = 2 us
Switching time(off) = 15nC/7.5mA = 2 us
```
Compared to the 25 kHz PWM period (T = 40 us):
```
Total switching time = 2 us + 2 us = 4 us
PWM time % = 4 us / 40 us = 10%
```
### 4. Power dissipation
 
Switching loss:
 
```
P_sw = 0.5 * V_DS * I_D * (t_on + t_off) * f
     = 0.5 * 12 V * 0.1 A * 4 us * 25 kHz = 60 mW
```
 
Conduction loss (R_DS(on)) 
 
```
P_cond = I_D^2 * R_DS(on) = (0.1 A)^2 * 35 mOhm = 0.35mW
 ```
Total transistor dissipation:
 
```
P_d = P_sw + P_cond = 60 + 0.35 = 60.35 mW
```
 
### 5. Thermal calculation
 
```
T_j = T_a + P_d * R_thJA
25 C + (60.35 mW * 62.5 C/W) = 28.77 C
```
 
Maximum allowed power at a given ambient temperature:
 
```
P_max = (T_j(max) - T_a) / R_thJA
P_max(25 C) = (175 - 25) / 62.5 = 2.4 W
P_max(60 C) = (175 - 60) / 62.5 = 1.84 W
```
  so no heatsink is needed
 

