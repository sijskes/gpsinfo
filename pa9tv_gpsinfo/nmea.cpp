/*
 *  nmea.cpp
 *
 *  Created on Apr 30, 2017 5:59:57 PM by Simon IJskes
 *
 * 
 *  GpsInfo - Support software for a GPSOD
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

#ifdef DEVxxx
#include <stdio.h>
#define PRINT printf
#else
#define PRINT /**/
#endif

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "nmea.h"
#include "printf.h"


void hal_background();

nmea_t nmea;

static inline char *parse_tok(char *&p)
{
    char *save = p;

    while (1) {
        if( *p == 0 ) {
            return save;
        }
        if( *p == '*' ) {
            *p = 0;
            return save;
        }
        if( *p == ',' ) {
            *p = 0;
            p++;
            return save;
        }
        p++;
    }
}

void parse_time(char *&p)
{
    char *tok = parse_tok(p);

    //    PRINT( "time: %s \n", tok );

    char *dot = strchr(tok, '.');
    if( dot != 0 ) {
        *dot = 0;
        dot++;
        int post = atoi(dot);
        nmea.time.msec = post;
    } else {
        nmea.time.msec = 0;
    }

    // dont get confused here, the data is human presentation.
    // so keep the calculations in 100s which amounts to 2 decimals.
    // no complicated 60*60
    long pre = atol(tok);
    nmea.time.sec = pre % 100;
    pre /= 100;
    nmea.time.min = pre % 100;
    pre /= 100;
    nmea.time.hr = pre % 100;

    // if we want the time in millisecond, whe do have to use 60 etc
    nmea.msec_time = nmea.time.msec;
    nmea.msec_time += nmea.time.sec * 1000L;
    nmea.msec_time += nmea.time.min * 1000L * 60L;
    nmea.msec_time += nmea.time.hr * 1000L * 60L * 60L;
}

void parse_status(char *&p)
{
    char *tok = parse_tok(p);

    //PRINT( "status: %s \n", tok );

    if( *tok == 'A' ) {
        nmea.valid = 1;
    } else {
        nmea.valid = 0;
    }
}

void parse_date(char *&p)
{
    char *tok = parse_tok(p);

    if( strlen(tok) != 6 ) {
        nmea.date.day = 0;
        nmea.date.mon = 0;
        nmea.date.yr = 0;
        return;
    }

    char buf[3];

    strncpy(buf, tok, 2);
    buf[2] = 0;
    nmea.date.day = atoi(buf);

    strncpy(buf, tok + 2, 2);
    buf[2] = 0;
    nmea.date.mon = atoi(buf);

    strncpy(buf, tok + 4, 2);
    buf[2] = 0;
    nmea.date.yr = atoi(buf) + 2000;
}

void parse_latlon_ind(char *&p, navmag_t &navmag)
{
    char *tok = parse_tok(p);
    navmag.direction = *tok;
}

void parse_latlon_magn(char *&p, navmag_t &navmag)
{
    char *tok = parse_tok(p);

    //PRINT( "status: %s \n", tok );

    strncpy(navmag.str, tok, sizeof (navmag.str) - 1);

    char *endpos = strchr(tok, '.');
    if( endpos == 0 ) {
error:
        navmag.degrees = -1;
        return;
    }
    endpos -= 2;
    if( endpos <= tok ) {
        goto error;
    }
    char *pos2 = tok;
    int val = 0;
    while (pos2 < endpos) {
        char c = *pos2;
        if( !isdigit(c) ) {
            goto error;
        }
        val *= 10;
        val += *pos2 - '0';
        pos2++;
    }
    navmag.degrees = val;
    endpos += 2;
    val = 0;
    while (pos2 < endpos) {
        char c = *pos2;
        if( !isdigit(c) ) {
            goto error;
        }
        val *= 10;
        val += *pos2 - '0';
        pos2++;
    }
    navmag.minutes = val;
    pos2++; // skip dot.

    long lval = 0;
    for(int i = 0; i < 6; i++) {
        lval *= 10L;
        char c = *pos2;
        if( !isdigit(c) ) {
            // ignore, and keep pos.
        } else {
            lval += *pos2 - '0';
            pos2++;
        }
    }
    navmag.micromin = lval;
}

static inline void parse_latlon_magn_ind(char *&p, navmag_t &navmag)
{
    parse_latlon_magn(p, navmag);
    parse_latlon_ind(p, navmag);
}

static void parse_sats_inview(char *&p)
{
    char *tok = parse_tok(p);
    nmea.sats_inview = atoi(tok);
}

static void parse_sats_used(char *&p)
{
    char *tok = parse_tok(p);
    nmea.sats_used = atoi(tok);
}

/**
 * Does not contain '$'
 */
static int nmea_chksum(char *pktpoi)
{
    char *p = pktpoi;

    uint8_t chk = 0;
    while (*p != 0) {
        char c = *p;
        if( c == '*' ) {
            break;
        }
//        printf( "%c ",c);
        chk ^= c;
//        printf( "[%x] ",chk);
        p++;
    }
    if( *p != '*' ) {
        // general format err.
        nmea.fmterr++ ;
        return 0 ;
    }
    // skip '*'
    p++ ;
    
    char buf[3];
    sprintf(buf,"%02x",chk);
    
//    printf("|%s|%s| \n",p,buf);
    if( toupper(buf[0]) != toupper(p[0]) ||
            toupper(buf[1]) != toupper(p[1]) ) {        
        nmea.chkerr++ ;
        return 0 ;
    } 
    return 1 ;
}

/**
 * Does not contain '$'
 */
bool nmea_parse_buf(char *pktpoi)
{
    if( !nmea_chksum(pktpoi) ) {
        return false ;
    }
    
    char *tok = parse_tok(pktpoi);

//    // fix talker id.
//    if( tok[0] == 'G' && tok[1] == 'N' ) {
//        tok[1] = 'P';
//    }

    if( strcmp(tok, "GPRMC") == 0 ) {
        parse_time(pktpoi);
        hal_background();
        parse_status(pktpoi);
        hal_background();
        parse_latlon_magn_ind(pktpoi, nmea.lat);
        hal_background();
        parse_latlon_magn_ind(pktpoi, nmea.lon);
        hal_background();
        parse_tok(pktpoi);
        parse_tok(pktpoi);
        parse_date(pktpoi);
        hal_background();
        nmea.touched = 1;
        return true ;
    } 
    if( strcmp(tok, "GPGSV") == 0 ) {
        parse_tok(pktpoi);
        parse_tok(pktpoi);
        parse_sats_inview(pktpoi);
        nmea.touched = 1;
        return true ;
    }
    if( strcmp(tok, "GPGGA") == 0 ) {
        parse_tok(pktpoi); // time
        parse_tok(pktpoi); // lat
        parse_tok(pktpoi); // lat ind
        parse_tok(pktpoi); // lon
        parse_tok(pktpoi); // lon ind
        parse_tok(pktpoi); // quality
        parse_sats_used(pktpoi); // numSV
        nmea.touched = 1;
        return true ;
    }
    return false ;
}

