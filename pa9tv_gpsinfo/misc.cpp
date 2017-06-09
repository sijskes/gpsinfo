/*
 *  misc.cpp
 * 
 *  Created on May 7, 2017 9:24:18 PM by Simon IJskes
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

#include "gpsinfo.h"

#include <stddef.h>
#include <limits.h>

#define NM_BUFSIZE 82

char nm_buf0[NM_BUFSIZE];
char nm_buf1[NM_BUFSIZE];

char *nm_buf = nm_buf0 ;
char *nm_buf_poi = NULL ;
char *nm_buf_rd = NULL ;

void hal_background()
{
    int c = hal_gps_in();
    
    if( c < 0 ) {
        return ;
    }
    
    if( c == '\n' ) {
      return ;
    }

    if( nm_buf_poi == NULL ) {
        if( c != '$' ) {
            return;
        }
        
//        if( nm_buf_rd != NULL ) {
//            // double buffer overflow.
//            return ;
//        }

        nm_buf_poi = nm_buf;
        return;
    }
    
    if( nm_buf_poi >= (nm_buf + NM_BUFSIZE) ) { 
        // overflow, reset
        nm_buf_poi = NULL;
        return ;
    }
    
    if( c == '\r' ) {
        *nm_buf_poi++ = 0;
        nm_buf_rd = nm_buf ;
        if( nm_buf == nm_buf0 ) {
            nm_buf = nm_buf1 ;
        } else {
            nm_buf = nm_buf0 ;            
        }
        nm_buf_poi = NULL;
        return;
    }

    *nm_buf_poi++ = c;
}

static long timer_start ;

void timer_reset()
{
    timer_start = hal_millis();
}

long timer_count()
{
    long m = hal_millis();
    long count = m - timer_start;
    if( count < 0 ) {
        count += LONG_MAX;
        count++;        
    }
    return count ;
}


