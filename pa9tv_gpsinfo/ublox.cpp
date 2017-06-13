/*
 *  ublox.cpp
 * 
 *  Created on Jun 3, 2017 3:20:04 PM by Simon IJskes
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

#include <stdlib.h>
#include "printf.h"

const int gnss_len = 0x34;
const uint8_t gnss[gnss_len] ROM = {
    0xB5, 0x62, 0x06, 0x3E, 0x2C, 0x00, 0x00, 0x00, 0x20, 0x05, 0x00, 0x08, 0x10, 0x00, 0x01, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x08, 0x10, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x01, 0x06, 0x08, 0x0E, 0x00, 0x00, 0x00,
    0x01, 0x01, 0xFC, 0x11
};

const int nav5_len = 0x2c;
const uint8_t nav5[nav5_len] ROM = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27,
    0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x3C, 0x00, 0x00,
    0x00, 0x00, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xA0
};

const int pms_len = 0x10;
const uint8_t pms[pms_len] ROM = {
    0xB5, 0x62, 0x06, 0x86, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x5A
};

const int rxm_len = 0xa;
const uint8_t rxm[rxm_len] ROM = {
    0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x00, 0x21, 0x91
};

const int tp5_len = 0x28;
#if 0
// 4 hz / 10 Mhz
const uint8_t tp5[tp5_len] ROM = {
    0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x04, 0x00,
    0x00, 0x00, 0x80, 0x96, 0x98, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x6F, 0x00, 0x00, 0x00, 0xAB, 0x50
};
#endif
const uint8_t tp5[tp5_len] ROM = {
    0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x00, 0x01, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x80, 0x96,
    0x98, 0x00, 0x80, 0x96, 0x98, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x6F, 0x00, 0x00, 0x00, 0x55, 0x7a
};

static const uint8_t pattern[] ROM = {
    0xB5, 0x62, 0x05, 0x01
};

int patidx = 0;
int ack_seen = 0;

void ublox_poll()
{
    int c = hal_gps_in();

    if( c < 0 ) {
        return ;
    }
    
    uint8_t c8 = c ;
    
    if( patidx == 3 ) {
        if( c8 == 0 ) {
            ack_seen = -1 ;
        } else if( c8 == 1 ) {
            ack_seen = 1 ;
        }
        patidx = 0;
        return ;
    }

    if( c8 == pgm_read_byte_near(pattern + patidx) ) {
        patidx++;
    }
    
//
//    if( patidx >= 4 ) {
//        ack_seen = 1;
//        patidx = 0;
//    }
}

void ublox_send(int len, const uint8_t msg[] ROM, bool warn)
{
    for(int attempt = 0; attempt < 3; attempt++) {
        timer_reset();
        ack_seen = 0;
        for(int i = 0; i < len; i++) {
            uint8_t c = pgm_read_byte_near(msg + i);
            LOG_C_H(c);
            hal_gps_out(c);
            ublox_poll();
        }
        LOG_C('\n');
        while (timer_count() < 2000) {
            // loop.
            ublox_poll();
            if( ack_seen > 0 ) {
                hal_disp_out('+');
                return;
            }
            if( ack_seen < 0 ) {
                if( warn ) {
                    hal_disp_out('-');
                    return;
                }                
            }
        }
        hal_disp_out('!');
    }
    printf("\vGPSINFO PA9TV\n");
    printf("initialization error\n"); 
    while(1) { /*forever*/} 
}

void ublox_init()
{
#ifdef PA0JBB_UBLOX_INIT
    hal_disp_out('G');
    ublox_send(gnss_len, gnss, 0);
    hal_disp_out('N');
    ublox_send(nav5_len, nav5, 0);
    hal_disp_out('P');
    ublox_send(pms_len, pms, 1);
    hal_disp_out('R');
    ublox_send(rxm_len, rxm, 0);
    hal_disp_out('T');
    ublox_send(tp5_len, tp5, 0);
    hal_disp_out('r');

    timer_reset();
    while (timer_count() < 2000) {
      // nothing, just wait.
    }    
#endif    
}


