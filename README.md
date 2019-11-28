# agv
## Resumen
Código para la realización de un vehículo guiado automáticamente.

## Materiales
* ESP-32
* SonicDisc
* Puente H L298N
* Batería recargable de 12V
* [Chasis de tanque](https://www.amazon.com/-/es/ROB0153-Chasis-de-Gladiador-Negro/dp/B07RZP1H3R/ref=sr_1_30?__mk_es_US=%C3%85M%C3%85%C5%BD%C3%95%C3%91&keywords=arduino+tank&qid=1574921974&sr=8-30)

## GitHub SonicDisc
[SonicDisc](https://github.com/platisd/sonicdisc)
## Componentes SonicDisc
* [SonicDisc PCB](https://www.pcbway.com/project/shareproject/SonicDisc___A_360__ultrasonic_scanner__rev_1_.html)
* 8 x HC-SR04
* 1 x Atmega328P-PU
* 2 x 22pF capacitores
* 3 x 0.1uF capacitores
* 1 x 1206 LED
* 1 x 1206 220Ω resistencia
* 3 x 4.7KΩ resistencia
* 1 x botón
* 1 x 16MHz oscilador

## Conexión SonicDisc
| SonicDisc |   ESP-32  |
| :----:    |:----:     |
| VCC       |  VIN      |
| GND       |  GND      |
| SDA       |  SDA      |
| SCL       |  SCL      |
| INT       |  ISR      |
