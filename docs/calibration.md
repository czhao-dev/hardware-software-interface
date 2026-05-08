# Calibration

This project includes two spreadsheet files related to inductor testing and scaling:

```text
data/inductor_measurement.xlsx
data/inductor scaling.xlsx
```

These files help explain how the analog inductive pickups behaved and how their readings could be converted into useful control information.

## Inductor Measurement

`inductor_measurement.xlsx` records readings from the left, center, and right inductive sensors at different positions.

The sheet labels indicate:

- `pos`: lateral position
- positive position: left
- negative position: right
- `Left`, `Center`, and `Right`: measured sensor responses

The chart data shows the center sensor peaking around the center of the guide reference, while the left and right sensors respond more strongly as the guide reference moves toward their side.

## Inductor Scaling

`inductor scaling.xlsx` compares several inductor response curves and includes linear trendlines. The chart labels include:

```text
y1 = 401.56x - 85.273
y2 = 383.39x - 20.909
y3 = 290.85x - 38.625
```

Those trendlines suggest the team was trying to normalize or compare the sensitivity of different sensor channels.

## How Calibration Connects to Firmware

The archived sketches contain logic that maps inductor readings into a coarse left/center/right position estimate:

```cpp
int positionAFE = PosAFE(inductor1, inductor2, inductor3);
```

Earlier versions then used that position estimate to select steering angles when the camera did not detect the line. For example, the archived control path could steer harder left or right depending on which inductor had the strongest response.

## Practical Calibration Workflow

A repeatable calibration process would look like this:

1. Place the car over the guide reference at known lateral offsets.
2. Record left, center, and right analog readings.
3. Plot response curves for each channel.
4. Identify offsets where each sensor is most sensitive.
5. Tune thresholds for line detection and loss-of-line recovery.
6. Validate the thresholds on the moving car.
