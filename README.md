# GUIDE glove and ARM64 Assembler-Emulator

A 4-person group project in C, including:

- code for the **GUIDE glove** (the extension),
- an *assembler* for ARM64 assembly,
- an *emulator* for the ELF binaries generated.
  
The **GUIDE glove** is a wearable assistive device for the visually impaired, that increases spatial awareness.

(This repo has been made as the original repo was under our private university accounts)

## Table of Contents

- [About the Project](#about-the-project)
  - [Part 1: The Assembler-Emulator](#part-1-the-assembler-emulator)
  - [Part 2: The GUIDE glove](#part-2-the-extension-name)
- [How to Run](#how-to-run)
- [Project Structure](#project-structure)

## About the Project

As part of university coursework, our team chose to develop an assembler for ARM64 assembly code, and a program that emulates the assembly code's effect on a Raspberry PI. Following this, we wanted to implement a Raspberry PI into an extension of our own, that solved a real-world problem.

We chose to solve an issue faced by a product with a limited use case: canes. The visually impaired use canes to assist with their lack of vision, but they don’t suit every situation. Inspired by a viral video on the internet, we’ve come up with a solution that addresses the direct shortcomings of canes – not to be used as an alternative, but rather in tandem: the GUIDE Glove.

We designed and developed a glove that can be worn by the blind and visually impaired to increase their spatial awareness. Our glove uses ultrasonic sensors to scan the wearer’s surroundings, and provide information about the surrounding environment through haptic and audio feedback. By modulating at specific frequencies which correlate to specific distance ranges, the GUIDE Glove informs its user of obstacles within close-quarters. By supporting near-field environments that canes are too large to get to, our glove lets the visually impaired interact with completely unfamiliar environments that they previously would not be able to – such as a new kitchen where they don’t know the layout. Some of the problems with canes that our glove addresses are: portability, discreteness, durability, confined spaces, and improper sizing.

### Part 1: The Assembler-Emulator

Our project includes a robust ARMv8 AArch64 assembler and emulator, designed to convert assembly code into executable binaries and then simulate their execution. It provides a foundational tool for understanding and developing low-level programs for ARM-based systems.

* **Two-Pass Assembly Process**: Implements a classic two-pass approach to handle forward references, building a symbol table in the first pass and generating 32-bit instruction words in little-endian format in the second.
* **Refactored Tokenizer**: Features an advanced tokenizer that handles special characters like `[`, `]`, `,`, `#`, and `!` as individual tokens, significantly simplifying parsing for complex addressing modes.
* **Comprehensive Instruction Support**: Accurately assembles and emulates Data Processing (DP), Data Transfer (LDR/STR), and Branch instructions, including complex addressing modes and correct offset calculations for branching.

### Part 2: The GUIDE glove

The **GUIDE Glove** is our project's most impressive extension, a real-time, multi-modal navigation aid designed to assist visually impaired individuals in traversing complex and confined environments. This innovative system integrates an ultrasonic sensor, a buzzer, and a vibration motor with a Raspberry Pi to provide dynamic haptic and auditory feedback based on object proximity.

* **Real-time Multi-Modal Feedback**: The glove delivers instantaneous haptic (vibration) and auditory (buzzer tone and rhythm) feedback, providing a comprehensive spatial awareness experience for the user.
* **Dynamic Intensity & Non-Linear Algorithm**: The feedback intensifies as an object gets closer, driven by a custom-designed non-linear algorithm that translates distance data into varying pitches, rhythms, and vibration intensities. This provides nuanced information about proximity.
* **Critical "Danger Zone" Alert**: For immediate safety, a critical "danger zone" (e.g., 5cm proximity) triggers a distinct, constant, and high-intensity sound and vibration, ensuring rapid alerts for imminent collisions.
* **Hardware Integration & C Control System**: Developed as a C application on the Raspberry Pi using the `pigpio` library, the extension seamlessly interfaces with the ultrasonic sensor to gather data and control the buzzer and vibration motor, demonstrating robust hardware-software co-design.

### How to Run

Detailed instructions on how to execute both the assembler-emulator and the extension.

#### Running the Assembler-Emulator

To run, cd into the `src` directory and simply run: 
  - `make` to compile all files.
  - `./assemble <file_in> [file_out]` to assemble the ARM64 assembly in `<file_in>` into an ELF binary in `[file_out]`
  - `./emulate <file_in> [file_out]` to emulate the ELF binary `<file_in>` into `[file_out]`
    
```bash
# Example usage
make
./assemble led_blink.s kernel8.img
./emulate kernel8.img emulated_output
```

#### Running the GUIDE glove

Unfortunately, due to the projects nature, the GUIDE glove's code cannot be run without the glove itself (and its sensors).

#### Project Structure
```
├── src/                  # Source code for the assembler and emulator
├── extension/            # Source code for the GUIDE glove
├── programs/             # Example assembly program and our assemblers generated binary
├── doc/                  # Documentation about the development of our project
└── README.md
```
