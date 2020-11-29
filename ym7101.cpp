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
#define LCB reg_80[5]
#define DISP reg_81[6]

int hv_unklatch1;

int hsync_pin;
int csync_pin;
int hsync_in;
int csync_in;
int hsync;
int csync;

int m68kbg_pin;
int m68kintak_pin;

int vc_inc;
int vreset_latch;
int vunk_latch;

int oddeven;

int databus;

int hv_unklatch2;
int hv_unklatch3;
int hv_unklatch4;
int hv_unklatch5;
int hv_unklatch6;
int hv_unklatch7;
int hv_unklatch8;
int hv_unklatch9;
int hv_unklatch10;
int hv_unklatch11;
int hv_unklatch12;
int hv_unklatch13;
int hv_unklatch14;
int hv_unklatch15;
int hv_unklatch16;
int hv_unklatch17;
int hv_unklatch18;
int hv_unklatch19;
int hv_unklatch20;
int hv_unklatch21;
int hv_unklatch22;
int hv_unklatch23;
int hv_unklatch24;
int hv_unklatch25;
int hv_unklatch26;
int hv_unklatch27;
int hv_unklatch28;
int hv_unklatch29;
int hv_unklatch30;
int hv_unktrig1;
int hv_unktrig2;
int hv_unktrig3;
int hv_unktrig4;
int hv_unktrig5;
int hv_unktrig6;

int hv_unklatch31;
int hv_unklatch32;
int hv_unklatch33;

int vsync;

void VDP_DoHVCounters(void)
{
    int reset = pin_ic | testf_write_signal;

    int unkbit3 = (vunk_latch == 1) && reg_80[0];

    int vreset = test2_write_signal | reset | (vc_inc && vreset_latch) | unkbit3;
    int vinc = reg_test1[2] ? m68kbg_pin : (vc_inc && !vreset);

    int unk1 = hv_unklatch1 == 1 && reg_80[0];
    int hreset = test3_write_signal | reset | hreset_latch | unk1;
    int hinc = reg_test1[3] ? m68kintak_pin : !hreset;

    int unkbit12 = hreset && reg_80[0];
    int unkbit13 = hv_unklatch31 || unkbit12;

    int unkbit1 = ((hsync_in && reg_8c[5]) || csync_in || (hv_unklatch1 & 1) != 0) && !((hunk_latch & 1) != 0 || reset);
    hv_unklatch1 <<= 1;
    hv_unklatch1 |= unkbit1;
    hv_unklatch1 &= 3;

    hv_unktrig1 |= unkbit3 && (hv_unklatch5 & 2) != 0;
    if ((unkbit3 && (hunk_latch & 2) != 0) || reset)
        hv_unktrig1 = 0;

    hv_unktrig2 |= hv_unklatch7;
    if (hv_unklatch3 || reset)
        hv_unktrig2 = 0;

    int unkbit6 = hv_unklatch13 || !oddeven;
    int unkbit7 = unkbit6 && hv_unklatch6;

    hv_unktrig3 |= unkbit7 || reset;
    if (hv_unklatch8)
        hv_unktrig3 = 0;
    int unkbit9 = unkbit6 && hv_unklatch9;

    hv_unktrig4 |= unkbit7;
    if (unkbit9 || reset)
        hv_unktrig4 = 0;

    hv_unktrig5 |= unkbit9 || reset;
    if (hv_unklatch10 && unkbit6)
        hv_unktrig5 = 0;

    hv_unktrig6 |= hv_unklatch10 && unkbit6;
    if ((hv_unklatch11 && unkbit6) || reset)
        hv_unktrig6 = 0;



    int vactive = (hv_unklatch12 || hv_unktrig2) && DISP;

    int unkbit10 = reg_80[0] && !reg_8c[4];
    int unkbit11 = unkbit10 && !reg_81[0];

    int s1 = 0;
    if (vactive && M5 && ((hcounter & 319) == 50) || (H40 && hcounter == 306))
        s1 = 1;
    if (!vactive && (hcounter & 63) == 50)
        s1 = 1;
    int s2 = 0;

    if (vactive && ((hcounter & 271) == 0 || (H40 && M5 && (hcounter & 463) == 256) || (!M5 && (hcounter & 271) == 8) || (M5 && hcounter== 496)))
        s2 = 1;

    int s3 = 0;
    if (vactive && ((hcounter & 271) == 8 || (H40 && M5 && (hcounter & 463) == 264) || (M5 && hcounter == 504)))
        s3 = 1;

    int s4 = 0;
    if (vactive && ((hcounter & 263) == 4 || (H40 && M5 && (hcounter & 455) == 260) || (M5 && hcounter == 500) || (M5 && hcounter == 508)))
        s4 = 1;

    int s5 = 0;
    if (vactive && ((hcounter & 263) == 6 || (H40 && M5 && (hcounter & 455) == 262) || (M5 && hcounter == 502) || (M5 && hcounter == 510)))
        s5 = 1;

    int s6 = 0;
    if (vactive && ((M5 && (hcounter & 271) == 10) || (M5 && H40 && (hcounter & 463) == 458)
        || (!M5 && ((hcounter & 509) == 264 || (hcounter & 509) == 276 || (hcounter & 509) == 468 || (hcounter & 509) == 480))))
        s6 = 1;

    int s7 = 0;
    if (vactive && ((M5 && H40 && hcounter == 484) || (M5 && H40 && hcounter == 460) || (M5 && H40 && hcounter == 458)
        || (M5 && H40 && !unkbit11 && (hcounter & 505) == 344)
        || (M5 && H40 && !unkbit10 && hcounter == 364)
        || (M5 && H40 && !unkbit10 && (hcounter & 509) == 360)
        || (M5 && H40 && !unkbit10 && (hcounter & 505) == 352)
        || (M5 && H40 && (hcounter & 505) == 336)
        || (M5 && H40 && (hcounter & 505) == 328)
        || (M5 && H40 && (hcounter & 509) == 324)
        || (M5 && !H40 && !unkbit10 && hcounter == 290)
        || (M5 && !H40 && !unkbit10 && (hcounter & 509) == 292)
        || (M5 && !H40 && !unkbit11 && (hcounter & 505) == 280)
        || (M5 && !H40 && (hcounter & 505) == 264)
        || (M5 && !H40 && (hcounter & 509) == 260)
        || (M5 && !H40 && (hcounter & 505) == 272)
        // || (M5 && hcounter == 486)
        || (M5 && (hcounter & 503) == 498)
        || (M5 && (hcounter & 505) == 488)
        || (M5 && (hcounter & 509) == 480)
        || (M5 && (hcounter & 497) == 464)
        // || (!M5 && (hcounter & 509) == 488)
        // || (!M5 && (hcounter & 509) == 476)
        // || (!M5 && (hcounter & 509) == 284)
        || (!M5 && (hcounter & 509) == 484)
        || (!M5 && (hcounter & 509) == 472)
        || (!M5 && (hcounter & 509) == 280)
        || (!M5 && (hcounter & 509) == 268)))
        s7 = 1;

    int s8 = (hcounter & 1) == 1;
    int s9 = 0;
    if (vactive && !M5 && (hcounter == 488 || hcounter == 484 || (hcounter & 507) == 472 || (hcounter & 503) == 272 || (hcounter & 495) == 268))
        s9 = 1;

    int s10 = 0;
    if (vactive && !M5 && ((hcounter & 509) == 480 || (hcounter & 509) == 468 || (hcounter & 509) == 276 || (hcounter & 509) == 264))
        s10 = 1;

    int s11 = 0;

    if (unkbit13)
        s11 = 1;

    if (vactive && !M5 && ((hcounter & 279) == 18 || (hcounter & 287) == 10 || (hcounter & 497) == 496))
        s11 = 1;

    int s12 = 0;
    if (vactive && ((M5 && H40 && hcounter == 484) || (M5 && H40 && hcounter == 460) || (M5 && H40 && hcounter == 458)
        || (M5 && H40 && !unkbit11 && (hcounter & 505) == 344)
        || (M5 && H40 && !unkbit10 && hcounter == 364)
        || (M5 && H40 && !unkbit10 && (hcounter & 509) == 360)
        || (M5 && H40 && !unkbit10 && (hcounter & 505) == 352)
        || (M5 && H40 && (hcounter & 505) == 336)
        || (M5 && !H40 && !unkbit10 && hcounter == 290)
        || (M5 && !H40 && !unkbit10 && (hcounter & 509) == 292)
        || (M5 && !H40 && !unkbit11 && (hcounter & 505) == 280)
        || (M5 && !H40 && (hcounter & 505) == 272)
        || (M5 && hcounter == 486)
        || (M5 && (hcounter & 503) == 498)
        || (M5 && (hcounter & 505) == 488)
        || (M5 && (hcounter & 509) == 480)
        || (M5 && (hcounter & 497) == 464)
        || (!M5 && (hcounter & 279) == 18)
        || (!M5 && (hcounter & 287) == 10)
        || (!M5 && (hcounter & 497) == 496)
        || (M5 && hcounter == 496)
        || (M5 && hcounter == 504)
        || (M5 && hcounter == 500)
        || (M5 && hcounter == 508)
        || (M5 && hcounter == 502)))
        s12 = 1;
    
    int s13 = 0;
    if (vactive && M5 && hcounter == 486)
        s13 = 1;

    int s14 = 0;
    if (vactive && ((!M5 && (hcounter & 509) == 488)
        || (!M5 && (hcounter & 509) == 476)
        || (!M5 && (hcounter & 509) == 284)
        || (!M5 && (hcounter & 509) == 272)))
        s14 = 1;

    hv_unklatch33 = !(!hv_unklatch32 && s8);

    hv_unklatch31 = unkbit12;
    hv_unklatch32 = unkbit13;


    int unkbit5 = ((hv_unklatch5 & 1) != 0 || (hunk_latch & 1) != 0) && csync_in;
    int unkbit4 = ((vunk_latch & 1) != 0 || unkbit5) && (!hv_unklatch8 && !reset);

    vunk_latch <<= 1;
    vunk_latch |= unkbit4;
    vunk_latch &= 3;


    int unkbit2 = hv_unklatch3 && hv_unklatch2;
    int oeinc = unkbit2 && !reg_80[0];
    int oereset = reset || !LSM0 || (!hv_unktrig1 && unkbit2 && reg_80[0]);
    int oeone = hv_unklatch4;

    hv_unklatch4 = (unkbit2 && reg_80[0] && hv_unktrig1);

    hsync_in = hsync_pin | hsync; // FIXME
    csync_in = csync_pin | csync; // FIXME

    hreset_latch = hcounter == (H40 ? 363 : 294);
    hunk_latch <<= 1;
    hunk_latch |= hcounter == (H40 ? 253 : 206);
    hv_unklatch2 = hcounter == 0;
    hv_unklatch5 <<= 1;
    hv_unklatch5 |= hcounter == (H40 ? 43 : 36);

    vc_inc = hcounter == (M5 ? (H40 ? 328 : 264) : 488);
    vreset_latch = 0;
    hv_unklatch8 = 0;
    hv_unklatch6 = 0;

    hv_unklatch13 = hcounter == (H40 ? 120 : 95);
    hv_unklatch14 = hcounter == 18;
    if (!LCB && hcounter == 10)
        hv_unklatch14 = 1;

    hv_unklatch15 = hcounter == (H40 ? 330 : 266);
    hv_unklatch16 = hcounter == (H40 ? 348 : 284);
    hv_unklatch17 = hcounter == (H40 ? 1 : 0);
    hv_unklatch18 = hcounter == (H40 ? 120 : 95);
    hv_unklatch19 = hcounter == (H40 ? 148 : 121);
    if (H40)
    {
        hv_unklatch20 = hcounter == 164 || hcounter == 466;
    }
    else
    {
        hv_unklatch20 = hcounter == 134 || hcounter == 475;
    }
    hv_unklatch21 = hcounter == (H40 ? 358 : 292);
    hv_unklatch22 = hcounter == (H40 ? 482 : 488);
    hv_unklatch23 = hcounter == (H40 ? 322 : 258);
    hv_unklatch24 = (hcounter & 7) == 0;
    hv_unklatch25 = (hcounter & 15) == 7;
    if (!M5 && (hcounter & 15) == 15)
        hv_unklatch25 |= 1;

    hv_unklatch26 = (hcounter & 15) == 15;
    hv_unklatch27 = 0;
    if (!M5 && (hcounter == 267 || hcounter == 279 || hcounter == 471 || hcounter == 483))
        hv_unklatch27 = 1;
    hv_unklatch28 = 0;
    if (M5 && (((hcounter & 271) == 13) || (H40 && ((hcounter & 463) == 269))))
        hv_unklatch28 = 1;
    hv_unklatch29 = (hcounter == 507);
    hv_unklatch30 = ((hcounter & 239) == 227);

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
    if (!M5)
    {
        hv_unklatch3 = vcounter == 192;
    }
    else if (!M2)
    {
        hv_unklatch3 = vcounter == 224;
    }
    else
    {
        hv_unklatch3 = vcounter == 240;
    }

    if (!pal)
    {
        if (!M5)
        {
            hv_unklatch6 = vcounter == 216;
        }
        else if (!M2)
        {
            hv_unklatch6 = vcounter == 232;
        }
    }
    else
    {
        if (!LSM0)
        {
            if (!M5)
            {
                hv_unklatch6 = vcounter == 240;
            }
            else
            {
                if (!M2)
                {
                    hv_unklatch6 = vcounter == 256;
                }
                else
                {
                    hv_unklatch6 = vcounter == 264;
                }
            }
        }
        else
        {
            if (!M2)
            {
                hv_unklatch6 = vcounter == 255;
            }
            else
            {
                hv_unklatch6 = vcounter == 263;
            }
        }
    }
    hv_unklatch7 = vcounter == 0;
    if (!pal)
    {
        if (!M5)
        {
            hv_unklatch8 = vcounter == 485;
        }
        else if (!M2)
        {
            hv_unklatch8 = vcounter == 501;
        }
    }
    else
    {
        if (!M5)
        {
            hv_unklatch8 = vcounter == 458;
        }
        else
        {
            if (!M2)
            {
                hv_unklatch8 = vcounter == 474;
            }
            else
            {
                hv_unklatch8 = vcounter == 482;
            }
        }
    }
    hv_unklatch9 = 0;
    if (!oddeven)
    {
        if (!pal)
        {
            if (!M5)
            {
                hv_unklatch9 = vcounter == 469;
            }
            else if (!M2)
            {
                hv_unklatch9 = vcounter == 485;
            }
        }
        else
        {
            if (!M5)
            {
                hv_unklatch9 = vcounter == 442;
            }
            else
            {
                if (!M2)
                {
                    hv_unklatch9 = vcounter == 458;
                }
                else
                {
                    hv_unklatch9 = vcounter == 466;
                }
            }
        }
    }
    else
    {
        if (!pal)
        {
            if (M5 && !M2)
            {
                hv_unklatch9 = vcounter == 484;
            }
        }
        else
        {
            if (M5)
            {
                if (!M2)
                {
                    hv_unklatch9 = vcounter == 457;
                }
                else
                {
                    hv_unklatch9 = vcounter == 465;
                }
            }
        }
    }
    hv_unklatch10 = 0;
    if (!oddeven)
    {
        if (!pal)
        {
            if (!M5)
            {
                hv_unklatch10 = vcounter == 472;
            }
            else if (!M2)
            {
                hv_unklatch10 = vcounter == 488;
            }
        }
        else
        {
            if (!M5)
            {
                hv_unklatch10 = vcounter == 445;
            }
            else
            {
                if (!M2)
                {
                    hv_unklatch10 = vcounter == 461;
                }
                else
                {
                    hv_unklatch10 = vcounter == 469;
                }
            }
        }
    }
    else
    {
        if (!pal)
        {
            if (M5 && !M2)
            {
                hv_unklatch10 = vcounter == 487;
            }
        }
        else
        {
            if (M5)
            {
                if (!M2)
                {
                    hv_unklatch10 = vcounter == 460;
                }
                else
                {
                    hv_unklatch10 = vcounter == 468;
                }
            }
        }
    }
    hv_unklatch11 = 0;
    if (!oddeven)
    {
        if (!pal)
        {
            if (!M5)
            {
                hv_unklatch11 = vcounter == 475;
            }
            else if (!M2)
            {
                hv_unklatch11 = vcounter == 491;
            }
        }
        else
        {
            if (!M5)
            {
                hv_unklatch11 = vcounter == 448;
            }
            else
            {
                if (!M2)
                {
                    hv_unklatch11 = vcounter == 464;
                }
                else
                {
                    hv_unklatch11 = vcounter == 472;
                }
            }
        }
    }
    else
    {
        if (!pal)
        {
            if (M5 && !M2)
            {
                hv_unklatch11 = vcounter == 490;
            }
        }
        else
        {
            if (M5)
            {
                if (!M2)
                {
                    hv_unklatch11 = vcounter == 463;
                }
                else
                {
                    hv_unklatch11 = vcounter == 471;
                }
            }
        }
    }
    hv_unklatch12 = vcounter == 511;

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
    else if (hinc)
    {
        hcounter++;
    }
    hcounter &= 511;

    if (oeone)
        oddeven = 1;
    oddeven = (oddeven + oeinc) & 1;
    if (oereset)
        oddeven = 0;

    vsync = hv_unktrig5;
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
