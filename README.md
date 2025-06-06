# Smart-Waiter

This is the repo for a diy embeded system project.

## Description

The system will wake up due to a recorded and detected wake word , load a audio from a
storage media and play it. This repository involves two projects. First the ESP32 flash
and secondly the wake word detection.

## How It Works

### Wake Word Detection

The wake word detection will be done with tensorflow. After a model is trained with the
required data it is converted to a tensorflow lite model. Then the tensorflow lite model
is transcribed to an applicable format for the use with the ESP32.

### ESP32 Flash
The ESP32 will be mostly in doze mode recording surrounding sound and try to detect the wake word
with the tensorflow lite model. When it was successful the ESP32 will load an audio sample from
a storage media and send it's data via I2C to an audio amplifier.