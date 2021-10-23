/*
** $Id: rockchip_key.h  7249 2010-03-24 03:38:27Z czzhao $
**
** cisco_touchpad.h: the head file of Low Level Input Engine for 
**             Cisco touchpad 
**
** Copyright (C) 2007 ~ 2011 Feynman Software.
**
** Created by Zhao Chengzhang, 2010/03/24
*/

#ifndef _IAL_RKKeybroad_H
#define _IAL_RKKeybroad_KEY_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define CMD_SET_MODE     1
#define MICE_MODE        0
#define APP_MODE         1

BOOL    InitRKInput (INPUT* input, const char* mdev, const char* mtype);
void    TermRKInput (void);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _IAL_ROCKCHIP_KEY_H*/
