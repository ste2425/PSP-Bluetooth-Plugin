# PSP Bluetooth TechDemo [![C/C++ CI](https://github.com/ste2425/PSP-Bluetooth-TechDemo/actions/workflows/c-cpp.yml/badge.svg?branch=next)](https://github.com/ste2425/PSP-Bluetooth-TechDemo/actions/workflows/c-cpp.yml) ![Github latest downloads](https://img.shields.io/github/downloads/ste2425/PSP-Bluetooth-TechDemo/total.svg)

PSP Bluetooth Tech Demo is a PSP application built using the unofficial PSPSDK and [glib2d (modded)](https://github.com/joel16/gLib2D). The code base for this project is based on the brilliant [CMFileManager](https://github.com/joel16/CMFileManager-PSP) project by *joel16*. It leverages the equally good SIO library from the [Operation Ditto](https://github.com/operation-ditto) project.

The purpose of this homebrew is to demonstrate the `PSP Bluetooth` project and how the PSP can interface with bluetooth controllers.

## Acknowledgements

The work ive done is just a thin layer ontop of work many other people have done, Iv'e also had lots of help from others.

* TokyoDrift - Originally created work that uses the SIO port to allow external controllers back around 2010. Since uploaded to [Github](https://github.com/unraze/PSXControllerToPSP)
* OPDitto project - A project that uses the SIO port to allow for a real second analog stick. Its SIO implementation has been extremly helpful to me. Repo [Here](https://github.com/Operation-DITTO)
* X41 - Shared with me an initial working SIO hello world app back when i could barely compile PSP code. This was invaluable in helping me get the hardware working right at the start of the project.
* PSP Homebrew Discord - Full of incredibly smart and helpful people too many to list individually. Without their help I'd never have managed to complete any of this.

**Features:**

- PONG demo demonstrating the use of two controllers

[![Watch the video](https://img.youtube.com/vi/E2xcQoVHV1s/default.jpg)](https://youtu.be/E2xcQoVHV1s)

- LED Colour demo demonstrating setting the colour of controllers which support LEDs (DualSense etc)

[![Watch the video](https://img.youtube.com/vi/3fGxJrmux3g/default.jpg)](https://youtu.be/3fGxJrmux3g)
- Vibration demo. Demonstrating the use of setting rumble including controllers which support srong and weak rumble.
- Player LED demo. Demonstrating setting the player LED of conrollers.
- Ping. Demonstratings pinging the ESP32 to be sure if the `PSP Bluetooth Dongle` is connected.
- Controller type detection. Demonstrates requesting controller type and bettery information.

[![Watch the video](https://img.youtube.com/vi/i-L--a_C-jo/default.jpg)](https://youtu.be/i-L--a_C-jo)
- Enabling/disabling new connections. Demonstrates enabling new controller connections when pairing.

TODO add pictures and achnowledgements and setup instructions
