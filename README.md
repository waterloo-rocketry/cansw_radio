# Radio Board Control Software

Runs on the [Radio PCB (Earl
Grey)](https://github.com/waterloo-rocketry/canhw/tree/master/radio). This
board is responsible for all communications between the rocket's
onboard electronics and the ground support equipment (GSE). It's also
responsible for the power state of all RocketCAN PCBs, being able to
disconnect power from the CANbus in order to extend battery life.

Radio board maintains counts for how long it's been since it received
a heartbeat message from each board on the bus
([canlib/message_types.h::BOARD_STAT
message](https://github.com/waterloo-rocketry/canlib/blob/master/message_types.h))
in order to report other system failures to the GSE.

## How to build and run

- Open MPLAB X
- Connect a PICKIT 3 to your computer via USB
- Line up the PICKIT cable such that the red wire lines up with A) the
  arrow on the PICKIT, and B) the MCLR marking on the board.
- Power the board through the 12V barrier block. The positive terminal
  should be indicated with red paint.
- Click the "Make and Program Device" toolbar button in MPLAB.
