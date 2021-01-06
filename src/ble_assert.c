#include <stdint.h>
#if INCLUDE_DEBUG_STATEMENTS
#include "debug.h"
#endif

void __ble_assert(const char *file, uint16_t line)
{
#if INCLUDE_DEBUG_STATEMENTS
    log_info("ERROR: %s, line %d\r\n", file, line);
#endif
}
