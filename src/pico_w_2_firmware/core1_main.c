#include "common.h"
#include "main.h"
#include <pico/multicore.h>
#include "pico/util/queue.h"

void core1_entry()
{

    // multicore_fifo_push_blocking(FLAG_VALUE);

    // uint32_t g = multicore_fifo_pop_blocking();

    while (1)
        tight_loop_contents();
}