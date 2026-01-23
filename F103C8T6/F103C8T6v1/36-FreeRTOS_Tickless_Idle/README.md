# Program 36: FreeRTOS Tickless Idle

Low power RTOS dengan dynamic tick suppression untuk mengurangi konsumsi daya.

## Hardware
- PA0: Button (wake-up source)
- PC13: LED
- Ammeter: Untuk ukur power consumption

## Power Consumption
- Active: ~20mA @ 72MHz
- Idle (tickless): ~500µA
- Savings: ~95% during idle

## Concept
FreeRTOS normally ticks every 1ms (1kHz). Tickless idle **disables tick** during long idle periods, allowing deeper sleep.
