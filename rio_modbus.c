#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "rio_modbus.h"
#include "rio_config.h"

modbus_t *modbus_ctx;

void modbus_init(modbus_t **ctx)
{
    if(strcmp(global_config.modbus.modbus_type, "modbus_tcp") == 0)
        *ctx = modbus_new_tcp(global_config.modbus.modbus_tcp_addr, global_config.modbus.modbus_tcp_port);
    else
        *ctx = modbus_new_rtu(global_config.modbus.modbus_rtu_dev, global_config.modbus.modbus_rtu_baudrate, 'N', 8, 1);
    
    if (*ctx == NULL)
    {
        fprintf(stderr, "modbus_new_tcp error\n");
        exit(0);
    }

    if(strcmp(global_config.modbus.modbus_type, "modbus_tcp") == 0)
        modbus_set_slave(*ctx, global_config.modbus.modbus_tcp_slaveid);
    else
        modbus_set_slave(*ctx, global_config.modbus.modbus_rtu_slaveid);

    if (modbus_connect(*ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(*ctx);
        exit(0);
    }
    fprintf(stdout, "modbus_connect() success!\n");

    modbus_set_response_timeout(*ctx, 1, 0);

}