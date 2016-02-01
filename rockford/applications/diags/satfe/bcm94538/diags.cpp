/******************************************************************************* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bstd.h"
//#include "bi2c.h"
#include "stdafx.h"
#include "windows.h"
#include "process.h"
#include "resource.h"
#include "satfe.h"
#include "satfe_platform.h"


SATFE_Diags_Config g_SATFE_Config;

extern "C" {
   extern uint8_t bcm4538_ap_image[];
   extern bool open_constellation_window(SATFE_Chip *pChip);
   extern bool open_plot_window(SATFE_Chip *pChip, int status_item, float min_value, float max_value);
};

#define CONSTELLATION_TIMER_EVENT 0x100
#define PLOT_TIMER_EVENT          0x110

static int PLOT_DATA_TYPE[] = {
   PLOT_DATA_UNSIGNED,
   PLOT_DATA_FLOAT,
   PLOT_DATA_FLOAT,
   PLOT_DATA_SIGNED,
   PLOT_DATA_UNSIGNED,
   PLOT_DATA_BOOL
};


/******************************************************************************
 bcm4538_read_firmware_file()
******************************************************************************/
bool bcm4538_read_firmware_file(const char *filename, uint8_t **pFirmwareImage)
{
   FILE *f;
   int line = 0, state, len, pass = 0;
   int len_2, len_1, len_0, addr_2, addr_1, addr_0;
   bool bRetVal = true;
   uint32_t addr, image_size;
   uint8_t *pImage = NULL, *pCurrPosition, sb;
   char r[256], *linestr, *token, *pstr;

   begin_pass:
   if ((f = fopen(filename, "rt")) == NULL)
   {
      printf("unable to open %s for reading\n", filename);
      return false;
   }

   state = 0;
   image_size = 0;
   while (!feof(f) && bRetVal)
   {
      line++;
      fgets(r, 256, f);
      linestr = r;

      /* skip to the first non whitespace */
      while ((*linestr == ' ') || (*linestr == '\t') || (*linestr == '\r') || (*linestr == '\n'))
      {
         linestr++;
      }

      if (strlen(linestr) == 0)
         continue;

      if (state == 0)
      {
         if (strstr(r, "bcm4538_ap_image"))
            state = 1;
      }
      else if (state == 1)
      {
         if (sscanf(linestr, "0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X", &len_2, &len_1, &len_0, &addr_2, &addr_1, &addr_0) != 6)
         {
            printf("ERROR in %s, line %d: 6 byte segment header expected (linestr=%s)\n", filename, line, linestr);
            bRetVal = false;
            goto done;
         }
         image_size += 6;
         len = (len_2 << 16) | (len_1 << 8) | len_0;
         addr = (addr_2 << 16) | (addr_1 << 8) | addr_0;
         image_size += len;
         if (pass)
         {
            *pCurrPosition++ = len_2;
            *pCurrPosition++ = len_1;
            *pCurrPosition++ = len_0;
            *pCurrPosition++ = addr_2;
            *pCurrPosition++ = addr_1;
            *pCurrPosition++ = addr_0;
         }
         if (len == 0)
         {
            state = 3;
            break;
         }
         else
            state = 2;
      }
      else if (state == 2)
      {
         /* skip over the comments */
         pstr = strstr(linestr, "*/");
         if (pstr)
            linestr = pstr + 3;

         token = strtok(linestr, " ,\t\n");
         while (token != NULL)
         {
            if (strncmp(token, "0x", 2))
            {
               printf("ERROR in %s, line %d: parsing error, len=%d\n", filename, line, len);
               bRetVal = false;
               goto done;
            }
            sb = (uint8_t)strtoul(token, NULL, 16);
            if (pass)
               *pCurrPosition++ = sb;
            token = strtok(NULL, " ,\t\n");
            len--;
         }
         if (len < 0)
         {
            printf("ERROR in %s, line %d: segment header expected\n", filename, line);
            bRetVal = false;
            goto done;
         }
         if (len == 0)
         {
            state = 1;
         }
      }
   }

   done:
   fclose(f);

   if (!bRetVal)
      goto done2;

   if (state != 3)
   {
      printf("ERROR in %s, line %d: no end-of-stream delimiter found\n", filename, line);
      bRetVal = false;
      goto done2;
   }

   if (pass == 0)
   {
/* printf("image_size=0x%X\n", image_size); */
      pImage = (uint8_t*)malloc(image_size + 1);
      pass = 1;
      pCurrPosition = pImage;
      goto begin_pass;
   }

   done2:
   if (bRetVal)
      *pFirmwareImage = pImage;
   else if (pImage)
      free(pImage);

   return bRetVal;
}


/******************************************************************************
 main()
******************************************************************************/
int main(int argc, char* argv[])
{
   int i, nRetCode = 0;
   uint8_t *p;
   bool bInitAp = true;
   BSPI_Handle hSpi;
   BSPI_ChannelHandle hSpiChan;

   g_SATFE_Config.init_filename[0] = 0;
   g_SATFE_Config.lpt_addr = LPT_ADDR_DEFAULT;
   g_SATFE_Config.pFirmware = bcm4538_ap_image;
   g_SATFE_Config.bSpi = 0;

   if (argc > 1)
   {
         for (i = 1; i < argc; i++)
         {
            if (!strcmp(argv[i], "-spi"))
               g_SATFE_Config.bSpi = 1;
            else if (!strcmp(argv[i], "-a"))
               bInitAp = false;
            else if ((!strcmp(argv[i], "-lpt1")) && ((i+1) < argc))
            {
               i++;
               g_SATFE_Config.lpt_addr = (uint16_t)strtoul(argv[i], NULL, 16);
            }
            else if (!strcmp(argv[i], "-f") && ((i+1) < argc))
            {
               i++;
               printf("reading %s...\n", argv[i]);
               if (bcm4538_read_firmware_file(argv[i], &p))
                  g_SATFE_Config.pFirmware = p;
               else
               {
                  printf("unable to process %s\n", argv[i]);
                  return -1;
               }
            }
            else if (!strcmp(argv[i], "-i") && ((i+1) < argc))
            {
               i++;
               strcpy(g_SATFE_Config.init_filename, argv[i]);
            }
            else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-?") || !strcmp(argv[i], "/?"))
            {
               printf("\n");
               printf("diags.exe - BCM94538 diagnostics application for Windows\n");
               printf("\n");
               printf("command line syntax:\n");
               printf("diags <-a> <-lpt1 lpt1_addr> <-spi> <-f fw_filename> <-i init_filename>\n");
               printf("All command-line flags are optional.\n");
               printf("-a = do not download firmware\n");
               printf("lpt1_addr = LPT1 parallel port address (default is 0x%04X)\n", LPT_ADDR_DEFAULT);
               printf("-spi = use SPI to communicate with BCM4538 (default is I2C)\n");
               printf("hex_filename = filename of Broadcom supplied BCM4538 firmware image file\n");
               printf("init_filename = file containing diags commands to execute on startup\n");
               printf("\n");
               return -1;
            }
            else
            {
               printf("syntax error\n");
               return -1;
            }
         }
      }

      printf("LPT1 address = 0x%X\n", g_SATFE_Config.lpt_addr);

      BKNI_Init();
      BDBG_Init();
      BDBG_SetLevel(BDBG_eErr);

      if (g_SATFE_Config.bSpi)
      {
         // instantiate SPI REG object
         printf("using SPI...\n");

         if (BSPI_Open(&hSpi, NULL, NULL, NULL, NULL) != BERR_SUCCESS)
         {
            printf("BSPI_Open() failed\n");
            return -1;
         }
         if (BSPI_OpenChannel(hSpi, &hSpiChan, 0, NULL) != BERR_SUCCESS)
         {
            printf("BI2C_OpenChannel() failed\n");
            return -1;
         }
         BSPI_CreateSpiRegHandle(hSpiChan, &(g_SATFE_Config.hRegSpi));
      }
      else
      {
         // instantiate I2C PI object
         if (BI2C_Open(&(g_SATFE_Config.hI2c), NULL, NULL, NULL, NULL) != BERR_SUCCESS)
         {
            printf("BI2C_Open() failed\n");
            return -1;
         }
         if (BI2C_OpenChannel(g_SATFE_Config.hI2c, &(g_SATFE_Config.hI2cChan), 0, NULL) != BERR_SUCCESS)
         {
            printf("BI2C_OpenChannel() failed\n");
            return -1;
         }
         BI2C_CreateI2cRegHandle(g_SATFE_Config.hI2cChan, &(g_SATFE_Config.hRegI2c));
      }

      if (SATFE_Diags((void*)&g_SATFE_Config, bInitAp ? 1 : 0))
	      SATFE_Shutdown();

      BI2C_CloseChannel(g_SATFE_Config.hI2cChan);
      BI2C_Close(g_SATFE_Config.hI2c);
      if (g_SATFE_Config.pFirmware != bcm4538_ap_image)
         free(g_SATFE_Config.pFirmware);
	return nRetCode;
}


/******************************************************************************
 ConstellationWindow()
******************************************************************************/
void ConstellationWindow( void *p )
{
   SATFE_Chip *pChip = (SATFE_Chip*)p;
   SATFE_94538_Impl *pImpl = (SATFE_94538_Impl*)pChip->pImpl;
   unsigned long chn = pChip->currChannel;
   MSG msg;
   HDC hDC, hDC1;
   RECT rc;
   PAINTSTRUCT ps;
   HPEN hpen, hpenOld;
   HBRUSH hbrush, hbrushOld;
   UINT_PTR timer_id;
   int i, n, timer_count, nPoints = 0;
   short iBuf[15], qBuf[15];
   char title[32];

   timer_id = NULL;
   sprintf(title, "Constellation (channel %d)", chn);
   pImpl->wndConstellation[chn] = ::CreateDialog(NULL,
   MAKEINTRESOURCE(IDD_CONSTELLATION),NULL,NULL);
   if (pImpl->wndConstellation[chn] != NULL )
   {
      // show dialog
      ::ShowWindow(pImpl->wndConstellation[chn],SW_SHOW);
   }
   else
      return;

   hDC1 = GetDC(pImpl->wndConstellation[chn]);
   GetClientRect(pImpl->wndConstellation[chn], (LPRECT)&rc );
   SetWindowText(pImpl->wndConstellation[chn], title);
   timer_id = SetTimer(pImpl->wndConstellation[chn], CONSTELLATION_TIMER_EVENT + chn, 10, NULL);
   pImpl->bConstellation[chn] = true;

   while (::GetMessage(&msg, pImpl->wndConstellation[chn], 0, 0) != 0)
   {
      //printf("msg=0x%x, lparam=0x%x, wparam=0x%x\n", msg.message, msg.lParam, msg.wParam);
      if ((msg.message == WM_TIMER) && (msg.wParam == (CONSTELLATION_TIMER_EVENT + chn)))
      {
         KillTimer(pImpl->wndConstellation[chn], timer_id);
         if (SATFE_GetSoftDecisions(pChip, chn, iBuf, qBuf))
         {
            for (i = 0; i < 15; i++)
            {
               SetPixelV(hDC1, iBuf[i] + 128, qBuf[i] + 128, 0x00FFFFFF);
               nPoints++;
            }
         }

#if 0
         if (nPoints > 500)
         {
            nPoints = 0;
            InvalidateRect(pImpl->wndConstellation[chn], &rc, true);
         }
#endif

         for (i = n = 0; i < 8; i++)
         {
            if (pImpl->bConstellation[i])
               n++;
         }
         timer_count = n*20;
         if (timer_count < 0)
            timer_count = 20;
         timer_id = SetTimer(pImpl->wndConstellation[chn], CONSTELLATION_TIMER_EVENT + chn, timer_count, NULL);
      }
      else if (msg.message == WM_PAINT)
      {
         hDC = BeginPaint(pImpl->wndConstellation[chn], &ps);
         hpen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
         hbrush = CreateSolidBrush(RGB(0, 0, 0));
         hpenOld = (HPEN)SelectObject(hDC, hpen);
         hbrushOld = (HBRUSH)SelectObject(hDC, hbrush);

         FillRect(hDC, &rc, hbrush);
         MoveToEx(hDC, 128, 0, NULL);
         LineTo(hDC, 128, 256);
         MoveToEx(hDC, 0, 128, NULL);
         LineTo(hDC, 256, 128);

         EndPaint(pImpl->wndConstellation[chn], &ps);
         SelectObject(hDC, hpenOld);
         DeleteObject(hpen);
         SelectObject(hDC, hbrushOld);
         DeleteObject(hbrush);
      }
      else if (msg.message == WM_LBUTTONDOWN)
         InvalidateRect(pImpl->wndConstellation[chn], &rc, true);
      else if ((msg.message == WM_COMMAND) && (msg.wParam == 2))
      {
         printf("Closing constellation window %d...\n", chn);
         ::DestroyWindow(pImpl->wndConstellation[chn]);
         break;
      }

      // process message
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
   }
   pImpl->bConstellation[chn] = false;
   pImpl->wndConstellation[chn] = NULL;
}


/******************************************************************************
 open_constellation_window()
******************************************************************************/
bool open_constellation_window(SATFE_Chip *pChip)
{
   SATFE_94538_Impl *pImpl = (SATFE_94538_Impl*)pChip->pImpl;
   uint8_t chn;

   chn = pChip->currChannel;
   if (pImpl->bConstellation[chn])
   {
      printf("Constellation window %d is already displayed!\n", chn);
      return true;
   }

   if (_beginthread(ConstellationWindow, 0, (void*)pChip) == -1)
   {
      printf("Failed to create constellation window %d", chn);
      return false;
   }

   return true;
}


/******************************************************************************
 PlotWindowUpdateScale()
******************************************************************************/
void PlotWindowUpdateScale(SATFE_94538_Impl *pImpl, uint32_t chn)
{
   static float default_min_value[] = {0.0, 0.0, -80.0, -12000000.0, 0, 0};
   static float default_max_value[] = {0.0, 30.0, -10.0, 12000000.0, (float)0xffffffff, 1};
   int i, j;
   float x, g;

   for (i = 0; i < 3; i++)
   {
      if (pImpl->plotStatus[chn][i].item == PLOT_ITEM_NONE)
         continue;
      if (pImpl->plotStatus[chn][i].min == pImpl->plotStatus[chn][i].max)
      {
         pImpl->plotStatus[chn][i].min = default_min_value[pImpl->plotStatus[chn][i].item];
         pImpl->plotStatus[chn][i].max = default_max_value[pImpl->plotStatus[chn][i].item];
      }

      x = (pImpl->plotStatus[chn][i].max - pImpl->plotStatus[chn][i].min) / 4;
      for (j = 0; j <= 4; j++)
      {
         g = pImpl->plotStatus[chn][i].min + (x * j);
         switch (pImpl->plotStatus[chn][i].type)
         {
            case PLOT_DATA_FLOAT:
               sprintf(pImpl->plotStatus[chn][i].grid_label[j], "%.2f", g);
               break;

            case PLOT_DATA_BOOL:
               if (j == 0)
                  strcpy(pImpl->plotStatus[chn][i].grid_label[j], "0");
               else if (j == 1)
                  strcpy(pImpl->plotStatus[chn][i].grid_label[j], "1");
               else
                  pImpl->plotStatus[chn][i].grid_label[j][0] = 0;
               break;

            case PLOT_DATA_UNSIGNED:
               sprintf(pImpl->plotStatus[chn][i].grid_label[j], "%u", g);
               break;

            case PLOT_DATA_SIGNED:
               sprintf(pImpl->plotStatus[chn][i].grid_label[j], "%d", g);
               break;

            default:
               pImpl->plotStatus[chn][i].grid_label[j][0] = 0;
               break;
         }
      }
   }
}


/******************************************************************************
 PlotWindow()
******************************************************************************/
void PlotWindow( void *p )
{
#define PLOT_WINDOW_BOTTOM_MARGIN 25
#define PLOT_WINDOW_WIDTH 512
#define PLOT_WINDOW_HEIGHT 256
#define PLOT_WINDOW_MID_HEIGHT (PLOT_WINDOW_HEIGHT / 2)
#define PLOT_WINDOW_QUARTER_HEIGHT (PLOT_WINDOW_HEIGHT / 4)

   SATFE_Chip *pChip = (SATFE_Chip*)p;
   SATFE_94538_Impl *pImpl = (SATFE_94538_Impl*)pChip->pImpl;
   unsigned long chn = pChip->currChannel;
   float plot_value[3];
   MSG msg;
   HDC hDC, hDC1;
   RECT rc, rc2, rc3;
   PAINTSTRUCT ps;
   HPEN hpen, hpenOld;
   HBRUSH hbrush, hbrushOld;
   UINT_PTR timer_id;
   int timer_count, y[3], last_y[3], i, j, statusItem;
   bool bUpdate, b;
   char title[64], str[16];

   static char *status_str[] = {"", "SNR", "InputPower", "Carrier Error", "", "lock"};
   static char *status_unit[] = {"", "dB", "dBm", "Hz", "", ""};
   static COLORREF pen_color[] = {RGB(255, 0, 0), RGB(60, 200, 60), RGB(0, 255, 255)};

   timer_id = NULL;
   sprintf(title, "Plot (channel %d)", chn);
   pImpl->wndPlot[chn] = ::CreateDialog(NULL, MAKEINTRESOURCE(IDD_PLOT),NULL,NULL);
   if (pImpl->wndPlot[chn] != NULL )
   {
      // show dialog
      ::ShowWindow(pImpl->wndPlot[chn],SW_SHOW);
   }
   else
      return;

   PlotWindowUpdateScale(pImpl, chn);

   for (i = 0; i < 3; i++)
   {
      y[i] = PLOT_WINDOW_MID_HEIGHT;
      last_y[i] = PLOT_WINDOW_MID_HEIGHT;
   }

   hDC1 = GetDC(pImpl->wndPlot[chn]);
   GetClientRect(pImpl->wndPlot[chn], (LPRECT)&rc );
   SetWindowText(pImpl->wndPlot[chn], title);
   timer_id = SetTimer(pImpl->wndPlot[chn], PLOT_TIMER_EVENT + chn, 10, NULL);
   pImpl->bPlot[chn] = true;

   /* rc2 is the plotting rect */
   rc2.left = 0;
   rc2.top = 0;
   rc2.right = PLOT_WINDOW_WIDTH + 4;
   rc2.bottom = rc.bottom - PLOT_WINDOW_BOTTOM_MARGIN + 6;

   while (::GetMessage(&msg, pImpl->wndPlot[chn], 0, 0) != 0)
   {
//if (msg.message!=0x113) printf("msg=0x%x, lparam=0x%x, wparam=0x%x\n", msg.message, msg.lParam, msg.wParam);
      if (msg.message == WM_USER_RESCALE)
      {
         PlotWindowUpdateScale(pImpl, chn);
         InvalidateRect(pImpl->wndPlot[chn], NULL, true);
      }
      else if ((msg.message == WM_TIMER) && (msg.wParam == (PLOT_TIMER_EVENT + chn)))
      {
         KillTimer(pImpl->wndPlot[chn], timer_id);

         bUpdate = false;
         for (i = 0; i < 3; i++)
         {
            statusItem = -1;
            switch (pImpl->plotStatus[chn][i].item)
            {
               case PLOT_ITEM_INPUT_POWER:
                  statusItem = SATFE_STATUS_INPUT_POWER;
                  break;

               case PLOT_ITEM_SNR:
                  statusItem = SATFE_STATUS_SNR;
                  break;

               case PLOT_ITEM_CARRIER_ERROR:
                  statusItem = SATFE_STATUS_CARRIER_ERROR;
                  break;

               case PLOT_ITEM_LOCK:
                  statusItem = SATFE_STATUS_LOCK;
                  break;

               case PLOT_ITEM_REGISTER:
                  //TBD...
                  break;

               default:
                  break;
            }

            if (statusItem == -1)
               continue;

            bUpdate = true;
            if (pImpl->plotStatus[chn][i].type == PLOT_DATA_BOOL)
            {
               SATFE_GetStatusItem(pChip, chn, statusItem, (void*)&b);
               if (b)
                  plot_value[i] = 1.0;
               else
                  plot_value[i] = 0;
            }
            else
               SATFE_GetStatusItem(pChip, chn, statusItem, (void*)&plot_value[i]);

            /* scale the status value for plotting */
            if (pImpl->plotStatus[chn][i].type == PLOT_DATA_BOOL)
            {
               if (plot_value[i])
                  y[i] = PLOT_WINDOW_HEIGHT - PLOT_WINDOW_QUARTER_HEIGHT;
               else
                  y[i] = PLOT_WINDOW_HEIGHT - 1;
            }
            else
            {
               y[i] = (int)(PLOT_WINDOW_HEIGHT - ((float)PLOT_WINDOW_HEIGHT * (plot_value[i] - pImpl->plotStatus[chn][i].min)) / (pImpl->plotStatus[chn][i].max - pImpl->plotStatus[chn][i].min));
               if (y[i] < 0)
                  y[i] = 0;
               else if (y[i] >= PLOT_WINDOW_HEIGHT)
                  y[i] = PLOT_WINDOW_HEIGHT - 1;
            }
         }

         if (bUpdate)
         {
            ScrollWindowEx(pImpl->wndPlot[chn], -1, 0, &rc2, &rc2, NULL, NULL, SW_INVALIDATE);

            /* draw grid lines */
            for (i = 0; i <= 4; i++)
               SetPixelV(hDC1, PLOT_WINDOW_WIDTH, PLOT_WINDOW_QUARTER_HEIGHT * i, 0x00FF0000);

            for (i = 0; i < 3; i++)
            {
               if (pImpl->plotStatus[chn][i].item == PLOT_ITEM_NONE)
                  continue;

               hpen = CreatePen(PS_SOLID, 1, pen_color[i]);
               hpenOld = (HPEN)SelectObject(hDC1, hpen);

               /* display current value */
               switch (pImpl->plotStatus[chn][i].type)
               {
                  case PLOT_DATA_BOOL:
                     sprintf(str, "%s=%d", status_str[pImpl->plotStatus[chn][i].item], (uint32_t)plot_value[i]);
                     break;

                  case PLOT_DATA_FLOAT:
                     sprintf(str, "%s=%.2f %s", status_str[pImpl->plotStatus[chn][i].item], plot_value[i], status_unit[pImpl->plotStatus[chn][i].item]);
                     break;

                  case PLOT_DATA_SIGNED:
                     sprintf(str, "%s=%d %s", status_str[pImpl->plotStatus[chn][i].item], (int32_t)plot_value[i], status_unit[pImpl->plotStatus[chn][i].item]);
                     break;

                  case PLOT_DATA_UNSIGNED:
                     sprintf(str, "%s=%u %s", status_str[pImpl->plotStatus[chn][i].item], (uint32_t)plot_value[i], status_unit[pImpl->plotStatus[chn][i].item]);
                     break;

                  default:
                     str[0] = 0;
                     break;
               }
               rc3.left = i * 170;
               rc3.bottom = rc.bottom;
               rc3.top = rc3.bottom - PLOT_WINDOW_BOTTOM_MARGIN;
               rc3.right = rc3.left + 190;

               SetTextColor(hDC1, pen_color[i]);
               DrawText(hDC1, str, -1, &rc3, DT_SINGLELINE | DT_LEFT | DT_BOTTOM);

               /* draw the line */
               MoveToEx(hDC1, PLOT_WINDOW_WIDTH-1, last_y[i], NULL);
               LineTo(hDC1, PLOT_WINDOW_WIDTH, y[i]);

               SelectObject(hDC1, hpenOld);
               DeleteObject(hpen);

               last_y[i] = y[i];
            }

            UpdateWindow(pImpl->wndPlot[chn]);
         }

         /* schedule the next plot update */
         if (pImpl->bPlot[chn ^ 1] || pImpl->bConstellation[0] || pImpl->bConstellation[1])
            timer_count = 50;
         else
            timer_count = 20;
         timer_id = SetTimer(pImpl->wndPlot[chn], PLOT_TIMER_EVENT + chn, timer_count, NULL);
      }
      else if (msg.message == WM_PAINT)
      {
         hDC = BeginPaint(pImpl->wndPlot[chn], &ps);
         hpen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
         hbrush = CreateSolidBrush(RGB(255, 255, 255));
         hpenOld = (HPEN)SelectObject(hDC, hpen);
         hbrushOld = (HBRUSH)SelectObject(hDC, hbrush);

         FillRect(hDC, &rc2, hbrush);

         for (j = 0; j < 3; j++)
         {
            if (pImpl->plotStatus[chn][j].item == PLOT_ITEM_NONE)
               continue;

            for (i = 0; i <= 4; i++)
            {
               MoveToEx(hDC, 0, PLOT_WINDOW_QUARTER_HEIGHT * i, NULL);
               LineTo(hDC, PLOT_WINDOW_WIDTH, PLOT_WINDOW_QUARTER_HEIGHT * i);
               rc3.left = (rc2.right + 4) + (j*50);
               rc3.right = rc3.left + 40;
               if (i == 0)
               {
                  rc3.bottom = PLOT_WINDOW_HEIGHT;
                  rc3.top = rc3.bottom - 16;
               }
               else
               {
                  rc3.top = PLOT_WINDOW_QUARTER_HEIGHT * (4 - i);
                  if (i != 4)
                     rc3.top -= 8;
                  rc3.bottom = rc3.top + 16;
               }
               SetTextColor(hDC, pen_color[j]);
               DrawText(hDC, pImpl->plotStatus[chn][j].grid_label[i], -1, &rc3, DT_SINGLELINE | DT_LEFT);
            }
         }

         EndPaint(pImpl->wndPlot[chn], &ps);
         SelectObject(hDC, hpenOld);
         DeleteObject(hpen);
         SelectObject(hDC, hbrushOld);
         DeleteObject(hbrush);
      }
      else if (msg.message == WM_LBUTTONDOWN)
      {
         InvalidateRect(pImpl->wndPlot[chn], &rc, true);
      }
      else if ((msg.message == WM_COMMAND) && (msg.wParam == 2))
      {
         printf("Closing plot window %d...\n", chn);
         ::DestroyWindow(pImpl->wndPlot[chn]);
         break;
      }

      // process message
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
   }
   pImpl->bPlot[chn] = false;
   pImpl->wndPlot[chn] = NULL;
}


/******************************************************************************
 open_plot_window()
******************************************************************************/
bool open_plot_window(SATFE_Chip *pChip, int status_item, float min_value, float max_value)
{
   SATFE_94538_Impl *pImpl = (SATFE_94538_Impl*)pChip->pImpl;
   int i, iAvail;
   bool bFound;
   uint8_t chn;

   chn = pChip->currChannel;
   if (pImpl->bPlot[chn] == false)
   {
      for (i = 0; i < 3; i++)
         pImpl->plotStatus[chn][i].item = PLOT_ITEM_NONE;

      i = iAvail = 0;
   }
   else
   {
      bFound = false;
      for (i = 0; (!bFound && (i < 3)); i++)
      {
         if (pImpl->plotStatus[chn][i].item == PLOT_ITEM_NONE)
         {
            iAvail = i;
            break;
         }
         else if (pImpl->plotStatus[chn][i].item == status_item)
         {
            bFound = true;
            break;
         }
      }
      if (!bFound)
         i = iAvail;
   }

   pImpl->plotStatus[chn][i].item = status_item;
   pImpl->plotStatus[chn][i].min = min_value;
   pImpl->plotStatus[chn][i].max = max_value;
   pImpl->plotStatus[chn][i].type = PLOT_DATA_TYPE[status_item];

   if (pImpl->bPlot[chn])
   {
      ::PostMessage(pImpl->wndPlot[chn], WM_USER_RESCALE, 0, 0);
   }
   else if (_beginthread(PlotWindow, 0, (void*)pChip) == -1)
   {
      printf("Failed to create plot window %d", chn);
      return false;
   }

   return true;
}
