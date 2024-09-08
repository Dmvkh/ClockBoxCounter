# ClockBoxCounter
'Arduino' based device which helps kids track scheduled tasks and chores progresses.

This device designed with 2 independent controllers in mind. Main controller (Wemos board) does Wi-Fi connections to maintain web-server API interactions and processes user actions.
Secondary controller (Attiny board) does async LED blinking, buzz sounds and reads some button inputs, without stuttering main board.

## Used Electrical Parts:

### A) Controller Boards:

#### - Wemos D1 mini.
<img src="./Images/Wemos.png" width="200" />

Used as Master Controller.

**Pros**

* Has integrated Esp8266 onboard to connect to Wi-Fi
* Interrupts on each digital pin
* Large sketch/variable memory
* Quick reboot time

**Cons**

* Small pins amount
* ESP architecture can't propely communicate with Attiny via I2C
* Analog pin can be used only as input
* Can't properly load and flash sketches if D8 pin is pulled-up
* Long compiling and flashing time, comparing to Attiny (up to 1 minute vs 10 secs)
* Randomly blocks virtual COM port after several flashes until PC is rebooted
* Requires special CH341 chip driver to support USB communication
  
#### - AH-TINY Attiny 88.
<img src="./Images/Attiny.png" width="200" />

Used as Slave Controller.

**Pros**

* Very cheap alternative to Arduino
* Plenty of digital and analog PINs. Supports I2C

**Cons**

* **EXTREMELY** ! small RAM size for sketch and variables, which makes programming of many options very tricky
* Doesn't have hardware serial support, which makes it almost impossible to debug
* If variables use more than 96% of memory, became unstable
* For some reason can't communicate with ESP via I2C
* Long time to boot after reset (up to 8 seconds)

### B) Arduino components:

#### SSD1306 I2C OLED Display.
<img src="./Images/SSD1306.png" width="200" />

Used for short pop-up messages and progress bar visualisation.

