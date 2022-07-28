<div id="top" align="center">

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
  
</div>

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/microgui/MicroGUI-Embedded">
    <img src="images/MicroGUI.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">MicroGUI Embedded</h3>

  <p align="center">
    Open-source Arduino library for deploying GUIs created with MicroGUI on embedded displays.
    <br />
    <a href="https://github.com/microgui/MicroGUI-Embedded/wiki"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="https://github.com/microgui/MicroGUI-Embedded/issues/new?labels=bug">Report Bug</a>
    ·
    <a href="https://github.com/microgui/MicroGUI-Embedded/issues/new?labels=enhancement">Request Feature</a>
  </p>
</div>

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li>
      <a href="#usage">Usage</a>
      <ul>
        <li><a href="#tutorial">Tutorial</a></li>
        <li><a href="#examples">Examples</a></li>
        <li><a href="#api">API</a></li>
      </ul>
    </li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## About The Project
<div align='center'>
  <a>
    <img src="images/ExampleGUI.jpg" width="500">
  </a>
</div>

The main purpose of MicroGUI is to offer a convenient solution for makers to implement touch displays in their embedded projects. This library is supposed to be used together with the [web application](https://github.com/microgui/MicroGUI) to enable rapid prototyping of your own GUI.

MicroGUI-Embedded takes care of making your GUI work on embedded displays with minimal coding from your part, so that you can focus on the project at hand instead of having to learn how to program displays. You will also be able to monitor and control your displays remotely by connecting them to your WiFi. Read more about this in the following sections.


<p align="right">(<a href="#top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Prerequisites

* Any of the below listed embedded displays:
  * WT32-SC01

* IDE (Integrated Development Environment)
    * It is recommended to use the [PlatformIO](https://platformio.org/platformio-ide) extension for [VS Code](https://code.visualstudio.com/) to program your embedded display for use with MicroGUI. The installation instructions below are for PlatformIO, but you can install this library in [Arduino IDE](https://www.arduino.cc/en/software) as well if you would prefer that.


## Installation
Note: These instructions are specific to the WT32-SC01 display. As more displays are supported by MicroGUI in the future, this section will most likely change.
### New project:
1. Create a new PlatformIO project and select `Espressif ESP32 Dev Module` as development board. Make sure that Arduino is set as framework. For detailed instructions, look [here](https://docs.platformio.org/en/stable/tutorials/espressif32/arduino_debugging_unit_testing.html).
2. Open the `platformio.ini` and paste the following:
   ```ini
    [env:esp32dev]
    platform = https://github.com/platformio/platform-espressif32.git
    board = esp32dev
    framework = arduino
    platform_packages = framework-arduinoespressif32 @ https://github.com/espressif/arduino-esp32#master
    upload_speed = 921600
    monitor_speed = 115200
    lib_deps = https://github.com/microgui/MicroGUI-Embedded.git
   ```
   This makes sure that you use the latest Arduino core for the ESP32 as well as adds MicroGUI-Embedded to the list of library dependencies.
3. That should be all, have fun!

### Existing project:
1. Simply add MicroGUI-Embedded to your library dependencies, like this: 
   ```ini
    lib_deps = https://github.com/microgui/MicroGUI-Embedded.git
   ```
2. Done, enjoy!

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- USAGE -->
## Tutorial
### First test
After you've installed both the MicroGUI web application and this library, you are set to create and deploy GUIs.

However, as a quick first test to see that the library installation was successful, copy the template into your main sketch. Hit upload and wait for it to finish. The default GUI should appear prompting you to continue reading the tutorials.
```cpp
#include <MicroGUI.h>
//#include <RemoteMicroGUI.h>     // Uncomment if you want to use MicroGUI Remote features

void setup() {
  Serial.begin(115200);

  mgui_init();
  //mgui_remote_init();           // Uncomment if you want to use MicroGUI Remote features
}

void loop() {
  MGUI_event * latest = mgui_run();
}
```
You can continue using this template as a base for any new projects you create with MicroGUI.

### Second test
After you've seen that this works, test the MicroGUI Remote features by uncommenting the commented out lines of the template. Once the display is programmed you will be presented with the same default GUI except for this time you will see a red border indicating that the display is not connected to any WiFi network. The display is now in AP (access point) mode, meaning you can connect to it like any other WiFi network. If you connect to the display's AP through your phone, you will be served a captive portal where you can enter WiFi network credentials for the display to connect to. If you connect through your computer you have to browse to [http://192.168.4.1/](http://192.168.4.1/) to enter the same website for entering your WiFi network credentials. Once you click the 'Submit' button, the display will start trying to connect to the provided network. If successful, the display will switch to stationary mode, meaning the access point closes.

At this point, you are ready to use MicroGUI Remote features including remote monitoring and control as well as GUI upload which you can find in the web application. Your display's IP adress will be displayed on the screen.

## Work flow
The following section is just a recommended way of working with MicroGUI.

## API


<p align="right">(<a href="#top">back to top</a>)</p>

<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#top">back to top</a>)</p>

<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.md` for more information.

<p align="right">(<a href="#top">back to top</a>)</p>


<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/microgui/MicroGUI-Embedded.svg?style=for-the-badge
[contributors-url]: https://github.com/microgui/MicroGUI-Embedded/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/microgui/MicroGUI-Embedded.svg?style=for-the-badge
[forks-url]: https://github.com/microgui/MicroGUI-Embedded/network/members
[stars-shield]: https://img.shields.io/github/stars/microgui/MicroGUI-Embedded.svg?style=for-the-badge
[stars-url]: https://github.com/microgui/MicroGUI-Embedded/stargazers
[issues-shield]: https://img.shields.io/github/issues/microgui/MicroGUI-Embedded.svg?style=for-the-badge
[issues-url]: https://github.com/microgui/MicroGUI-Embedded/issues
[license-shield]: https://img.shields.io/github/license/microgui/MicroGUI-Embedded.svg?style=for-the-badge
[license-url]: https://github.com/microgui/MicroGUI-Embedded/blob/main/LICENSE.md
[product-screenshot]: images/canvas.png
