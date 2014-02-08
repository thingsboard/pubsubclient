#include "PubSubClient.h"
#include "ShimClient.h"
#include "Buffer.h"
#include "BDDTest.h"
#include "trace.h"


byte server[] = { 172, 16, 0, 2 };

bool callback_called = false;
char lastTopic[1024];
char lastPayload[1024];
unsigned int lastLength;

void reset_callback() {
    callback_called = false;
    lastTopic[0] = '\0';
    lastPayload[0] = '\0';
    lastLength = 0;
}

void callback(char* topic, byte* payload, unsigned int length) {
    callback_called = true;
    strcpy(lastTopic,topic);
    memcpy(lastPayload,payload,length);
    lastLength = length;
}

int test_receive_callback() {
    IT("receives a callback message");
    reset_callback();
    
    ShimClient shimClient;
    shimClient.setAllowConnect(true);
    
    byte connack[] = { 0x20, 0x02, 0x00, 0x00 };
    shimClient.respond(connack,4);
    
    PubSubClient client(server, 1883, callback, shimClient);
    int rc = client.connect((char*)"client_test1");
    IS_TRUE(rc);
    
    byte publish[] = {0x30,0xe,0x0,0x5,0x74,0x6f,0x70,0x69,0x63,0x70,0x61,0x79,0x6c,0x6f,0x61,0x64};
    shimClient.respond(publish,16);
    
    rc = client.loop();
    
    IS_TRUE(rc);
    
    IS_TRUE(callback_called);
    IS_TRUE(strcmp(lastTopic,"topic")==0);
    IS_TRUE(memcmp(lastPayload,"payload",7)==0);
    IS_TRUE(lastLength == 7);
    
    IS_FALSE(shimClient.error());

    END_IT
}

int test_receive_stream() {
    IT("receives a stream payload");
    // TODO
    IS_FALSE(true);
    END_IT
}

int test_receive_max_sized_message() {
    IT("receives an max-sized message");
    reset_callback();
    
    ShimClient shimClient;
    shimClient.setAllowConnect(true);
    
    byte connack[] = { 0x20, 0x02, 0x00, 0x00 };
    shimClient.respond(connack,4);
    
    PubSubClient client(server, 1883, callback, shimClient);
    int rc = client.connect((char*)"client_test1");
    IS_TRUE(rc);
    
    byte length = MQTT_MAX_PACKET_SIZE;
    byte publish[] = {0x30,length-2,0x0,0x5,0x74,0x6f,0x70,0x69,0x63,0x70,0x61,0x79,0x6c,0x6f,0x61,0x64};
    byte bigPublish[length];
    memset(bigPublish,'A',length);
    memcpy(bigPublish,publish,16);
    shimClient.respond(bigPublish,length);
    
    rc = client.loop();
    
    IS_TRUE(rc);
    
    IS_TRUE(callback_called);
    IS_TRUE(strcmp(lastTopic,"topic")==0);
    IS_TRUE(lastLength == length-9);
    IS_TRUE(memcmp(lastPayload,bigPublish+9,lastLength)==0);
    
    IS_FALSE(shimClient.error());

    END_IT
}

int test_receive_oversized_message() {
    IT("drops an oversized message");
    reset_callback();
    
    ShimClient shimClient;
    shimClient.setAllowConnect(true);
    
    byte connack[] = { 0x20, 0x02, 0x00, 0x00 };
    shimClient.respond(connack,4);
    
    PubSubClient client(server, 1883, callback, shimClient);
    int rc = client.connect((char*)"client_test1");
    IS_TRUE(rc);
    
    byte length = MQTT_MAX_PACKET_SIZE+1;
    byte publish[] = {0x30,length-2,0x0,0x5,0x74,0x6f,0x70,0x69,0x63,0x70,0x61,0x79,0x6c,0x6f,0x61,0x64};
    byte bigPublish[length];
    memset(bigPublish,'A',length);
    memcpy(bigPublish,publish,16);
    shimClient.respond(publish,length);
    
    rc = client.loop();
    
    IS_TRUE(rc);
    
    IS_FALSE(callback_called);
    
    IS_FALSE(shimClient.error());

    END_IT
}


int main()
{
    test_receive_callback();
    test_receive_stream();
    test_receive_max_sized_message();
    test_receive_oversized_message();
    
    FINISH
}
