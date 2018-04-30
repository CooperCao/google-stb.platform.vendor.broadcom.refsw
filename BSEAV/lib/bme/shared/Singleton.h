/***************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*  See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
***************************************************************************/

#ifndef BME_SINGLETON_HEADER
#define BME_SINGLETON_HEADER

namespace Broadcom
{
    template<class TInstance>
    class Singleton
    {
    public:
        virtual ~Singleton() {};

        static TInstance* getInstance() {
            // The C++11 standard already guarantees that static variables are initialized in a threadsafe manner
            // The threadsafety of the initialization can be found in �6.7.4 of the(C++11) standard:
            static TInstance instance;
            return &instance;
        }

        // Do not allow copying or assigning
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

    protected:
        Singleton() {} ;
    };
}

#endif /* BME_SINGLETON_HEADER */
