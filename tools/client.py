#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import random
import sys
import errno
import time
from paho.mqtt import client as mqtt_client


broker = 'localhost'
port = 1883
topic="/strateam/fw_ver_client/control_board"
rsp_topic="/strateam/fw_ver_client/control_board/rsp"

# Generate a Client ID with the publish prefix.
client_id = f'publish-{random.randint(0, 1000)}'
# username = 'emqx'
# password = 'public'

def subscribe_topics(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f">>>>>>>>>> {msg.payload.decode()}")
        client.disconnect()

    # print( ">>> subscribe topics {}".format( rsp_topic ) )
    client.on_message = on_message
    client.subscribe(rsp_topic)
    

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            subscribe_topics( client )
            publish(client)
        else:
            print("Failed to connect, return code %d\n", rc)
            sys.exit( errno.EFAULT )

    client = mqtt_client.Client(client_id)
    # client.username_pw_set(username, password)
    client.on_connect = on_connect
    # print(f"connecting to`{broker}`:`{port}`")
    client.connect(broker, port)
    return client

        # rj::SetValueByPointer( req2, "/load_cell/monitor/squeeze/msv", 100 );
        # rj::SetValueByPointer( req2, "/load_cell/monitor/squeeze/sdf", 25000 );
        # rj::SetValueByPointer( req2, "/load_cell/monitor/squeeze/sdt", 500 );
        # rj::SetValueByPointer( req2, "/load_cell/monitor/squeeze/hv", 500 );
        # rj::SetValueByPointer( req2, "/load_cell/monitor/squeeze/bv", 500 );

def publish(client):
    payload = """
    {"ver":null}
    """
    msg = payload
    result = client.publish(topic, msg)
    # result: [0, 1]
    status = result[0]

    if status != 0:
        print(f"Failed to send message to topic {topic}")
        sys.exit( errno.EFAULT )
    


def run():
    client = connect_mqtt()
    # publish(client)
    client.loop_forever()


if __name__ == '__main__':
    run()
