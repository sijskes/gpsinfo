/*
 *  gspinfo.cpp
 * 
 *  Created on May 5, 2017 10:13:42 PM by Simon IJskes
 * 
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

#include "gpsinfo.h"
#include "nmea.h"
#include "printf.h"

#include <limits.h>

gpsinfo_t gpsinfo;

bool last_valid = 0;
bool locked = 0;
bool locked_long = 0;

static inline void outch(void*, char c)
{
    hal_disp_out(c);
}

void gpsinfo_init()
{
    init_printf(0, outch);

    gpsinfo.comm_err = 1;
    gpsinfo.touched = 1;
}

gpsinfo_time_t tim1;

void fix_overflow(int& field, long& accu, int modulo)
{
    field = accu % modulo;
    accu /= modulo;
}

void set_time(long time)
{
    fix_overflow(tim1.seconds, time, 60);
    fix_overflow(tim1.minutes, time, 60);
    fix_overflow(tim1.hours, time, 24);
    fix_overflow(tim1.days, time, 365);
    tim1.years = time; // max 68 jr.
}

void gpsinfo_disp()
{
#if DISP_LINES == 4
    printf("\f");
    if( gpsinfo.comm_err ) {
        printf("\vGPSINFO PA9TV\n");
        printf("communication error\n");
    } else {
        set_time(gpsinfo.lock_time);

        printf("val: %d ", nmea.valid);
        printf("sats: %2d/%2d\n", nmea.sats_used, nmea.sats_inview);

        printf("%02d:%02d:%02d  ", nmea.time.hr, nmea.time.min, nmea.time.sec);
        printf("%02d-%02d-%04d\n", nmea.date.day, nmea.date.mon, nmea.date.yr);

        if( nmea.valid ) {
            if( (nmea.time.sec % 10) < 5 ) {
                printf("lat:   %3d'%02d.%04ld %c\n", nmea.lat.degrees, nmea.lat.minutes, nmea.lat.micromin / 100L, nmea.lat.direction);
            } else {
                printf("lon:   %3d'%02d.%04ld %c\n", nmea.lon.degrees, nmea.lon.minutes, nmea.lon.micromin / 100L, nmea.lon.direction);
            }
        } else {
            printf("\n");
        }
        //printf("%ld %d\n", gpsinfo.lock_time, nmea.chkerr);

        printf("lck: %02d.%03d %02d:%02d:%02d\n", tim1.years, tim1.days, tim1.hours, tim1.minutes, tim1.seconds);
    }
#elif DISP_LINES == 2
    printf("\f");

    if( gpsinfo.comm_err ) {
        printf("\vGPSINFO PA9TV\n");
        printf("comms error\n");
    } else {
        set_time(gpsinfo.lock_time);

        printf("%02d:%02d:%02d sats:%d\n", nmea.time.hr, nmea.time.min, nmea.time.sec, nmea.sats_inview);
        //        if( tim1.days < 10 ) {
        //          printf("lck: %01d %02d:%02d:%02d\n", tim1.days, tim1.hours, tim1.minutes, tim1.seconds );
        //        } else {
        //          printf("lck: %02d.%03d %02d\n", tim1.years, tim1.days, tim1.hours );
        //        }
        printf("%02d.%03d %02d:%02d:%02d\n", tim1.years, tim1.days, tim1.hours, tim1.minutes, tim1.seconds);
    }
#else
#error bad define DISP_LINES
#endif    

}

long last_mil = 0;

void signal_commerr()
{
    gpsinfo.lock_time = 0;
    if( gpsinfo.comm_err == false ) {
        gpsinfo.comm_err = true;
        gpsinfo.touched = true;
    }
}

void gpsinfo_poll()
{
    hal_background();

    {
//        long time = hal_millis();
//        time -= gpsinfo.pkt_time;
//        if( time < 0 ) {
//            time += LONG_MAX;
//            time++;
//        }
        long time = timer_count();
        if( time > 2000L ) {
            // 2 seconds no info
            signal_commerr();
        }
    }

    if( nm_buf_rd != NULL ) {
        timer_reset();
//        gpsinfo.pkt_time = hal_millis();
        if( nmea_parse_buf(nm_buf_rd) == true ) {
            if( gpsinfo.comm_err == true ) {
                gpsinfo.comm_err = false;
                gpsinfo.touched = true;
            }
        }
        nm_buf_rd = NULL;
    }

    if( nmea.touched ) {

        long nmea_time = nmea.msec_time / 1000L;

        if( nmea.valid != last_valid ) {
            if( nmea.valid ) {
                last_mil = nmea_time;
                gpsinfo.lock_time = 0;
                locked = 1;
            } else {
                gpsinfo.lock_time = 0;
                locked = 0;
            }
            last_valid = nmea.valid;
        }

        if( locked ) {

            // calculate time step
            long diff = nmea_time - last_mil;

            if( diff < 0L ) {
                // wrap around
                diff += 60L * 60L * 24L; // 24hr * 60min * 60sec 
            }
            last_mil = nmea_time;

            gpsinfo.lock_time += diff;
        } else {
            last_mil = 0L;
        }

        nmea.touched = 0;
        gpsinfo.touched = 1;
    }

    if( gpsinfo.touched ) {
        gpsinfo_disp();
        gpsinfo.touched = 0;
    }
}
