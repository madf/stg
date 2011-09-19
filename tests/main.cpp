#include "tut/tut.hpp"
#include "tut/tut_reporter.hpp"

namespace tut
{
    test_runner_singleton runner;
}

volatile time_t stgTime = 0;

int main()
{
    tut::reporter reporter;
    tut::runner.get().set_callback(&reporter);

    tut::runner.get().run_tests();

    return !reporter.all_ok();
}
