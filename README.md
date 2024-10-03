# Dynamixel_Arduino_library
The next repository is a library to use Dynamixel motors with arduino without using their proprietary hardware. 
Currentrly support only for Protocol 1.0 and works with physical serial ports of arduino. (Working on SoftwareSerial compatibility).
Description will be added later. However commands work similar to matlab tools for dynamixel.

To use this library with your respective Dynamixel motor go to the [official site](https://emanual.robotis.com/docs/en/dxl/) and look for the specifications. Check the Control Table section in which it is detailed the data name, byte size and addres in the internal memory of the motro. This is the addres that you need to send to the motro in order to acces the values to reed or modify.

![Control table](https://github.com/TulioCarmona/Dynamixel_Arduino_library/blob/main/control_table.png)

Whit the information of the table then you can use the libreary functions write/read 1,2,4 bytes.
