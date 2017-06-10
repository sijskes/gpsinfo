
# Purpose

This software is to provide feedback on the state of the GPS part of a 
GPSDO (GPS disciplined oscilator). It displays the number of satelites visible 
and the satelites used. It counts the seconds the signal is valid, from the 
moment it is valid, providing some indication of the quality of the 10Mhz TCXO 
(temperature controlled crystal oscillator)
it drives. If a certain lock time has passed, now 3 hours, 
the indication in the display is changed from locked into stable.
The ublox gps module is configured by the controller into a mode giving the best
performance for timebase generation. Powersave options are disabled, Glonass 
support is disabled, and 10Mhz is selected as a time pulse output. The GPS and 
Arduino Nano controller are combined with a 10 Mhz TCXO, the electronics and the concept
of the system designed by Hans, PA0JBB.

# Disable U-blox initialization

It is possible to skip the u-blox initialization. You have to remove the:

    #define PA0JBB_UBLOX_INIT 1

line from gpsinfo.h


# License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Notice

This program contains a modified copy of the printf library, 
copyright Kustaa Nyholm / SpareTimeLabs. License info to be found in the 
printf.c and printf.h files.



