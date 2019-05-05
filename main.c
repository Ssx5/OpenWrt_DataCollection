#include <stdio.h>
#include "rio_config.h"
#include "rio_mqtt.h"
#include "rio_modbus.h"
#include "rio_thread.h"

#define CONFIG_FILE "./config/remoteio"
#define PUBLISH_CONFIG_FILE "./config/publish.cfg"
#define SUBSCRIBE_CONFIG_FILE "./config/subscribe.cfg"

int main()
{

    load_config(CONFIG_FILE);

    
    load_publish_file(PUBLISH_CONFIG_FILE);
    
    load_subscribe_file(SUBSCRIBE_CONFIG_FILE);

    modbus_init(&modbus_ctx);
    
    mqtt_init(&mosq);

    




    return 0;
}