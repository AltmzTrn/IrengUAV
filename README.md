# IrengUAV
lightweight UAV firmware

## Introduction
This project was started in early 2024 by Altamis Tristan to implement direct, low-level language control in UAV systems. The core objective is to create a highly optimized, lightweight firmware footprint capable of running on lower-cost microcontrollers—specifically the STM32F1 and STM32F4 series development boards, ESP32 development platforms, and Arduino. The target airframes for this firmware are small UAVs falling safely under the 25kg MTOW limit.

## Current Version
Currently, the primary active development branch of this firmware focuses on a sub-100g brushed quadcopter utilizing an STM32F103C8T6 as the main bare-metal flight controller.  
The firmware is developed using arduino framework with a modular architecture for fast prototyping on each of the airframe types. Each of the files are separated based on its functions:
- Attitude_Indicator files: Obtains information from the IMU
- CRSFHandler: Handles the RC input from ELRS/CRSF Modules
- actuators: Handles the actuation (servo timers, ESC PWM/dshot modules, MOSFET analog signals)
- controlSystems: implements control mixing & PID stabilisation.

## Future Development Plans
Once the firmware has been confirmed to be able to establish a stable flight on an airframe, the firmware framework will be optimised from Arduino-based frameworks to bare-metal C++ and Assembly in order to save space for the development of more features that can be implemented on the Airframe.


## Archived Versions
Development originally began merely as a weekend project as early as January 2024. Early updates, commits, and versions were initially logged and stored on Google Drive. The ```/archive``` folder in this repository stores all of the previously developed firmwares for various types of experimental Airframes from 2024 to 2025. 



_Altamis Tristan -- 2026.06.05_
_Last edited 2026.06.06_
