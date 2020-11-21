/* YM7101 VDP
 * Copyright (C) 2020 Nuke.YKT
 *
 * This file is part of YM7101 VDP.
 *
 * YM7101 VDP is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * YM7101 VDP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YM7101 VDP. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Yamaha YM7101 VDP
 *  Thanks:
 *      Fritzchens Fritz for YM7101 decap and die shot.
 *
 * version: 0.0
 */

#include <stdio.h>

int pin_ic;
int testf_write_signal;

int reset_latch;
int reset_latch2;
int reset_latch3[2];
int subclock1_state[2];
int subclock2_state[2];
int dclk_state[2]; // MCLK / 4
int dclk2_state[3]; // MCLK / 5
int dclk3_state[6]; // MCLK / 15
int dclk4_state[4]; // MCLK / 7
int dclk5_state[6]; // MCLK / 12
int dclk, odclk;
int edclk;
int z80clock;
int m68kclock;
int subcar;

int pal;

int reg_test1[11];
int reg_80[8];
int reg_81[8];
int reg_8c[8];

int m68kclockdir = 0;

int hcounter;
int vcounter;

int test2_write_signal;
int test3_write_signal;

int hreset_latch;
int hunk_latch;

#define M5 reg_81[2]
#define M2 reg_81[3]
#define H40 reg_8c[0]
#define LSM0 reg_8c[1]
#define LSM1 reg_8c[2]

int hv_unklatch1;

int hsync_pin;
int csync_pin;
int hsync_in;
int csync_in;
int hsync;
int csync;

int m68kbg_pin;

int vc_inc;
int vreset_latch;
int vunk_latch;

int databus;

void VDP_DoHVCounters(void)
{
    int reset = pin_ic | testf_write_signal;


    int vreset = test2_write_signal | reset | (vc_inc && vreset_latch) | vunk_latch;
    int vinc = reg_test1[2] ? m68kbg_pin : (vc_inc && !vreset);

    int unk1 = hv_unklatch1 == 1 && reg_80[0];
    int hreset = test3_write_signal | reset | hreset_latch | unk1;

    int unkbit1 = ((hsync_in && reg_8c[5]) || csync_in || (hv_unklatch1 & 1) != 0) && !(hunk_latch || reset);
    hv_unklatch1 <<= 1;
    hv_unklatch1 |= unkbit1;
    hv_unklatch1 &= 3;



    hsync_in = hsync_pin | hsync; // FIXME
    csync_in = csync_pin | csync; // FIXME

    hreset_latch = hcounter == (H40 ? 363 : 294);
    hunk_latch = hcounter == (H40 ? 253 : 206);

    vc_inc = hcounter == (M5 ? (H40 ? 328 : 264) : 488);
    vreset_latch = 0;
    int oddeven = 0; // TODO
    if (!pal)
    {
        if (!M5)
        {
            vreset_latch = vcounter == 218;
        }
        else if (!M2)
        {
            vreset_latch = vcounter == 234;
        }
    }
    else
    {
        if (!LSM0)
        {
            if (!M5)
            {
                vreset_latch = vcounter == 242;
            }
            else
            {
                if (!M2)
                {
                    vreset_latch = vcounter == 258;
                }
                else
                {
                    vreset_latch = vcounter == 266;
                }
            }
        }
        else
        {
            if (!M2)
            {
                vreset_latch = vcounter == 257;
            }
            else
            {
                vreset_latch = vcounter == 265;
            }
        }
    }

    if (vreset)
    {
        if (!test2_write_signal)
        {
#if 0
            int b0 = !pal ^ oddeven;
            int b1 = pal && !oddeven;
            int b2 = !pal;
            int b3 = pal && (!M5 || (M5 && !M2));
            int b4 = !(M5 && !M2);
            int b5 = pal ? !M5 : (M5 && !M2);
            int b6 = !(pal && !M5);
            int b7 = 1;
            int b8 = 1;
            vcounter = (b8<<8) |(b7<<7)|(b6<<6)|(b5<<5)|(b4<<4)|(b3<<3)|(b2<<2)|(b1<<1)|(b0<<0);
#endif
            if (!pal)
            {
                if (M5 && !M2)
                {
                    vcounter = 485;
                }
                else
                {
                    vcounter = 469;
                }
            }
            else
            {
                if (!M5)
                {
                    vcounter = 442;
                }
                else
                {
                    if (M2)
                    {
                        vcounter = 466;
                    }
                    else
                    {
                        vcounter = 458;
                    }
                }
            }
            vcounter -= oddeven;
        }
        else
        {
            vcounter = databus & 511;
        }
    }
    else if (vinc)
    {
        vcounter++;
    }
    vcounter &= 511;

    if (hreset)
    {
        if (!test3_write_signal)
        {
#if 0
            int b0 = M5 && H40 && !reg_80[0];
            int b1 = !reg_80[0] && !H40;
            int b2 = reg_80[0] && !H40;
            int b3 = (!reg_80[0] && M5 && H40) | (!H40 && reg_80[0] && M5);
            int b4 = reg_80[0] || !M5 || !H40;
            int b5 = 0;
            int b6 = 1;
            int b7 = 1;
            int b8 = 1;
            hcounter = (b8<<8) |(b7<<7)|(b6<<6)|(b5<<5)|(b4<<4)|(b3<<3)|(b2<<2)|(b1<<1)|(b0<<0);
#endif
            switch (M5 + H40 * 2 + reg_80[0] * 4)
            {
            case 0:
                hcounter = 466;
                break;
            case 1:
                hcounter = 466;
                break;
            case 2:
                hcounter = 464;
                break;
            case 3:
                hcounter = 457;
                break;
            case 4:
                hcounter = 468;
                break;
            case 5:
                hcounter = 476;
                break;
            case 6:
                hcounter = 464;
                break;
            case 7:
                hcounter = 464;
                break;
            }
        }
        else
        {
            hcounter = databus & 511;
        }
    }
    else
    {
        hcounter++;
    }
    hcounter &= 511;
}

void VDP_ClockDotSub(void)
{
    VDP_DoHVCounters();
}

void VDP_ClockDot(void)
{

    VDP_DoHVCounters();

    if (reset_latch3[1])
    {
        VDP_ClockDotSub();
    }

    reset_latch3[1] = reset_latch3[0];
    reset_latch3[0] ^= 1;

    if (reset_latch2 == 1)
    {
        reset_latch3[0] = 0;
    }
    int reset = pin_ic | testf_write_signal;
    reset_latch2 <<= 1;
    reset_latch2 |= reset;
    reset_latch2 &= 3;
}

void VDP_Clock(void)
{
    int t;

    if (dclk && !odclk)
    {
        VDP_ClockDot();
    }

    odclk = dclk;

    if (dclk2_state[2])
    {
        dclk3_state[0] = !(dclk3_state[3] | dclk3_state[1]);
        dclk3_state[2] = dclk3_state[1];
        dclk3_state[5] = dclk3_state[4];
    }
    else
    {
        dclk3_state[1] = dclk3_state[0];
        dclk3_state[3] = dclk3_state[2];
        dclk3_state[4] = dclk3_state[3];
    }

    if (dclk_state[1])
    {
        dclk5_state[0] = !(dclk5_state[3] | dclk5_state[1]);
        dclk5_state[2] = dclk5_state[1];
        dclk5_state[5] = dclk5_state[4];
    }
    else
    {
        dclk5_state[1] = dclk5_state[0];
        dclk5_state[3] = dclk5_state[2];
        dclk5_state[4] = dclk5_state[3];
    }

    t = dclk_state[1];
    dclk_state[1] = !dclk_state[0];
    dclk_state[0] = t; // MCLK / 4

    t = dclk2_state[2];
    dclk2_state[2] = !(dclk2_state[1] & dclk2_state[0]);
    dclk2_state[1] = dclk2_state[0];
    dclk2_state[0] = t; // MCLK / 5

    t = dclk4_state[3];
    dclk4_state[3] = dclk4_state[2];
    dclk4_state[2] = !(dclk4_state[0] & dclk4_state[1]);
    dclk4_state[1] = dclk4_state[0];
    dclk4_state[0] = t; // MCLK / 7

    if (reset_latch == 1)
    {
        dclk_state[0] = 0;
        dclk_state[1] = 0;
        dclk2_state[0] = 0;
        dclk2_state[1] = 0;
        dclk2_state[2] = 0;
        dclk3_state[0] = 0;
        dclk3_state[1] = 0;
        dclk3_state[2] = 0;
        dclk3_state[3] = 0;
        dclk3_state[4] = 0;
        dclk3_state[5] = 0;
        dclk4_state[0] = 0;
        dclk4_state[1] = 0;
        dclk4_state[2] = 0;
        dclk4_state[3] = 0;
        dclk2_state[0] = 0;
        dclk2_state[1] = 0;
        dclk2_state[2] = 0;
        dclk5_state[0] = 0;
        dclk5_state[1] = 0;
        dclk5_state[2] = 0;
        dclk5_state[3] = 0;
        dclk5_state[4] = 0;
        dclk5_state[5] = 0;
    }
    int dclkh40 = dclk_state[1];
    int dclkh32 = dclk2_state[2];
    
    if (reg_8c[7] || reg_test1[0]) // Use external dclk
    {
        dclk = reg_test1[1] ? 0 : edclk;
    }
    else
    {
        dclk = reg_8c[0] ? dclkh40 : dclkh32;
    }
    
    int z80c = dclk3_state[5] | dclk3_state[3];
    int palsubcar = dclk5_state[5] | dclk5_state[3];
    int m68kc = !dclk4_state[3];

    if (!m68kclockdir)
        m68kclock = m68kc;

    z80clock = reg_test1[0] ? m68kclock : z80c;
    subcar = pal ? palsubcar : z80c;

    int reset = pin_ic | testf_write_signal;
    reset_latch <<= 1;
    reset_latch |= reset;
    reset_latch &= 3;
}

void VDP_Update(int cycles)
{
    while (cycles--)
    {
        VDP_Clock();
    }
}


int main()
{
    for (int i = 0; i < 1000; i++)
    {
        VDP_Clock();
        printf("Clock: %i m68: %i z80: %i dclk %i car %i\n", i, m68kclock, z80clock, dclk, subcar);
    }
    return 0;
}
