#include <sdk.h>
#include <macros.h>


// Fix underwater hovering when all water particles are used
SMS_WRITE_32(SMS_PORT_REGION(0x8026cd84, 0, 0, 0), 0x60000000);