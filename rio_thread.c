
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>

#include "rio_thread.h"
#include "rio_modbus.h"
#include "rio_mqtt.h"


typedef struct _publish_info
{
    int signal_id;
    int function_code;
    int start_address;
    int register_count;
    char publish_topic[256];
    int publish_qos;
    int publish_varied;
    int publish_period_ms;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

} publish_info_t;

typedef struct _publish_config_t
{
    int count;
    publish_info_t *infos;
} publish_config_t;

typedef struct _subscribe_info
{
    char subscribe_topic[256];
    int subscribe_qos;
    int function_code;
    int start_address;
} subscribe_info_t;

typedef struct _subscribe_config_t
{
    int count;
    subscribe_info_t *infos;
} subscribe_config_t;

publish_config_t global_publish;
subscribe_config_t global_subscribe;

void load_publish_file(char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "open %s error\n", path);
        exit(0);
    }
#define LINE 512
    int c = 0;
    char buf[LINE];
    while (fgets(buf, LINE, fp) != NULL)
    {
        if (buf[0] != '#')
            c++;
    }

    global_publish.count = c;
    global_publish.infos = (publish_info_t *)malloc(sizeof(publish_info_t) * c);
    if (global_publish.infos == NULL)
    {
        printf("malloc error\n");
        exit(0);
    }

    fseek(fp, 0, SEEK_SET);
    int i = 0;
    while (fgets(buf, LINE, fp) != NULL)
    {
        if (buf[0] != '#')
        {
            sscanf(buf, "%d,%d,%d,%d,%d,%d,%d,%s",
                   &global_publish.infos[i].signal_id,
                   &global_publish.infos[i].function_code,
                   &global_publish.infos[i].start_address,
                   &global_publish.infos[i].register_count,
                   &global_publish.infos[i].publish_qos,
                   &global_publish.infos[i].publish_varied,
                   &global_publish.infos[i].publish_period_ms,
                   global_publish.infos[i].publish_topic);

            pthread_mutex_init(&global_publish.infos[i].mutex, NULL);
            pthread_cond_init(&global_publish.infos[i].cond, NULL);
            i++;
        }
    }
    i = 0;
    printf("(%d,%d,%d,%d,%s,%d,%d,%d)\n",
           global_publish.infos[i].signal_id,
           global_publish.infos[i].function_code,
           global_publish.infos[i].start_address,
           global_publish.infos[i].register_count,
           global_publish.infos[i].publish_topic,
           global_publish.infos[i].publish_qos,
           global_publish.infos[i].publish_varied,
           global_publish.infos[i].publish_period_ms);
}

void load_subscribe_file(char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "open %s error\n", path);
        exit(0);
    }
#define LINE 512
    int c = 0;
    char buf[LINE];
    while (fgets(buf, LINE, fp) != NULL)
    {
        if (buf[0] != '#')
            c++;
    }

    global_subscribe.count = c;
    global_subscribe.infos = (subscribe_info_t *)malloc(sizeof(subscribe_info_t) * c);
    if (global_subscribe.infos == NULL)
    {
        printf("malloc error\n");
        exit(0);
    }

    fseek(fp, 0, SEEK_SET);
    int i = 0;
    while (fgets(buf, LINE, fp) != NULL)
    {
        if (buf[0] != '#')
        {
            sscanf(buf, "%d,%d,%d,%s",
                   &global_subscribe.infos[i].subscribe_qos,
                   &global_subscribe.infos[i].function_code,
                   &global_subscribe.infos[i].start_address,
                   global_subscribe.infos[i].subscribe_topic);
            i++;
        }
    }
    i = 0;
    printf("%d,%d,%d,%s\n",
           global_subscribe.infos[i].subscribe_qos,
           global_subscribe.infos[i].function_code,
           global_subscribe.infos[i].start_address,
           global_subscribe.infos[i].subscribe_topic);
}


typedef struct _publisher{

    int publish_period_ms;
    time_t next_publish_time;
    pthread_cond_t *cond;

}publisher_t;


publisher_t *global_publishers;

void publoisher_init()
{
    global_publishers = (publisher_t *)malloc(sizeof(publisher_t) * global_publish.count);
    if(global_publishers == NULL)
    {
        printf("malloc error\n");
        exit(0);
    }
    time_t now = time(NULL);
    for(int i = 0; i < global_publish.count; ++i)
    {
        global_publishers[i].publish_period_ms = global_publish.infos[i].publish_period_ms;
        global_publishers[i].next_publish_time = now + global_publishers[i].publish_period_ms;
        global_publishers[i].cond = &global_publish.infos[i].cond;
    }
}

void publish_watcher(publisher_t* p, int n)
{

    time_t now = time(NULL);
    for(int i = 0; i < n; ++i)
    {
        if(now >= p[i].next_publish_time)
        {
            pthread_cond_broadcast(p[i].cond);
            p[i].next_publish_time += p[i].publish_period_ms;
        }
    }
}


void *publisher_routine(void *arg)
{
    publish_info_t *p = (publish_info_t *)arg;

    pthread_mutex_lock(&p->mutex);
    pthread_cond_wait(&p->cond, &p->mutex);
    pthread_mutex_unlock(&p->mutex);

    int ret;
    uint8_t bitbuf[32];
    uint16_t regbuf[16];

    switch (p->function_code)
    {
    case 1:
        ret = modbus_read_bits(modbus_ctx, p->start_address, p->register_count, bitbuf);
        if (ret < 0)
        {
            fprintf(stderr, "Read Failed: %s\n", modbus_strerror(errno));
        }
        mosquitto_publish(mosq, NULL, p->publish_topic, ret, bitbuf, p->publish_qos, false);
        break;
    case 2:
        ret = modbus_read_input_bits(modbus_ctx, p->start_address, p->register_count, bitbuf);
        if (ret < 0)
        {
            fprintf(stderr, "Read Failed: %s\n", modbus_strerror(errno));
        }
        mosquitto_publish(mosq, NULL, p->publish_topic, ret, bitbuf, p->publish_qos, false);
        break;
    case 3:
        ret = modbus_read_registers(modbus_ctx,p->start_address, p->register_count, regbuf);
        if (ret < 0)
        {
            fprintf(stderr, "Read Failed: %s\n", modbus_strerror(errno));
        }
        mosquitto_publish(mosq, NULL, p->publish_topic, ret * 2, regbuf, p->publish_qos, false);
        break;
    case 4:
        ret = modbus_read_input_registers(modbus_ctx, p->start_address, p->register_count, regbuf);
        if (ret < 0)
        {
            fprintf(stderr, "Read Failed: %s\n", modbus_strerror(errno));
        }
        mosquitto_publish(mosq, NULL, p->publish_topic, ret * 2, regbuf, p->publish_qos, false);
        break;
    default:
        break;
    }
    return 0;
}