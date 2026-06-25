# Hardware

The hardware files describe a custom controller board for the NATCAR vehicle:

```text
hardware/Natcar 1.01.sch
hardware/Natcar 1.01.brd
```

## Major Components

| Component | Purpose |
| --- | --- |
| Teensy 3.1-compatible controller footprint | Main microcontroller / Arduino-compatible firmware target |
| 5V regulator | Servo and board-level 5V supply |
| 3.3V regulator | Microcontroller, analog front end, and low-voltage electronics |
| MCP602 op-amps | Analog front-end conditioning for the inductive sensors |
| N-channel and P-channel MOSFETs | Left and right motor drive stages |
| Battery connector | Main power input |
| Servo connector | Steering servo interface |
| Left / center / right inductor connectors | Track guide sensing |
| Camera connector/header | Line-scan camera interface through `SI`, `CLK`, and analog output pins |

## Firmware Pin Map

The final firmware uses the following pin assignments:

| Signal | Firmware name | Pin |
| --- | --- | ---: |
| Camera analog output | `PIN_CAMERA_AO` | `22` |
| Camera clock | `PIN_CAMERA_CLK` | `20` |
| Camera start integration | `PIN_CAMERA_SI` | `21` |
| Steering servo | `PIN_SERVO` | `23` |
| Left inductor | `PIN_LEFT_INDUCTOR` | `19` |
| Center inductor | `PIN_CENTER_INDUCTOR` | `18` |
| Right inductor | `PIN_RIGHT_INDUCTOR` | `17` |
| Motor driver A | `PIN_PWM_A` | `4` |
| Motor driver B | `PIN_PWM_B` | `3` |
| Motor driver C | `PIN_PWM_C` | `9` |
| Motor driver D | `PIN_PWM_D` | `10` |

The inductor pins remain declared in the final sketch even though the active control path is camera-based. Earlier archived versions use the three inductor readings directly.

## Motor Drive

The schematic includes separate left and right motor connectors and MOSFET drive circuitry for each side. The firmware writes complementary PWM-style outputs:

```cpp
analogWrite(PIN_PWM_A, 0);
analogWrite(PIN_PWM_B, clampPwm(leftMotorPwm));
analogWrite(PIN_PWM_C, 0);
analogWrite(PIN_PWM_D, clampPwm(rightMotorPwm));
```

That indicates the car is driving both motors forward with speed set by `PIN_PWM_B` and `PIN_PWM_D` in the final code path.

## Sensor Front End

The left, center, and right inductive sensors connect into analog front-end circuitry built around MCP602 op-amps, resistors, capacitors, and diodes. The intent is to condition the raw pickup signals before reading them on analog-capable microcontroller pins.

The spreadsheets in `data/` capture measurements used to understand and scale these sensor responses.

## Power

The board includes a battery input, a physical switch, and both 5V and 3.3V regulators. This separates the higher-current actuator needs from the lower-voltage logic and sensing electronics.
