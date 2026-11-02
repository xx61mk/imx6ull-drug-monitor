/* 仅 -DWITH_MQTT 时编译。需要 Paho MQTT C 库（libpaho-mqtt3c）。 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <MQTTClient.h>

#include "../include/mqtt_client.h"

static MQTTClient g_client;
static int g_created = 0;
static int g_connected = 0;
static char g_topic_pub[200] = {0};

static void on_delivered(void *context, MQTTClient_deliveryToken dt)
{
    (void)context;
    (void)dt;
}

static int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    (void)context;
    (void)topicLen;
    printf("[mqtt] <- %s : %.*s\n", topicName,
           message->payloadlen, (char *)message->payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

static void on_conn_lost(void *context, char *cause)
{
    (void)context;
    g_connected = 0;
    printf("[mqtt] connection lost: %s\n", (cause != NULL) ? cause : "?");
}

static const char *env_or_null(const char *name)
{
    const char *v = getenv(name);
    if (NULL == v || '\0' == v[0])
    {
        return NULL;
    }
    return v;
}

int mqttc_connect(void)
{
    const char *broker = env_or_null("MQTT_BROKER");
    const char *product = env_or_null("MQTT_PRODUCT");
    const char *device = env_or_null("MQTT_DEVICE");
    const char *token = env_or_null("MQTT_TOKEN");
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;

    if (g_connected)
    {
        return 0;
    }
    if (NULL == broker || NULL == product || NULL == device || NULL == token)
    {
        printf("[mqtt] env not set, skip cloud. need MQTT_BROKER / MQTT_PRODUCT / MQTT_DEVICE / MQTT_TOKEN\n");
        return -1;
    }

    if (!g_created)
    {
        rc = MQTTClient_create(&g_client, broker, device,
                               MQTTCLIENT_PERSISTENCE_NONE, NULL);
        if (MQTTCLIENT_SUCCESS != rc)
        {
            printf("[mqtt] create failed rc=%d\n", rc);
            return -1;
        }
        rc = MQTTClient_setCallbacks(g_client, NULL, on_conn_lost, on_message, on_delivered);
        if (MQTTCLIENT_SUCCESS != rc)
        {
            printf("[mqtt] setCallbacks failed rc=%d\n", rc);
            MQTTClient_destroy(&g_client);
            g_created = 0;
            return -1;
        }
        snprintf(g_topic_pub, sizeof(g_topic_pub),
                 "$sys/%s/%s/thing/property/post", product, device);
        g_created = 1;
    }

    opts.keepAliveInterval = 30;
    opts.cleansession = 1;
    opts.username = product;
    opts.password = token;
    opts.connectTimeout = 5;
    opts.MQTTVersion = MQTTVERSION_3_1_1;

    rc = MQTTClient_connect(g_client, &opts);
    if (MQTTCLIENT_SUCCESS != rc)
    {
        printf("[mqtt] connect failed rc=%d (%s)\n", rc, MQTTClient_strerror(rc));
        return -1;
    }

    g_connected = 1;
    printf("[mqtt] connected %s as %s\n", broker, device);
    return 0;
}

int mqttc_is_connected(void)
{
    return g_connected;
}

int mqttc_publish_json(const char *json)
{
    MQTTClient_message msg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    if (!g_connected || NULL == json)
    {
        return -1;
    }

    msg.payload = (void *)json;
    msg.payloadlen = (int)strlen(json);
    msg.qos = 1;                /* QoS1：至少送达一次，配合本地去重保证不丢 */
    msg.retained = 0;

    rc = MQTTClient_publishMessage(g_client, g_topic_pub, &msg, &token);
    if (MQTTCLIENT_SUCCESS != rc)
    {
        printf("[mqtt] publish failed rc=%d\n", rc);
        return -1;
    }
    rc = MQTTClient_waitForCompletion(g_client, token, 5000L);
    if (MQTTCLIENT_SUCCESS != rc)
    {
        printf("[mqtt] waitForCompletion failed rc=%d\n", rc);
        return -1;
    }
    return 0;
}

void mqttc_disconnect(void)
{
    if (g_created)
    {
        if (g_connected)
        {
            MQTTClient_disconnect(g_client, 3000);
        }
        MQTTClient_destroy(&g_client);
        g_created = 0;
        g_connected = 0;
    }
}
