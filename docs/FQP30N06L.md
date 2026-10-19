## Fan switch: FQP30N06L
 
Load: 12 V fan, 0.1 A, PWM 25 kHz.

```math
\begin{aligned}
V_{supply} &= 12\ \text{V} \\
V_{drive} &= 3.3\ \text{V} \\
I_{load} &= 0.1\ \text{A} \\
f_{PWM} &= 25\ \text{kHz} \\
V_{th} &= 1.0 \ldots 2.5\ \text{V} \quad \text{(datasheet)} \\
R_g &= 220\ \Omega \\
R_{DS(on)} &= 35\ \text{m}\Omega \quad \text{(from datasheet)} \\
R_{thJA} &= 62.5\ ^\circ\text{C/W} \quad \text{(TO-220)} \\
T_{j(max)} &= 175\ ^\circ\text{C}
\end{aligned}
```
---

$$V_{load} = V_{supply} - I_{load} \cdot R_{DS(on)}$$
$$V_{load} = 12\ \text{V} - 0.0035\ \text{V} = 11.9965\ \text{V}$$


### 1. Switching time

**Parameters:**

$$Q_g = 15\ \text{nC}$$

$$I_{g(peak)} = \frac{V_{drive}}{R_g} = \frac{3.3\ \text{V}}{220\ \Omega} = 15\ \text{mA}$$

**Average:** rough estimation assuming linear change in current during siwtching.

$$I_{g(avg)} = \frac{I_{g(peak)}}{2} = \frac{15\ \text{mA}}{2} = 7.5\ \text{mA}$$

$$t_{on} = \frac{Q_g}{I_{g(avg)}} = \frac{15\ \text{nC}}{7.5\ \text{mA}} = 2\ \mu\text{s}$$

$$t_{off} = \frac{Q_g}{I_{g(avg)}} = \frac{15\ \text{nC}}{7.5\ \text{mA}} = 2\ \mu\text{s}$$

---

**Period:**
$$T = \frac{1}{f} = \frac{1}{25\ \text{kHz}} = 40\ \mu\text{s}$$

**Total switching time:**
$$t_{sw} = t_{on} + t_{off} = 2\ \mu\text{s} + 2\ \mu\text{s} = 4\ \mu\text{s}$$

**Fraction of the PWM period spent on switching:**
$$\frac{t_{sw}}{T} = \frac{4\ \mu\text{s}}{40\ \mu\text{s}} = 10\%$$


### 4. Power dissipation
 
**Switching loss:**

$$\begin{aligned}
P_{sw} &= \frac{1}{2} \cdot V_{DS} \cdot I_D \cdot (t_{on} + t_{off}) \cdot f \\
&= 0.5 \cdot 12\ \text{V} \cdot 0.1\ \text{A} \cdot 4\ \mu\text{s} \cdot 25\ \text{kHz} = 60\ \text{mW}
\end{aligned}$$

Lets define it as a function $p_s(x,y)$: $$p_s(x, y) = 0.5 \cdot x \cdot y \cdot 4\mu s \cdot 25 \text{ kHz}$$

 
Conduction loss ($R_{DS(on)}$) 

$$P_{cond} = I_D^2 \cdot R_{DS(on)} = (0.1\ \text{A})^2 \cdot 35\ \text{m}\Omega = 0.35\ \text{mW}$$

lets define second function $p_c(x)$:

$$p_c(x) = x^2 \cdot 35\ \text{m}\Omega$$


$$P_d = P_{sw} + P_{cond} = 60\ \text{mW} + 0.35\ \text{mW} = 60.35\ \text{mW}$$

in form of functions:
$$p_s(V, I) + p_c(I) = p_t$$

#### Maximum current as a function of voltage

$p_s(x,y)$ can be simplified: $4\mu s \cdot 25 \text{ kHz} = 0.1$

$$0.5 \cdot x \cdot y \cdot 0.1 = 0.05 \cdot x \cdot y = p_s(x,y)$$

substituing into $p_t$ and replacing $y=\text{V}$ and $x = \text{I}$

$$0.05 \cdot I \cdot V + 0.035\ \Omega \cdot I^2 = p_t$$

This is the quadratic relative to $I$:
$$I_{max} = \frac{-0.05\,V + \sqrt{(0.05\,V)^2 + 4 \cdot 0.035\ \Omega \cdot P_{max}}}{2 \cdot 0.035\ \Omega}$$


For $V = 12 \text{ V}$

$$I_{max}(25\ ^\circ\text{C}) = \frac{-0.6 + \sqrt{0.36 + 0.14 \cdot 2.4}}{0.07} \approx 3.35\ \text{A}$$

$$I_{max}(60\ ^\circ\text{C}) = \frac{-0.6 + \sqrt{0.36 + 0.14 \cdot 1.84}}{0.07} \approx 2.66\ \text{A}$$


### 5. Thermal calculation

$$\begin{aligned}
T_j &= T_a + P_d \cdot R_{thJA} \\
&= 25\ ^\circ\text{C} + 60.35\ \text{mW} \cdot 62.5\ ^\circ\text{C/W} = 28.77\ ^\circ\text{C}
\end{aligned}$$

 
Maximum allowed power at a given ambient temperature:

$$P_{max} = \frac{T_{j(max)} - T_a}{R_{thJA}}$$
$$P_{max}(25\ ^\circ\text{C}) = \frac{175 - 25}{62.5} = 2.4\ \text{W}$$

$$P_{max}(60\ ^\circ\text{C}) = \frac{175 - 60}{62.5} = 1.84\ \text{W}$$


**so no heatsink is needed**
 

