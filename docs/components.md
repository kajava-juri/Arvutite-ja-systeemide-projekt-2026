Description of protocols, components choice, and modules used. For the overview refer to the [[README.md]],


**RS485 IC transceiver modules:**

| Transceiver | Duplex | Module | Vendor | Price | Link |
|---|---|---|---|---|---|
| [MAX485](https://www.analog.com/en/products/max485.html) | half-duplex | USB | satkit | 2,45 € | [link](https://satkit.ee/usb-rs485-muundur-plc-jaoks-max485?search=USB-RS485%20muundur%20PLC%20jaoks%20Max485&category_id=0) |
| [MAX485](https://www.analog.com/en/products/max485.html) | half-duplex | USB | oomipood | 6,00 € | [link](https://www.oomipood.ee/en/product/oky3406_6_usb_rs485_uleminek_kuni_6mbps_1200m?ref=product_also_bought) |
| [MAX485](https://www.analog.com/en/products/max485.html) | half-duplex | serial | satkit | | [link](https://satkit.ee/max485-ttl-rs485-adapter-arduino-ja-raspberry-pi-jaoks?search=USB-RS485%20muundur%20PLC%20jaoks%20Max485&category_id=0) |
| [MAX485](https://www.analog.com/en/products/max485.html) | half-duplex | serial | | | |
| [SN75176](https://www.ti.com/lit/ds/symlink/sn75176a.pdf?ts=1789015653049) | | | | | |
| SP3485 | | serial | waveshare | | [link](https://www.waveshare.com/wiki/Pico-2CH-RS485) |

## Fan / Actuator


**Pin configurations:**
- **2-pin** - power, ground
- **3-pin** - power, ground, tach (RPM feedback).
- **4-pin** - adds PWM control pin.

| Fan | Voltage | Pins  | Control | Static pressure | Vendor | Cheapest Price | Link | Link 2 |
| --- | ------- | ----- | ------- | --------------- | ------ | -------------- | ---- | ------ |
|     | 12 V    | 4-pin | PWM     |                 |        |                |      |        |
|     | 24 V    | 4-pin | PWM     |                 |        |                |      |        |
|     | 12 V    | 3-pin | voltage |                 |        |                |      |        |
|     | 24 V    | 3-pin | voltage |                 |        |                |      |        |
|     | 12 V    | 2-pin | voltage |                 |        |                |      |        |
|     | 24 V    | 2-pn  | voltage |                 |        |                |      |        |
