/*
 * Copyright (c) 2020-2021 Arm Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#if !MBED_CONF_AWS_CLIENT_SHADOW
#include <string>     // std::string, std::to_string
#include "mbed.h"
#include "mbed-trace/mbed_trace.h"
#include "rtos/ThisThread.h"
#include "AWSClient/AWSClient.h"

// BSP Sensor related includes
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_hsensor.h"
// #include "stm32l475e_iot01_accelero.h"
// #include "stm32l475e_iot01_gyro.h"
// #include "stm32l475e_iot01_magneto.h"
// #include "stm32l475e_iot01_qspi.h"

extern "C" {
#include "core_json.h"
}

#define TRACE_GROUP "Main"

#define THIS_DEVICE_NAME "mySTBoard" //to be used in MQTT message

static bool reply_received = false;

// Callback when a MQTT message has been added to the topic
void on_message_callback(
    const char *topic,
    uint16_t topic_length,
    const void *payload,
    size_t payload_length)
{
    char *json_value;
    size_t value_length;
    auto ret = JSON_Search((char *)payload, payload_length, "sender", strlen("sender"), &json_value, &value_length);
    if (ret == JSONSuccess && (strncmp(json_value, THIS_DEVICE_NAME, strlen(THIS_DEVICE_NAME)) == 0)) {
        tr_info("Message sent successfully");
    } else {
        ret = JSON_Search((char *)payload, payload_length, "message", strlen("message"), &json_value, &value_length);
        if (ret == JSONSuccess) {
            reply_received = true;
            tr_info("Message received from the cloud: \"%.*s\"", value_length, json_value);
        } else {
            tr_error("Failed to extract message from the payload: \"%.*s\"", payload_length, (const char *) payload);
        }
    }
}

// Reverses a string 'str' of length 'len'
void reverse(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}
  
// Converts a given integer x to string str[]. 
// d is the number of digits required in the output. 
// If d is more than the number of digits in x, 
// then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
  
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';
  
    reverse(str, i);
    str[i] = '\0';
    return i;
}
  
// Converts a floating-point/double number to a string.
void ftoa(float n, char* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
  
    // Extract floating part
    float fpart = n - (float)ipart;
  
    // convert integer part to string
    int i = intToStr(ipart, res, 0);
  
    // check for display option after point
    if (afterpoint != 0) {
        res[i] = '.'; // add dot
  
        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter 
        // is needed to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);
  
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

void demo()
{
    float tSensVal = 0,hSensVal = 0, pSensVal = 0;
    char strtSensVal[] = "00.00", strhSensVal[] = "00.00", strpSensVal[] = "00.00";
    AWSClient &client = AWSClient::getInstance();

    // Subscribe to the topic
    const char topic[] = MBED_CONF_APP_AWS_MQTT_TOPIC;
    int ret = client.subscribe(topic, strlen(topic));
    if (ret != MBED_SUCCESS) {
        tr_error("AWSClient::subscribe() failed");
        return;
    }

    // Send ten message to the cloud (one per second)
    // Stop when we receive a cloud-to-device message
    char payload[128];

    for(char msgSendCount = 0; msgSendCount < 10 ; msgSendCount++)
    {
        if (reply_received) {
            // If we have received a message from the cloud, don't send more messeges
            break;
        }

        // Read BSP sensor Values
        tSensVal = BSP_TSENSOR_ReadTemp();
        pSensVal = BSP_PSENSOR_ReadPressure();
        hSensVal = BSP_HSENSOR_ReadHumidity();

        ftoa(tSensVal, strtSensVal, 2);
        ftoa(hSensVal, strhSensVal, 2);
        ftoa(pSensVal, strpSensVal, 2);

        // The MQTT protocol does not distinguish between senders,
        // so we add a "sender" attribute to the payload
        sprintf(payload, "{\n"
                "    \"sender\": \"%s\",\n"
                "    \"temp\": \"%s\",\n"
                "    \"pres\": \"%s\",\n"
                "    \"humi\": \"%s\"\n"
                "}",
                THIS_DEVICE_NAME, strtSensVal, strpSensVal, strhSensVal);
        tr_info("Publishing to topic \"%s\"", topic);
        tr_info("Publishing \"tempVal = %s\"", strtSensVal);
        tr_info("Publishing \"presVal = %s\"", strpSensVal);
        tr_info("Publishing \"humiVal = %s\"", strhSensVal);
        ret = client.publish(
                  topic,
                  strlen(topic),
                  payload,
                  strlen(payload)
              );
        if (ret != MBED_SUCCESS) {
            tr_error("AWSClient::publish() failed");
            goto unsubscribe;
        }

        rtos::ThisThread::sleep_for(1s);
    }

    // If the user didn't manage to send a cloud-to-device message earlier,
    // let's wait until we receive one
    // while (!reply_received) {
        // Continue to receive messages in the communication thread
        // which is internally created and maintained by the AWS SDK.
        // sleep();
    // }

unsubscribe:
    // Unsubscribe from the topic
    ret = client.unsubscribe(topic, strlen(topic));
    if (ret != MBED_SUCCESS) {
        tr_error("AWSClient::unsubscribe() failed");
    }
}

#endif // !MBED_CONF_AWS_CLIENT_SHADOW