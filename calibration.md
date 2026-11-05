# Acoustic calibration guide

## Why calibration is necessary

The KY-037 is a low-cost microphone/amplifier module. It detects sound amplitude; it does not identify a species. Its response changes with module gain, microphone orientation, distance, enclosure, wind, and background noise. The firmware's frequency estimator is only a first-pass prototype.

## Recommended calibration procedure

1. Keep automatic relay activation disabled.
2. Fix the microphone position and do not change it during a test session.
3. Capture baseline readings in the field with no known target insect present.
4. Record known target-insect activity, if available, and save labelled audio using a separate recorder if possible.
5. Repeat under different distances, times, and environmental conditions.
6. Compare target and background distributions rather than selecting a threshold from one observation.
7. Split recordings by session/day/location for evaluation to avoid data leakage.
8. Report false positives and false negatives.

## Parameters to tune

- `MIN_DETECTION_HZ`
- `MAX_DETECTION_HZ`
- `MIN_PEAK_TO_PEAK`
- `REQUIRED_DETECTIONS`
- `DETECTION_HOLD_MS`

The defaults in the sketch are placeholders for bench testing. They are not claimed biological limits for stem borer wingbeats.

## Better next step

For research-grade detection, record raw audio at a stable sample rate, apply filtering and FFT/spectral feature extraction, and evaluate a classifier on independently labelled recordings. Keep environmental noise and non-target insects in the dataset.
