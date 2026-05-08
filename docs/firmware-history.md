# Firmware History

The firmware folder is split into the final sketch and archived development versions:

```text
firmware/
  natcar_final/
    natcar_final.ino
  archive/
    natcar_v1.1.ino
    natcar_v1.2.ino
    natcar_v1.3.ino
    natcar_v1.4.ino
    natcar_v1.5.ino
    natcar_v1.6.ino
    natcar_v1.8.ino
```

The archive shows the project moving from broader sensor-fusion experiments toward a leaner camera-based controller.

## Version Themes

| Version | Main theme |
| --- | --- |
| `v1.1` / `v1.2` | Early camera and inductor integration, discrete speed and turn-angle logic |
| `v1.3` / `v1.4` | Expanded dual-motor PWM outputs and more explicit line-detection helpers |
| `v1.5` / `v1.6` | PD-style tuning experiments and additional serial debugging output |
| `v1.8` | Cleaner camera-only loop with top-speed reduction based on line error |
| `v1.9` / `natcar_final` | Finalized camera midpoint detection, PD steering, speed ramp-down on line loss |

## Development Progression

Early sketches read the camera and all three inductors each loop. They include helper functions for:

- Estimating camera line position with `PosCam()`
- Estimating guide position with `PosAFE()`
- Checking whether the camera or inductor front end detected the line
- Choosing motor speed with `CalculateSpeed()`
- Choosing steering angle with `CalculateAngle()`

Later sketches simplify the control loop:

```text
readCamera() -> detectLine() -> updateSteering() -> updateMotorSpeed()
```

That simplification makes the final code easier to tune and faster to reason about during track testing.

## Final Sketch Notes

The final sketch uses:

- 128-pixel camera sampling
- Edge-based line midpoint detection
- `CAMERA_CENTER_PIXEL = 58` as the calibrated center
- Steering centered at `45`
- Steering clamp from `20` to `70`
- PD steering with `STEERING_KP = 0.6` and `STEERING_KD = 0.6`
- Speed reduction proportional to absolute camera error
- Minimum PWM of `30` when the line is lost

## How to Read the Archive

The archive files are kept as historical snapshots. Because Arduino treats every `.ino` in a single sketch folder as part of one build, the archive folder is best read as source history rather than opened directly as one Arduino sketch.

To compile an archived version, copy that one `.ino` file into its own sketch folder with the same name, then open it in the Arduino IDE or Teensyduino environment.
