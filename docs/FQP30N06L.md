## Fan switch: FQP30N06L
 
Load: 12 V fan, 0.1 A, PWM 25 kHz.
 
Assumptions (values read from datasheet graphs, accuracy about ±30 %):
 
```
V_th = 1.0 ... 2.5 V (datasheet)    R_g = 220 Ohm    GPIO drive = 4 mA (default)
R_thJA = 62.5 C/W (TO-220)          T_j(max) = 175 C
```
 
### 1. Gate drive check
 
Miller plateau level at 0.1 A (square-law model, K(process transconductance parameter) fitted from the 32 A gate charge curve, V_th = 2 V):
 
```
I_D = K (VGS − Vth)² so:
K = I_D / (V_plateau - V_th)^2 = 32 A / (3.9 V - 2 V)^2 = 8.9 A/V^2
V_plateau = V_th + sqrt(I_D / K) = 2 V + sqrt(0.1 A / 8.9 A/V^2) = 2.1 V
```
 
### 2. Gate charge
 
Capacitances from the datasheet graph (V_GS = 0 V, 1 MHz):
 
```
C_gs = C_iss - C_rss = 1320 pF - 570 pF = ~740 pF   (almost constant over V_DS)
```
 
Charge up to the plateau:
 
```
Q_gs = C_gs * V_plateau = 0.74 nF * 2.1 V = 1.5 nC
```
 
Miller charge:
 
```
Q_gd = integral(C_gd dV_DS) = 3.6 nC
```
 
Charge from the plateau to 3.3 V (V_DS is ~0, C_iss = 1.75 nF):
 
```
Q_post = C_iss * (3.3 V - 2.1 V) = 1.75 nF * 1.2 V = 2.1 nC
```
 
Total:
 
```
Q_g(3.3 V) = Q_gs + Q_gd + Q_post = 1.5 + 3.6 + 2.1 = ~7 nC
```
 
### 3. Switching time
 
Gate current is limited by the GPIO, not by R_g (3.3 V / 220 Ohm = 15 mA would be more than the pin delivers):
Full gate charge time (rough): 


```
I_g(on) = (V_drive - V_plateau) / R_g = (3.3 V - 2.1 V) / 220 Ohm = 5.45 mA
I_g(off) = V_plateau / R_g = 2.1 V / 220 Ohm = 9.5 mA
t_on = Q_gd / I_g(on) = 3.6 nC / 5.45 mA = 0.66 us
t_off = Q_gd / I_g(off) = 3.6 nC / 9.5 mA = 0.38 us

```
Ig peak time 

```
t ~ R_g * (C_gs + C_gd) ~ 220 Ohm * ~0.9 nF ~ 0.2 us
 ```
Compared to the PWM period:
 
```
Full gate charge time (rough):
t_sw = Q_g / I_g(on) = 7 nC / 5.45 mA = 1.29 us
T = 1 / 25 kHz = 40 us
t_sw / T = 1.29 us / 40 us = 3.2 %
t_gate(off) = Q_g / I_g(off) = 7 nC / 9.5 mA = 0.74 us
t_gate(off) / T = 0.74 us / 40 us = 1.8 %
t_sw(total)=1.8+3.2~5%
```
### 4. Power dissipation
 
Switching loss:
 
```
P_sw = 0.5 * V_DS * I_D * (t_on + t_off) * f
     = 0.5 * 12 V * 0.1 A * 1.04 us * 25 kHz = 15.6 mW
```
 
Conduction loss (R_DS(on) 
 
```
P_cond = I_D^2 * R_DS(on) = (0.1 A)^2 * 35 mOhm = 0.35mW
 ```
Total transistor dissipation:
 
```
P_d = P_sw + P_cond = 15.6 + 0.35 = ~15.95 mW
```
 
### 5. Thermal calculation
 
```
T_j = T_a + P_d * R_thJA
25 C + 15.95 mW * 62.5 C/W = 25.9 C
```
 
Maximum allowed power at a given ambient temperature:
 
```
P_max = (T_j(max) - T_a) / R_thJA
P_max(25 C) = (175 - 25) / 62.5 = 2.4 W
P_max(60 C) = (175 - 60) / 62.5 = 1.84 W
```
  so no heatsink is needed
 

