#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import random
import time
import argparse

from paho.mqtt import client as mqtt_client

broker = '127.0.0.1'
port = 1883
topic = "/strateam/pyclient-tool/control-board"
client_id = f'publish-{random.randint(0, 1000)}'

def parse_args():
    parser = argparse.ArgumentParser(description="Command parameters.")
    parser.add_argument('--axis', type=str, help='Axis to control (e.g., m2, beam, z)')
    parser.add_argument('--speed', type=int, help='Speed of movement')
    parser.add_argument('--offset', type=int, help='Offset for movement')
    parser.add_argument('--action_type', type=str, help='Type of action (e.g., move, stop, setZero)')
    return parser.parse_args()

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    print(f"connecting to `{broker}` : `{port}`")
    client.connect(broker, port)
    return client

def publish_command(client, axis, speed, offset, action_type):
    if action_type == 'stop':
        payload = f"""
        {{
            "equipment": {{
                "axis": {{
                    "{axis}": {{
                        "stop": true
                    }}
                }}
            }}
        }}"""
    else:
        payload = f"""
        {{
            "equipment": {{
                "axis": {{
                    "{axis}": {{
                        "{action_type}": {{
                            "spd": {speed},
                            "offset": {offset}
                        }}
                    }}
                }}
            }}
        }}"""

    result = client.publish(topic, payload)
    status = result[0]
    if status == 0:
        print(f"Sent {action_type} command to axis {axis}")
    else:
        print(f"Failed to send message to topic {topic}")

def run():
    args = parse_args()

    client = connect_mqtt()
    client.loop_start()

    if args.axis and args.action_type:
        speed = args.speed if args.speed else 0
        offset = args.offset if args.offset else 0
        publish_command(client, args.axis, speed, offset, args.action_type)
    else:
        axis = input("Enter the axis to control (e.g., 'm2', 'beam', 'z'): ")
        action_type = input("Enter the action type (e.g., 'move', 'stop', 'setZero'): ")
        speed = int(input("Enter the speed: "))
        offset = int(input("Enter the offset: "))
        publish_command(client, axis, speed, offset, action_type)

    time.sleep(1)  # Give it a second to send the message.
    client.loop_stop()  # Stop the loop.

if __name__ == '__main__':
    run()
