/*
 * This is a every simple sample for MiniGUI.
 * It will create a main window and display a string of "Hello, world!" in it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "common.h"

LRESULT PreDefDialogProc_ex(HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam)
{
    HWND hCurFocus;

    switch (message)
    {
#if 0
    case MSG_CREATE:
    {
        int i;
        PCTRLDATA pCtrlData;
        HWND hCtrl;

        PDLGTEMPLATE pDlgTmpl
            = (PDLGTEMPLATE)(((PMAINWINCREATE)lParam)->dwReserved);

        for (i = 0; i < pDlgTmpl->controlnr; i++)
        {
            pCtrlData = pDlgTmpl->controls + i;
            if (pCtrlData->class_name)
            {
                hCtrl = CreateWindowEx2(pCtrlData->class_name,
                                        pCtrlData->caption,
                                        pCtrlData->dwStyle | WS_CHILD,
                                        pCtrlData->dwExStyle,
                                        pCtrlData->id,
                                        pCtrlData->x,
                                        pCtrlData->y,
                                        pCtrlData->w,
                                        pCtrlData->h,
                                        hWnd,
                                        pCtrlData->werdr_name,
                                        pCtrlData->we_attrs,
                                        pCtrlData->dwAddData);
            }
            else
                break;

            if (hCtrl == HWND_INVALID)
            {
                dlgDestroyAllControls(hWnd);
                return -1;
            }
        }

        return 0;
    }
#endif
    case MSG_DLG_GETDEFID:
    {
        HWND hDef;

        hDef = GetDlgDefPushButton(hWnd);
        if (hDef)
            return GetDlgCtrlID(hDef);
        return 0;
    }

    case MSG_DLG_SETDEFID:
    {
        HWND hOldDef;
        HWND hNewDef;

        hNewDef = GetDlgItem(hWnd, wParam);
        if (SendMessage(hNewDef, MSG_GETDLGCODE, 0, 0L) & DLGC_PUSHBUTTON)
        {
            hOldDef = GetDlgDefPushButton(hWnd);
            if (hOldDef)
            {
                ExcludeWindowStyle(hOldDef, BS_DEFPUSHBUTTON);
                InvalidateRect(hOldDef, NULL, TRUE);
            }
            IncludeWindowStyle(hNewDef, BS_DEFPUSHBUTTON);
            InvalidateRect(hNewDef, NULL, TRUE);

            return (LRESULT)hOldDef;
        }
        break;
    }
    case MSG_COMMAND:
        if (wParam == IDCANCEL)
        {
            HWND hCancel;

            hCancel = GetDlgItem(hWnd, IDCANCEL);
            if (hCancel && IsWindowEnabled(hCancel)
                    && IsWindowVisible(hCancel))
                EndDialog(hWnd, IDCANCEL);
        }
        break;

    case MSG_DESTROY:
    {
        HWND hCancel;

        hCancel = GetDlgItem(hWnd, IDCANCEL);
        if (hCancel && IsWindowEnabled(hCancel)
                && IsWindowVisible(hCancel))
            EndDialog(hWnd, IDCANCEL);

        return 0;
    }

    case MSG_ISDIALOG:
        return 1;
    case MSG_KEYLONGPRESS:
        break;
    case MSG_KEYDOWN:
        if ((hCurFocus = GetFocusChild(hWnd))
                && SendMessage(hCurFocus, MSG_GETDLGCODE, 0, 0L) &
                DLGC_WANTALLKEYS)
            break;

        switch (wParam)
        {
        case KEY_EXIT_FUNC:
            SendMessage(hWnd, MSG_COMMAND, IDCANCEL, 0L);
            return 0;
        case KEY_DOWN_FUNC:
        {
            HWND hNewFocus;

            if (hCurFocus && SendMessage(hCurFocus, MSG_GETDLGCODE, 0, 0L) &
                    DLGC_WANTTAB)
                break;

            if (lParam & KS_SHIFT)
                hNewFocus = GetNextDlgTabItem(hWnd, hCurFocus, TRUE);
            else
                hNewFocus = GetNextDlgTabItem(hWnd, hCurFocus, FALSE);

            if (hNewFocus != hCurFocus)
            {
                SetNullFocus(hCurFocus);
                SetFocus(hNewFocus);
#if 0
                SendMessage(hWnd, MSG_DLG_SETDEFID,
                            GetDlgCtrlID(hNewFocus), 0L);
#endif
            }

            return 0;
        }
        case KEY_UP_FUNC:
        {
            HWND hNewFocus;

            if (hCurFocus && SendMessage(hCurFocus, MSG_GETDLGCODE, 0, 0L) &
                    DLGC_WANTTAB)
                break;

            if (lParam & KS_SHIFT)
                hNewFocus = GetNextDlgTabItem(hWnd, hCurFocus, FALSE);
            else
                hNewFocus = GetNextDlgTabItem(hWnd, hCurFocus, TRUE);

            if (hNewFocus != hCurFocus)
            {
                SetNullFocus(hCurFocus);
                SetFocus(hNewFocus);
#if 0
                SendMessage(hWnd, MSG_DLG_SETDEFID,
                            GetDlgCtrlID(hNewFocus), 0L);
#endif
            }
            return 0;
        }
        case KEY_ENTER_FUNC:
        {
            HWND hDef;

            if (hCurFocus && SendMessage(hCurFocus, MSG_GETDLGCODE, 0, 0L) &
                    DLGC_WANTENTER)
                break;

            if (SendMessage(hCurFocus, MSG_GETDLGCODE, 0, 0L) &
                    DLGC_PUSHBUTTON)
                break;

            hDef = GetDlgDefPushButton(hWnd);
            /* DK[07/05/10]Fix Bug4798, Check the control if has WS_DISABLED property. */
            if (hDef && IsWindowEnabled(hDef))
            {
                SendMessage(hWnd, MSG_COMMAND, GetDlgCtrlID(hDef), 0L);
                return 0;
            }
        }
        }
        break;

    default:
        break;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

static LRESULT MsgBoxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case MSG_INITDIALOG:
    {
        HWND hFocus = GetDlgDefPushButton(hWnd);
        if (hFocus)
            SetFocus(hFocus);

        SetWindowAdditionalData(hWnd, (DWORD)lParam);
        SetWindowFont(hWnd, logfont);

        /* set the messagebox's body text font and color.*/
        hFocus = GetDlgItem(hWnd, IDC_STATIC + 100);
        SetWindowFont(hFocus, logfont);

        SetWindowElementAttr(hFocus, WE_FGC_WINDOW,
                             GetWindowElementAttr(hFocus, WE_FGC_MESSAGEBOX));
        return 0;
    }

    case MSG_COMMAND:
    {
        switch (wParam)
        {
        case IDOK:
        case IDCANCEL:
        case IDABORT:
        case IDRETRY:
        case IDIGNORE:
        case IDYES:
        case IDNO:
            if (GetDlgItem(hWnd, wParam))
                EndDialog(hWnd, wParam);
            break;
        }
        break;
    }

    case MSG_CHAR:
    {
        int id = 0;

        if (HIBYTE(wParam))
            break;
        switch (LOBYTE(wParam))
        {
        case 'Y':
        case 'y':
            id = IDYES;
            break;
        case 'N':
        case 'n':
            id = IDNO;
            break;
        case 'A':
        case 'a':
            id = IDABORT;
            break;
        case 'R':
        case 'r':
            id = IDRETRY;
            break;
        case 'I':
        case 'i':
            id = IDIGNORE;
            break;
        }

        if (id != 0 && GetDlgItem(hWnd, id))
            EndDialog(hWnd, id);
        break;
    }

    case MSG_DESTROY:
        if (GetDlgItem(hWnd, IDCANCEL))
        {
            EndDialog(hWnd, IDCANCEL);
        }
        else if (GetDlgItem(hWnd, IDIGNORE))
        {
            EndDialog(hWnd, IDIGNORE);
        }
        else if (GetDlgItem(hWnd, IDNO))
        {
            EndDialog(hWnd, IDNO);
        }
        else if (GetDlgItem(hWnd, IDOK))
        {
            EndDialog(hWnd, IDOK);
        }
        break;
    default:
        break;
    }
    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

static void get_box_xy(HWND hParentWnd, DWORD dwStyle, DLGTEMPLATE *MsgBoxData)
{
    RECT rcTemp;

    if (dwStyle & MB_BASEDONPARENT)
    {
        GetWindowRect(hParentWnd, &rcTemp);
    }
    else
    {
        rcTemp = g_rcDesktop;
    }

    switch (dwStyle & MB_ALIGNMASK)
    {
    case MB_ALIGNCENTER:
        MsgBoxData->x = rcTemp.left + (RECTW(rcTemp) - MsgBoxData->w) / 2;
        MsgBoxData->y = rcTemp.top + (RECTH(rcTemp) - MsgBoxData->h) / 2;
        break;

    case MB_ALIGNTOPLEFT:
        MsgBoxData->x = rcTemp.left;
        MsgBoxData->y = rcTemp.top;
        break;

    case MB_ALIGNBTMLEFT:
        MsgBoxData->x = rcTemp.left;
        MsgBoxData->y = rcTemp.bottom - MsgBoxData->h;
        break;

    case MB_ALIGNTOPRIGHT:
        MsgBoxData->x = rcTemp.right - MsgBoxData->w;
        MsgBoxData->y = rcTemp.top;
        break;

    case MB_ALIGNBTMRIGHT:
        MsgBoxData->x = rcTemp.right - MsgBoxData->w;
        MsgBoxData->y = rcTemp.bottom - MsgBoxData->h;
        break;
    }

    if ((MsgBoxData->x + MsgBoxData->w) > g_rcDesktop.right)
    {
        MsgBoxData->x = g_rcDesktop.right - MsgBoxData->w;
    }

    if ((MsgBoxData->y + MsgBoxData->h) > g_rcDesktop.bottom)
    {
        MsgBoxData->y = g_rcDesktop.bottom - MsgBoxData->h;
    }
}

int MessageBox_ex(HWND hParentWnd, const char *pszText,
                  const char *pszCaption, DWORD dwStyle)
{
    BOOL IsTiny;
    DLGTEMPLATE MsgBoxData =
    {
        WS_CAPTION | WS_BORDER,
        WS_EX_NONE, 0, 0, 0, 0, NULL, 0, 0, 0, NULL, 0L
    };
    CTRLDATA     CtrlData [5] =
    {
        {
            "button",
            BS_PUSHBUTTON | WS_TABSTOP | WS_VISIBLE | WS_GROUP,
            0, 0, 0, 0, 0, NULL, 0L
        },
        {
            "button",
            BS_PUSHBUTTON | WS_TABSTOP | WS_VISIBLE,
            0, 0, 0, 0, 0, NULL, 0L
        },
        {
            "button",
            BS_PUSHBUTTON | WS_TABSTOP | WS_VISIBLE,
            0, 0, 0, 0, 0, NULL, 0L
        }
    };

    int i, nButtons, buttonx, iBorder;
    RECT rcText, rcButtons, rcIcon;
    int width, height;
    int mb_margin, mb_buttonw, mb_buttonh, mb_textw;

    __mg_def_proc[1] = PreDefDialogProc_ex;
    IsTiny = !strcasecmp(GetDefaultWindowElementRenderer()->name, "tiny");
    if (IsTiny)
    {
        int font_size = GetSysFontHeight(SYSLOGFONT_CONTROL);
        mb_margin  =  2;
        mb_buttonw =  font_size * 3;
        mb_buttonh =  font_size + 6;
        mb_textw   =  120;
    }
    else
    {
        int font_size = GetSysFontHeight(SYSLOGFONT_CONTROL);
        mb_margin  = font_size;
        mb_buttonw = font_size * 4;
        mb_buttonh = (font_size < 8) ? (font_size + 6) : (font_size * 3 / 2);
        mb_textw   = 300;
    }

    /* use system font height to determine the width and height of button */

    if (pszCaption)
        MsgBoxData.caption  = pszCaption;
    else
        MsgBoxData.caption  = "";

    switch (dwStyle & MB_TYPEMASK)
    {
    case MB_OK:
        MsgBoxData.controlnr = 1;
        CtrlData [0].caption = res_str[RES_STR_OK];
        CtrlData [0].id      = IDOK;
        break;
    case MB_OKCANCEL:
        MsgBoxData.controlnr = 2;
        CtrlData [0].caption = res_str[RES_STR_OK];
        CtrlData [0].id      = IDOK;
        CtrlData [1].caption = res_str[RES_STR_CANCEL];
        CtrlData [1].id      = IDCANCEL;
        break;
    case MB_YESNO:
        MsgBoxData.controlnr = 2;
        CtrlData [0].caption = res_str[RES_STR_YES];
        CtrlData [0].id      = IDYES;
        CtrlData [1].caption = res_str[RES_STR_NO];
        CtrlData [1].id      = IDNO;
        break;
    case MB_RETRYCANCEL:
        MsgBoxData.controlnr = 2;
        CtrlData [0].caption = GetSysText(IDS_MGST_RETRY);
        CtrlData [0].id      = IDRETRY;
        CtrlData [1].caption = res_str[RES_STR_CANCEL];
        CtrlData [1].id      = IDCANCEL;
        break;
    case MB_ABORTRETRYIGNORE:
        MsgBoxData.controlnr = 3;
        CtrlData [0].caption = GetSysText(IDS_MGST_ABORT);
        CtrlData [0].id      = IDABORT;
        CtrlData [1].caption = GetSysText(IDS_MGST_RETRY);
        CtrlData [1].id      = IDRETRY;
        CtrlData [2].caption = GetSysText(IDS_MGST_IGNORE);
        CtrlData [2].id      = IDIGNORE;
        break;
    case MB_YESNOCANCEL:
        MsgBoxData.controlnr = 3;
        CtrlData [0].caption = res_str[RES_STR_YES];
        CtrlData [0].id      = IDYES;
        CtrlData [1].caption = res_str[RES_STR_NO];
        CtrlData [1].id      = IDNO;
        CtrlData [2].caption = res_str[RES_STR_CANCEL];
        CtrlData [2].id      = IDCANCEL;
        break;
    }

    switch (dwStyle & MB_DEFMASK)
    {
    case MB_DEFBUTTON1:
        CtrlData [0].dwStyle |= BS_DEFPUSHBUTTON;
        break;
    case MB_DEFBUTTON2:
        if (MsgBoxData.controlnr > 1)
            CtrlData [1].dwStyle |= BS_DEFPUSHBUTTON;
        break;
    case MB_DEFBUTTON3:
        if (MsgBoxData.controlnr > 2)
            CtrlData [2].dwStyle |= BS_DEFPUSHBUTTON;
        break;
    }

    nButtons = MsgBoxData.controlnr;
    rcButtons.left   = 0;
    rcButtons.top    = 0;
    rcButtons.bottom = mb_buttonh;
    rcButtons.right  = MsgBoxData.controlnr * mb_buttonw +
                       (MsgBoxData.controlnr - 1) * (mb_margin << 1);

    rcIcon.left   = 0;
    rcIcon.top    = 0;
    rcIcon.right  = 0;
    rcIcon.bottom = 0;
    if (dwStyle & MB_ICONMASK)
    {
        int id_icon = -1;
        i = MsgBoxData.controlnr;
        CtrlData [i].class_name = "static";
        CtrlData [i].dwStyle   = WS_VISIBLE | SS_ICON | WS_GROUP;
        CtrlData [i].x         = mb_margin;
        CtrlData [i].y         = mb_margin;

        if (IsTiny)
        {
            CtrlData [i].w     = 16;
            CtrlData [i].h     = 16;
        }
        else
        {
            CtrlData [i].w     = 32;
            CtrlData [i].h     = 32;
        }

        CtrlData [i].id        = IDC_STATIC;
        CtrlData [i].caption   = "";

        switch (dwStyle & MB_ICONMASK)
        {
        case MB_ICONSTOP:
            id_icon = IDI_STOP;
            break;
        case MB_ICONINFORMATION:
            id_icon = IDI_INFORMATION;
            break;
        case MB_ICONEXCLAMATION:
            id_icon = IDI_EXCLAMATION;
            break;
        case MB_ICONQUESTION:
            id_icon = IDI_QUESTION;
            break;
        }

        if (IsTiny)
        {
            if (id_icon != -1)
            {
                CtrlData [i].dwAddData = (DWORD)GetSmallSystemIcon(id_icon);
            }
            rcIcon.right  = 16;
            rcIcon.bottom = 16;
        }
        else
        {
            if (id_icon != -1)
            {
                CtrlData [i].dwAddData = (DWORD)GetLargeSystemIcon(id_icon);
                MsgBoxData.hIcon       = GetSmallSystemIcon(id_icon);
            }
            rcIcon.right  = 32;
            rcIcon.bottom = 32;
        }

        MsgBoxData.controlnr ++;
    }

    rcText.left = 0;
    rcText.top  = 0;
    //rcText.right = rcButtons.right + (mb_margin << 1);
    //rcText.right = MAX (rcText.right, mb_textw);
    rcText.right = LCD_W / 2;
    rcText.bottom = GetSysCharHeight() * 2;

    SelectFont(HDC_SCREEN, logfont);

    //DrawText (HDC_SCREEN, pszText, -1, &rcText,
    //        DT_LEFT | DT_TOP | DT_CHARBREAK | DT_EXPANDTABS | DT_CALCRECT);

    if (IsTiny)
        rcText.right = MAX(rcText.right, mb_textw);

    i = MsgBoxData.controlnr;
    CtrlData [i].class_name = "static";
    CtrlData [i].dwStyle   = WS_VISIBLE | SS_LEFT | WS_GROUP;
    CtrlData [i].dwExStyle = 0;
    CtrlData [i].x         = RECTW(rcIcon) + (mb_margin << 1);
    CtrlData [i].y         = mb_margin;
    CtrlData [i].w         = RECTW(rcText);
    CtrlData [i].h         = RECTH(rcText);
    CtrlData [i].id        = IDC_STATIC + 100;
    CtrlData [i].caption   = pszText;
    CtrlData [i].dwAddData = 0;

    MsgBoxData.controlnr ++;

    iBorder = 1 + 2 * GetWindowElementAttr(hParentWnd, WE_METRICS_WND_BORDER);

    width = MAX(RECTW(rcText), RECTW(rcButtons)) + RECTW(rcIcon)
            + (mb_margin << 2)
            + (iBorder << 1);
    height = MAX(RECTH(rcText), RECTH(rcIcon)) + RECTH(rcButtons)
             + mb_margin + (mb_margin << 1)
             + (iBorder << 1)
             + GetWindowElementAttr(hParentWnd, WE_METRICS_CAPTION);

    buttonx = (width - RECTW(rcButtons)) >> 1;
    for (i = 0; i < nButtons; i++)
    {
        CtrlData[i].x = buttonx + i * (mb_buttonw + mb_margin);
        CtrlData[i].y = MAX(RECTH(rcIcon), RECTH(rcText)) + (mb_margin << 1);
        CtrlData[i].w = mb_buttonw;
        CtrlData[i].h = mb_buttonh;
    }

    MsgBoxData.w = width;
    MsgBoxData.h = height;
    get_box_xy(hParentWnd, dwStyle, &MsgBoxData);

    MsgBoxData.controls = CtrlData;

    printf("0.0\n");
    printf("0.0\n");
    printf("0.0\n");

    return DialogBoxIndirectParam(&MsgBoxData, hParentWnd, MsgBoxProc,
                                  (LPARAM)dwStyle);
}
