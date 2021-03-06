# usbrelay

It is my version of USB relay module. This type of devices are cheaply availalbe on the eBay, but I just want to build one with parts arounds me.

These are relay modules that I purchased in the past.

![Relay Modules](https://github.com/0x4f48/usbrelay/blob/main/misc/relay-module.jpg)
![usbrelay with case](https://github.com/0x4f48/usbrelay/blob/main/misc/usbrelay-with-case.jpg)


A releay module and Ardunio Nano get powered by a USB port and operated with the control commands from the host.


## Building the usbrelay module

### Hookup Wires

Solder the wires as shown below.

```
Arduino digital pin3 <----> Relay control
Arduino         +5V  <----> Relay power IN
Arduino         GND  <----> Relay power GND 
```

### Flashing Arduino Nano

Import the file under the "ardunio" directory to the Ardunio IDE and build the firmware.


## Using usbrelay

### From a terminal

Open a terminal and set baudrate to 115200. The second command is for preventing the Arduino from auto resetting while opening a serial port.

```
$ stty -F /dev/ttyUSB0 115200

OR

$ stty -F /dev/ttyUSB0 115200 -hupcl
```

You are ready to send commands.

If you need to receive response messages, open another terminal and run "cat" command after setting the baudrate as shown above.

Run "cat" to get responses.

```
$ cat /dev/ttyUSB0
```


#### Commands and response

| Commands | Response | Meaning |
| ------ | ------ | ------ |
| @SW? | #SW0 or #SW1 | Relay Off(0) or On(1) |
| @SW1 | N/A | Relay On |
| @SW0 | N/A | Relay Off |
| @DN? | #DN!device_name | Query device name |
| @DN!device_name| N/A | Program device name |
| @CC? | #CC0 or #CC1 | Query curcuit config (normally open or closed) |
| @CC0 | N/A | Set normally open circuit |
| @CC1 | N/A | Set normally closed circuit |


### Using GUI application

Building and installing GUI application.

Requirement: gtk+3.0

```
$ sudo apt-get install libgtk-3-dev
```

Build and install the GUI app.

```
$ make
$ make install
```

See how it works @ https://www.youtube.com/watch?v=B4PfYyiPoTE
[![usbrelay demo](https://img.youtube.com/vi/B4PfYyiPoTE/0.jpg)](https://www.youtube.com/watch?v=B4PfYyiPoTE)
