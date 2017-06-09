/*
 *  nmea.h
 * 
 *  Created on Apr 30, 2017 6:04:47 PM by Simon IJskes
 * 
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

#ifndef NMEA_H
#define NMEA_H

#include <stdlib.h>
#include <stdint.h>
//#define int8_t int

typedef struct {
    int8_t day;
    int8_t mon;
    int yr;
} nm_date_t;

typedef struct {
    int8_t msec;
    int8_t sec;
    int8_t min;
    int8_t hr;
} nm_time_t;

typedef struct {
    char str[15];
    int degrees;
    int minutes;
    long micromin;
    char direction;
} navmag_t;

typedef struct {
    bool valid;
    bool touched;
    navmag_t lat;
    navmag_t lon;
    nm_date_t date;
    nm_time_t time;
    long msec_time;
    int sats_inview;
    int sats_used;
    int fmterr ;
    int chkerr ;
} nmea_t;

extern nmea_t nmea;

bool nmea_parse_buf(char *p);

//#ifdef __cplusplus
//extern "C" {
//#endif
//
//
//
//#ifdef __cplusplus
//}
//#endif

#endif /* NMEA_H */

