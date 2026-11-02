#ifndef HEAD_H
#define HEAD_H


#include <MQTTAsync.h>
#include <MQTTClient.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>
//#define OLD_ADDRESS     "tcp://218.201.45.7:1883"
#define NEW_ADDRESS     "tcp://183.230.40.96:1883"
#define DEV_NAME    "env"   //"device_name"  //"first_device"
#define CLIENTID DEV_NAME
/* v1 把 OneNET 三元组硬编码在这里，曾随仓库公开，已作废并从当前版本移除。
 * 真机的产品 ID / 设备名 / 鉴权 token 请填自己的，或参考 v2 的 src/mqtt_client.c
 * 改为从环境变量读取（MQTT_PRODUCT / MQTT_DEVICE / MQTT_TOKEN）。 */
#define PRODUCT_ID    "YOUR_PRODUCT_ID"
#define PASSWD        "YOUR_DEVICE_TOKEN"
#define QOS         0
#define TIMEOUT     10000L



extern int mqtt_init();
extern int mqtt_send(char * key, float value);
extern void mqtt_deinit();


//#define __cplusplus
#endif // HEAD_H
