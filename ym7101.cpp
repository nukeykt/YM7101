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


int pin_ic;
int test_reset;

int reset_latch;
int dclk_state[2]; // MCLK / 4
int dclk2_state[3]; // MCLK / 5
int dclk3_state[6]; // MCLK / 15
int dclk4_state[4]; // MCLK / 7
int dclk5_state[6]; // MCLK / 12
int dclk;
int edclk;
int z80clock;
int m68kclock;
int subcar;

int pal;

int reg_test1[11];
int reg_8c[8];

int m68kclockdir = 0;

void VDP_ClockDot(void)
{
}

void VDP_Clock(void)
{
    int t;

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

    int reset = pin_ic | test_reset;
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
