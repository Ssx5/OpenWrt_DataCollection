#ifndef __RIO_MODBUS_H__
#define __RIO_MODBUS_H__

#include <modbus/modbus.h>


void modbus_init(modbus_t **ctx);

extern modbus_t *modbus_ctx;
#endif