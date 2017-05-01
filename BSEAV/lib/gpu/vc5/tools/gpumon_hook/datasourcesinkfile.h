/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once


#include "datasource.h"
#include "datasink.h"

/* Unbuffered file IO from/to a file descriptor */
class DataSourceSinkFile: public DataSource, public DataSink
{
public:
   DataSourceSinkFile(int fd = -1, bool fsync = false) :
         m_fd(fd), m_fsync(fsync)
   {
   }
   virtual ~DataSourceSinkFile() = default;
   DataSourceSinkFile(const DataSourceSinkFile &) = delete;
   DataSourceSinkFile &operator=(const DataSourceSinkFile &) = delete;

   void SetFd(int fd, bool fsync = false) { m_fd = fd; m_fsync = fsync; }

public: //DataSource interface
   virtual size_t Read(void *data, size_t size) override;

public: //DataSink interface
   virtual size_t Write(const void *data, size_t size) override;
   virtual bool Flush() override;

private:
   int m_fd;
   bool m_fsync;
};
