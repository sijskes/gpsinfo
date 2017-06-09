/*
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

#include <LiquidCrystal.h>

#include <ctype.h>

#include "nmea.h"
#include "gpsinfo.h"

#if PA0JBB_GPSDO
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
#elif LCD_KEYPAD_SHIELD
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#endif

void setup()
{
    // set up the LCD's number of columns and rows: 
    lcd.begin(DISP_COLS, DISP_LINES);
    lcd.clear();
    lcd.print("GPSinfo - PA9TV");
    lcd.setCursor(0, 1);
    lcd.print("Startup...");

    gpsinfo_init();
    hal_gps_open();
    
    timer_reset();
    
    hal_stable_lock(0);
    
    pinMode(LED_LONG_LOCKED,OUTPUT);
    pinMode(LED_LONG_UNLOCKED,OUTPUT);
}


void loop()
{
    gpsinfo_poll();
}

#ifdef GPS_DUMMY  

#else

static void progress(char c)
{
    lcd.print(c);
}

void hal_gps_open()
{
    progress('I');
    //    Serial.begin(4800);   
    Serial.begin(9600);
    progress('R');
    ublox_init();
}

int hal_gps_in()
{
    int res = Serial.read();
    return res;
}

#endif

static int line = 0;
static int pos = 0;

void hal_disp_out(char c)
{
    hal_background();
    
    switch (c) {
        case '\v':
            lcd.clear();
            // fall through
        case '\f':
            line = 0;
            pos = 0;
            lcd.setCursor(0, 0);
            return;
        case '\n':
            for(int i=pos;i<DISP_COLS;i++) {
#if DISP_DEV              
                lcd.write('*');
#else                
                lcd.write(' ');
#endif                
            }
            line++;
            pos = 0;
            lcd.setCursor(0, line);            
            return;
    }
    if( pos >= DISP_COLS ) {
        return;
    }
    if( isprint(c) ) {
        lcd.print(c);
        pos++;
    }
}

void hal_gps_out(uint8_t c)
{
    Serial.print((char)c);
}

long hal_millis()
{
    return millis();
}

long hal_stable_lock( bool b )
{
    digitalWrite(LED_LONG_LOCKED,b);
    digitalWrite(LED_LONG_UNLOCKED,!b);
}

