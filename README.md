# SCE LED Sign

## Setup

1. Connect LED matrix Bonnet to RPI
2. Connect 5V 10A barrel jack power supply to Bonnet (do not supply power to usb port)
3. Connect to network via ethernet
2. Pull/Download repo to RPI
3. Build executable (may need to install gcc)

    make

4. Run server (python is pre-installed, no need to install)

    sudo python server.py
   
5. Open web interface via http://<pi-ip-address>
6. Play with it as much as you like!

## How to make system activate at boot

Add this line to your `/etc/rc.local` file

    sudo python /<path-to-sign-repo>/server.py &

Typical Example:

    sudo python /home/pi/sce-led-sign/server.py &

## To setup as wireless AP

Follow these instructions here: 

https://www.raspberrypi.org/documentation/configuration/wireless/access-point.md

