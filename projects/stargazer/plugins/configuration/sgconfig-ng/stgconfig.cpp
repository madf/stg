#include <boost/thread.hpp>

#include "base_store.h"
#include "base_settings.h"
#include "common.h"

// TODO: Fix this shit!!!
#include "../../../users.h"
#include "../../../tariffs.h"
#include "../../../admins.h"
#include "../../../settings.h"

#include "stgconfig.h"

STGCONFIG2::STGCONFIG2()
    : users(NULL),
      tariffs(NULL),
      admins(NULL),
      store(NULL),
      stgSettings(NULL),
      ct(admins, tariffs, users, stgSettings)
{
    thread = new boost::thread;
}

STGCONFIG2::~STGCONFIG2()
{
    delete thread;
}

int STGCONFIG2::ParseSettings()
{
    return 0;
}

int STGCONFIG2::Start()
{
    ct.SetClasses(admins, tariffs, users, stgSettings);
    *thread = boost::thread(boost::ref(ct));
    return 0;
}

int STGCONFIG2::Stop()
{
    ct.Stop();
    if (!thread->timed_join(boost::get_system_time() + boost::posix_time::milliseconds(5000))) {
        thread->detach();
        printfd(__FILE__, "STGCONFIG2::Stop() Thread not stopped.\n");
        errorStr = "Failed to stop config thread.";
        return -1;
    }
    return 0;
}

bool STGCONFIG2::IsRunning()
{
    return true;
}
