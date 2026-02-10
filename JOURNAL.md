# AuraLink Development Journal

### Feb 9, 2026 - Initial Concept & Simulation
* **Goal:** Validate if an ESP32 can handle 5 analog flex sensors and an MPU6050 simultaneously.
* **Progress:** Created a full circuit simulation in Wokwi. Confirmed that the ADC pins on the ESP32 can read finger positions and the I2C bus can track hand rotation.
* **Status:** Simulation works perfectly.

### Feb 10, 2026 - Repository & Architecture
* **Goal:** Organize the project for the Blueprint Expert Mode application.
* **Progress:** Set up the GitHub repository with dedicated folders for `firmware`, `hardware`, and `docs`. 
* **Decision:** Switched to Expert Mode to focus on custom biometric hardware (flex sensors) rather than the standard macro-pad kit.

### Feb 10, 2026 - Budgeting & Pitch
* **Goal:** Finalize the Bill of Materials (BOM).
* **Progress:** Researching part costs for high-quality flex sensors. Total estimated cost is ~$200, well within the $400 grant limit.
