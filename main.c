#include <stdio.h>
#include "rio_config.h"
#include "rio_mqtt.h"
#include "rio_modbus.h"
#include "rio_thread.h"
#include "rio_log.h"

#define CONFIG_FILE "./config/remoteio"


int main()
{
    LOG("main() start\n");
    load_config(CONFIG_FILE);

#ifndef DEBUG
    modbus_init(&modbus_ctx);
    mqtt_init(&mosq);
#endif

    pthread_t *tids = publish_task_init();
    pthread_t tid = publisher_watcher_init();



    publisher_watcher_wait(tid);
    publish_task_wait(tids);

    return 0;
}