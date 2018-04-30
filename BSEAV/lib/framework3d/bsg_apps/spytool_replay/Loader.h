/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __LOADER_H__
#define __LOADER_H__

#define BSG_NO_NAME_MANGLING

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mutex>
#include <vector>

#include "Command.h"
#include "bsg_task.h"
#include "datasource.h"


#ifdef HAS_UNZIP
#include "unzip.h"
#endif

class SpyToolReplay;

// The latest version of the binary capture format we support
#define CAPTURE_MAJOR_VER 1
#define CAPTURE_MINOR_VER 6

class Loader;

class LoaderTask : public bsg::Task
{
public:
   LoaderTask(Loader &loader);

   virtual void OnThread();
   virtual void OnCallback(bool finished);

private:
   Loader   &m_loader;
};

class Loader: private DataSource
{
public:
   Loader(SpyToolReplay *replay);
   ~Loader();

   bool Open(const std::string &filename, uint32_t bufferBytes, uint32_t ioBufferBytes, uint32_t maxCmds, bool reprime);
   bool LoadCommand(Command **cmd);

   friend class LoaderTask;

private: //DataSource interface
   size_t Read(void *buf, size_t count);

private:
   bool    ReadCommand(Command *cmd);
   void    PrimeBuffer();
   bool    FillBuffer(bool print);
   bool    CaptureHasPointerSize();

private:
   enum
   {
      CHUNK = 32768
   };

   SpyToolReplay                    *m_replay;
   FILE                             *m_fp;
   uint32_t                         m_major;
   uint32_t                         m_minor;
   uint32_t                         m_bufferLen;
   uint8_t                          *m_buffer;
   uint32_t                         m_readBytes;
   uint32_t                         m_cmdQueueBytes;
   bool                             m_taskDone;
   uint32_t                         m_insertAt;
   uint32_t                         m_takeAt;
   uint32_t                         m_numCmds;
   uint32_t                         m_bufferBytes;
   uint32_t                         m_ioBufferBytes;
   uint32_t                         m_maxCmds;
   bool                             m_reprime;
   uint8_t                          *m_readPtr;
   std::vector<SpecializedCommand>  m_cmdQueue;
   mutable std::mutex               m_queueMutex;
   LoaderTask                       *m_loaderTask;
   bsg::Tasker                      *m_tasker;

#ifdef HAS_UNZIP
   bool                 m_zipFile;
   unzFile              m_zipFP;
#endif
};

#endif /* __LOADER_H__ */
