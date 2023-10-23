#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import random
import time

from paho.mqtt import client as mqtt_client


broker = 'localhost'
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
    print(f"connecting to`{broker}` : `{port}`")
    client.connect(broker, port)
    return client

        # rj::SetValueByPointer( req2, "/load_cell/monitor/squeeze/msv", 100 );
        # rj::SetValueByPointer( req2, "/load_cell/monitor/squeeze/sdf", 25000 );
        # rj::SetValueByPointer( req2, "/load_cell/monitor/squeeze/sdt", 500 );
        # rj::SetValueByPointer( req2, "/load_cell/monitor/squeeze/hv", 500 );
        # rj::SetValueByPointer( req2, "/load_cell/monitor/squeeze/bv", 500 );

def publish(client):
    
    payload_squeeze= """
    {
        "equipment": {
            "controlBoard" : {
                "squeeze" : {
                "hnum" : 1,
                "bOffset" : 400.0,
                "bv" : 40.0,
                "hOffset" : 60.0,
                "hv" : 6.0
                }
            }
        }
    }"""
    payload_logs = """
    {
        "equipment": {
            "controlBoard" : {
                "sendToUart" : true
            }
        }
    }"""
    payload_set_zero = """
     {
         "equipment": {
             "controlBoard" : {
                 "setZero" : true
             }
         }
    }"""

    # payload = """
    # {
    #     "equipment": {
    #         "controlBoard" : {
    #             "stop" : true
    #         }
    #     }
    # }"""

    payload = """
    {
        "equipment": {
            "axis" : {
                "beam" : {
                    "moveToZero" : {
                        "spd" : 1000
                    }
                }
            }
        }
    }"""

    payload_get_z_pos = """
    {
        "equipment": {
            "axis" : {
                "z" : {
                    "position" : null
                }
            }
        }
    }"""


    payload_get_beam_pos = """
    {
        "equipment": {
            "axis" : {
                "beam" : {
                    "position" : null
                }
            }
        }
    }"""
    

    time.sleep(1)
    msg = payload_set_zero
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
