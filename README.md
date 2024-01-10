# Unnamed Safeguarding Prototype Client

## Overview
This client application is a part of the Unnamed Safeguarding Prototype project. It focuses on monitoring user activities and communicating with the server for real-time data transmission and analysis.

## Features
- **Real-Time Activity Monitoring**: Monitors keyboard inputs and screens for specific content.
- **Secure Communication**: Implements secure communication protocols to interact with the server.
- **Automated Locking Mechanism**: Capable of locking the screen based on specific triggers.

## Getting Started

### Prerequisites
- Windows environment for running the application.
- Boost library for network programming.
- GDI+ for graphical operations.

### Installation
1. Clone the repository.
2. Open the project using Visual Studio.
3. Build the solution for your platform (x86 or x64).

### Usage
- Run the client application in a Windows environment.
- The application will start monitoring activities and communicate with the server.

## Code Structure
- `main.cpp`: Entry point of the application.
- `KeyspaceMonitor.cpp/h`: Handles keyboard input monitoring and processing.
- `ScreenLocker.cpp/h`: Manages locking and unlocking of the user's screen.
- `AckHeader.h` and `Header.h`: Define structures for communication and data handling.

## Acknowledgements
- Boost library
- Windows API for system-level operations
- GDI+ for graphical functionalities
