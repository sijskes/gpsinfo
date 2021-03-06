/*
 *  gpsinfo.h
 * 
 *  Created on May 5, 2017 10:14:27 PM by Simon IJskes
 * 
 *  GpsInfo - Support software for a GPSDO
 *  (c) Simon IJskes 2017
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef GPSINFO_H
#define GPSINFO_H

// remove or comment line below to disable ublox initialisation.
#define PA0JBB_UBLOX_INIT 1 

// change line below to modify stable time
#define STABLE_LOCK_TIME (60L*60L*3L)


// below works automatically
#if defined(ARDUINO)

#define PA0JBB_GPSDO 1

// native code on arduino.

#include <avr/pgmspace.h>

#define LED_LONG_LOCKED 9
#define LED_LONG_UNLOCKED 8


#define ROM PROGMEM
#define LOG_C(c) /**/
#define LOG_C_H(c) /**/
#define LOG_S(s) /**/

#elif defined(DESKTOP)
// cross development environment, setup for x86 gcc.

#define PA0JBB_GPSDO 1
#define GPS_DUMMY 1
#undef PA0JBB_UBLOX_INIT 

#define ROM /**/
#define pgm_read_byte_near(p) (*(p))

void log_c( char c );
void log_c_h( char c );
void log_s( char *s );

#define LOG_C(c) log_c(c);
#define LOG_C_H(c) log_c_h(c);
#define LOG_S(s) log_s(s);


#else
#error please check platform define
#endif

#if PA0JBB_GPSDO

#define DISP_COLS 20
#define DISP_LINES 4

#elif LCD_KEYPAD_SHIELD

#define DISP_COLS 16
#define DISP_LINES 2
#define GPS_DUMMY 1
//#define DISP_DEV 1

#endif


#include <stdint.h>

typedef struct {
    bool touched ;
    bool comm_err ;
    long lock_time ; // in seconds.
    bool locked_stable ;
    bool locked ;
} gpsinfo_t ;

extern gpsinfo_t gpsinfo ;

void gpsinfo_init();
void gpsinfo_poll();

void hal_gps_open();
int hal_gps_in();
void hal_gps_out(uint8_t c);

void hal_disp_out(char c);

void hal_background();
long hal_millis();

void hal_stable_lock( bool b );


void timer_reset();
long timer_count();

extern char *nm_buf_rd ;

typedef struct
{
    int seconds ;
    int minutes ;
    int hours ;
    int days ;
    int years ;
} gpsinfo_time_t;


void ublox_init();

#endif /* GPSINFO_H */

