/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_graphicsv3d.h"
#include "bstd.h"

#include "nxclient.h"

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>

#include <map>
#include <vector>
#include <chrono>
#include <thread>

typedef struct CPUData
{
   uint32_t utime;
   uint32_t stime;
   uint32_t cutime;
   uint32_t cstime;
   uint32_t threads;
   uint32_t vsize;
   uint32_t lastCpu;

   std::chrono::time_point<std::chrono::steady_clock> last;

   bool     alive;
} CPUData;

long                          g_ticksPerSec = 1;
std::map<uint32_t, CPUData>   g_cpuData;

uint32_t                      g_totGPU = 0;
uint32_t                      g_totCPU = 0;
uint32_t                      g_totMem = 0;
uint32_t                      g_totRenders = 0;
uint32_t                      g_totThreads = 0;

static float TicksToUs(uint32_t ticks)
{
   return (float)((float)(1000000 * ticks) / (float)g_ticksPerSec);
}

void ResetCPUStats(void)
{
   std::vector<uint32_t> toDelete;

   for (auto const & m : g_cpuData)
   {
      auto d = m.second;
      if (!d.alive)
         toDelete.push_back(m.first);
      d.alive = false;
   }

   for (auto & l : toDelete)
      g_cpuData.erase(l);
}

void UpdateCPUStats(uint32_t pid)
{
   auto &d = g_cpuData[pid];

   d.alive = true;

   char buf[1024];
   char name[128];

   sprintf(buf, "/proc/%d/stat", pid);

   auto now = std::chrono::steady_clock::now();
   auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(now - d.last).count();
   d.last = now;

   FILE *fp = fopen(buf, "r");
   buf[0] = '\0';
   if (fp != NULL)
   {
      fgets(buf, 1024, fp);
      buf[1023] = '\0';
      fclose(fp);
   }
   else
      return;

   auto utime = d.utime;
   auto stime = d.stime;

   if (sscanf(buf, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu %ld %ld %*ld %*ld %ld %*ld %*llu %*lu %*ld %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*d %d",
                   &d.utime, &d.stime, &d.cutime, &d.cstime, &d.threads, &d.lastCpu) != 6)
      return;

   sprintf(buf, "/proc/%d/status", pid);
   fp = fopen(buf, "r");
   buf[0] = '\0';
   if (fp != NULL)
   {
      while (fgets(buf, 1024, fp) != 0)
      {
         buf[1023] = '\0';

         sscanf(buf, "%s:", name);
         if (!strcmp(name, "VmSize:"))
         {
            sscanf(buf, "VmSize: %u %s", &d.vsize, name);
            break;
         }
      }

      fclose(fp);
   }
   else
      return;

   utime  = TicksToUs(d.utime - utime);
   stime  = TicksToUs(d.stime - stime);

   auto cputime = 100.0f * (utime + stime) / elapsedUs;
   auto percent = (uint32_t)(cputime + 0.5f);

   g_totCPU += percent;
   g_totMem += d.vsize;
   g_totThreads += d.threads;

   printf(" % 4u%%  % 4u   % 8u %s % 2u ", percent, d.threads, d.vsize, name, d.lastCpu);
}

int main(int __attribute__((unused)) argc, const char __attribute__((unused)) **argv)
{
   auto rc = NxClient_Join(NULL);
   if (rc != NEXUS_SUCCESS)
   {
      printf("Failed to join an existing server\n");
      return -1;
   }

   g_ticksPerSec = sysconf(_SC_CLK_TCK);

   NEXUS_Graphicsv3d_SetGatherLoadData(true);

   while (1)
   {
      ResetCPUStats();

      g_totCPU = g_totGPU = g_totMem = g_totThreads = g_totRenders = 0;

      // Clear screen & home
      printf("\033[2J");
      printf("\033[H");

      // Header
      printf("\033[7m                         V3D CLIENT LOAD                            \n");
      printf(" CID     PID  RDRS  GPU%%  CPU%% THREADS     VMEM    CPU COMMAND      \033[0m\n");

      // How many clients?
      uint32_t numClients(0);
      NEXUS_Graphicsv3d_GetLoadData(NULL, 0, &numClients);

      if (numClients > 0)
      {
         auto loadData = (NEXUS_Graphicsv3dClientLoadData*)malloc(numClients * sizeof(NEXUS_Graphicsv3dClientLoadData));

         NEXUS_Graphicsv3d_GetLoadData(loadData, numClients, &numClients);

         for (auto i = 0; i < numClients; i++)
         {
            char buf[1024];
            sprintf(buf, "/proc/%d/cmdline", loadData[i].uiClientPID);

            FILE *fp = fopen(buf, "r");
            buf[0] = '\0';
            if (fp != NULL)
            {
               fgets(buf, 1024, fp);
               buf[1023] = '\0';
               fclose(fp);
            }

            g_totGPU += loadData[i].sRenderPercent;
            g_totRenders += loadData[i].uiNumRenders;

            printf("% 4d ", loadData[i].uiClientId);
            printf(" % 6d", loadData[i].uiClientPID);
            printf("  % 4d", loadData[i].uiNumRenders);
            printf(" % 4u%%", loadData[i].sRenderPercent);

            UpdateCPUStats(loadData[i].uiClientPID);

            printf(" %s", buf);
            printf("\n");
         }
      }

      printf("\033[4m                                                                    \033[0m\n");
      printf("              % 4d % 4u%% % 4u%%  % 4u    % 7u kB\n", g_totRenders, g_totGPU, g_totCPU, g_totThreads, g_totMem);

      std::this_thread::sleep_for(std::chrono::seconds(1));
   }

   NEXUS_Platform_Uninit();

   return 0;
}
