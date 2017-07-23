# pwr_controller
DC-DC step up module for Orange PI Zero, controlled by Attiny24A

This repository contains code for https://github.com/ildus/power_controller

# Orange PI Zero setup

1. Install Armbian (https://www.armbian.com/orange-pi-zero/). Use `mainline` version.
1. Download and compile avrdude. Use `--enable-linuxgpio` option for `./configure`.
1. Check connections. MISO, MOSI and SCLK like in image. Use CS0 for reset. Pinout:

![image](https://static.wixstatic.com/media/08fc87_2e51538d6d1b4c3bbd4a3351deaf3aa2~mv2.jpg)

4. Try to run `make upload`. It should work if connections are ok.
