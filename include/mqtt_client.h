#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

/* 仅在 -DWITH_MQTT 时参与编译（见 Makefile）。
 * 连接参数全部来自环境变量：
 *   MQTT_BROKER   例如 tcp://183.230.40.96:1883
 *   MQTT_PRODUCT  OneNET 产品 ID
 *   MQTT_DEVICE   设备名
 *   MQTT_TOKEN    设备鉴权 token
 * 源码里不保存任何密钥，避免像旧版本那样把 token 提交进仓库。 */

int  mqttc_connect(void);        /* 幂等：已连直接返回 0 */
int  mqttc_is_connected(void);
int  mqttc_publish_json(const char *json);   /* 0 成功 */
void mqttc_disconnect(void);

#endif
