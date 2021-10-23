/*
 *   This file is part of MiniGUI, a mature cross-platform windowing 
 *   and Graphics User Interface (GUI) support system for embedded systems
 *   and smart IoT devices.
 * 
 *   Copyright (C) 2002~2018, Beijing FMSoft Technologies Co., Ltd.
 *   Copyright (C) 1998~2002, WEI Yongming
 * 
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *   Or,
 * 
 *   As this program is a library, any link to this program must follow
 *   GNU General Public License version 3 (GPLv3). If you cannot accept
 *   GPLv3, you need to be licensed from FMSoft.
 * 
 *   If you have got a commercial license of this program, please use it
 *   under the terms and conditions of the commercial license.
 * 
 *   For more information about the commercial license, please refer to
 *   <http://www.minigui.com/en/about/licensing-policy/>.
 */
/*
** mou_usb.c: Intelligent PS/2 Mouse Driver
**
** Create Date: 2002/10/15 
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

#include "common.h"
#include "ial.h"
#include "gal.h"
#include "native.h"

#ifdef _MGCONSOLE_USB

#define SCALE           3       /* default scaling factor for acceleration */
#define THRESH          5       /* default threshhold for acceleration */

/* The intialization string of imps/2 mouse, comes from
 * xc/program/Xserver/hw/xfree86/input/mouse/mouse.c
 */
static unsigned char USB_Param [] = {243,200,243,100,243,80};

static int  USB_Open (const char* mdev);
static void USB_Close (void);
static int  USB_GetButtonInfo (void);
static void USB_GetDefaultAccel (int *pscale, int *pthresh);
static int  USB_Read (int *dx, int *dy, int *dz, int *bp);
static void USB_Suspend (void);
static int USB_Resume (void);

MOUSEDEVICE mousedev_USB = {
    USB_Open,
    USB_Close,
    USB_GetButtonInfo,
    USB_GetDefaultAccel,
    USB_Read,
    USB_Suspend,
    USB_Resume
};

static int mouse_fd;

static int USB_Open (const char* mdev)
{
    mouse_fd = open (mdev, O_RDONLY | O_NONBLOCK);

    return mouse_fd;
}

static void USB_Close (void)
{
    if (mouse_fd > 0)
        close (mouse_fd);

    mouse_fd = -1;
}

static int USB_GetButtonInfo(void)
{
    return BUTTON_L | BUTTON_M | BUTTON_R | WHEEL_UP | WHEEL_DOWN;
}

static void USB_GetDefaultAccel(int *pscale, int *pthresh)
{
    *pscale = SCALE;
    *pthresh = THRESH;
}

static int USB_Read (int *dx, int *dy, int *dz, int *bp)
{
    struct input_event buftmp;
    int n;

    n=read(mouse_fd, &buftmp, sizeof(buftmp));
    if (n < 0) {
        if ((errno != EINTR) && (errno != EAGAIN))
            return 0;
        return -1;
    }

    if (buftmp.type == 2) {
        if (buftmp.code == 0)
            *dx = buftmp.value;
        else if (buftmp.code == 1)
            *dy = buftmp.value;
        else if (buftmp.code == 2)
            *dz = buftmp.value;
        return 1;
    }

    return 0;
}

static void USB_Suspend (void)
{
    USB_Close();
}

static int USB_Resume (void)
{
    return USB_Open (IAL_MDev);
}

#endif /* _MGCONSOLE_USB */

