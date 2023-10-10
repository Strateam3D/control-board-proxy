#!/usr/bin/env python3.8
# -*- coding: utf-8 -*-

import random
import time

from paho.mqtt import client as mqtt_client


broker = '127.0.0.1'
port = 1883
topic="/strateam/pyclient-tool/control-board"
rsp_topic="/strateam/pyclient-tool/control-board/rsp"
status_topic="/strateam/pyclient-tool/control-board/notify"
# Generate a Client ID with the publish prefix.
client_id = f'publish-{random.randint(0, 1000)}'
# username = 'emqx'
# password = 'public'

def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")

    print( ">>> subscribe topics" )
    client.subscribe(rsp_topic)
    client.subscribe(status_topic)
    client.on_message = on_message

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
            subscribe(client)
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    # client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def publish(client):
    
    payload = """
    {
        "equipment": {
            "axis" : {
                "h1" : {
                    "move": {
                        "offset" : 10000,
                        "spd" : 1000
                    }
                }
            }
        }
    }"""

    time.sleep(1)
    msg = payload
    result = client.publish(topic, msg)
    # result: [0, 1]
    status = result[0]
    if status == 0:
        print(f"Send `{msg}` to topic `{topic}`")
    else:
        print(f"Failed to send message to topic {topic}")
    


def run():
    client = connect_mqtt()
    client.loop_start()
    publish(client)
    client.loop_forever()


if __name__ == '__main__':
    run()
