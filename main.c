#include <stdio.h>
#include "rio_config.h"
#include "rio_mqtt.h"
#include "rio_modbus.h"
#include "rio_thread.h"

#define CONFIG_FILE "./config/remoteio"


int main()
{

    load_config(CONFIG_FILE);

  
    pthread_t *tids = publish_task_init();
   



    modbus_init(&modbus_ctx);
    
    mqtt_init(&mosq);

    


    publish_task_wait(tids);

    return 0;
}