#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import random
from tkinter import *
from paho.mqtt import client as mqtt_client

broker = '192.168.1.185'
port = 1883
topic = "/strateam/pyclient-tool/control-board"
client_id = f'publish-{random.randint(0, 1000)}'

# Define max and min limits for distances and speeds for each axis
axis_limits = {
    'beam': {'max_dist': 200, 'min_dist': -200, 'max_speed': 40, 'min_speed': 1},
    'z': {'max_dist': 150, 'min_dist': -150, 'max_speed': 20, 'min_speed': 0.5},
    'h1': {'max_dist': 100, 'min_dist': -100, 'max_speed': 10, 'min_speed': 0.01},
    'h2': {'max_dist': 100, 'min_dist': -100, 'max_speed': 10, 'min_speed': 0.01},
}

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

# Initialize Tkinter window
window = Tk()
window.title("Machinery Control")

# Placeholder handling for Entry widgets
def focus_in(event, entry, placeholder):
    if entry.get() == placeholder:
        entry.delete(0, END)
        entry.config(fg='black')

def focus_out(event, entry, placeholder):
    if not entry.get():
        entry.insert(0, placeholder)
        entry.config(fg='grey')

# Create OptionBox for selecting the axis
axis_var = StringVar()
axis_var.set("beam")
axis_label = Label(window, text="Choose an Axis:")
axis_label.pack()
axis_options = ['beam', 'z', 'h1', 'h2']

# Function to update slider limits
def update_slider_limits():
    axis = axis_var.get()
    limits = axis_limits[axis]
    speed_scale.config(from_=limits['min_speed'], to=limits['max_speed'])
    offset_scale.config(from_=limits['min_dist'], to=limits['max_dist'])

# Create Speed slider
speed_scale = Scale(window, from_=0, to=100, orient=HORIZONTAL, label="Speed (mm/sec)", length=300)
speed_scale.pack()

# Create Offset slider
offset_scale = Scale(window, from_=0, to=1000, orient=HORIZONTAL, label="Offset (mm)", length=300)
offset_scale.pack()

# Update Entry widgets whenever the sliders are moved
def update_entries(val):
    speed_entry.delete(0, END)
    speed_entry.insert(0, str(speed_scale.get()))
    offset_entry.delete(0, END)
    offset_entry.insert(0, str(offset_scale.get()))

speed_scale.bind("<Motion>", update_entries)
offset_scale.bind("<Motion>", update_entries)

# Update slider limits when axis changes
for option in axis_options:
    Radiobutton(window, text=option, variable=axis_var, value=option, command=update_slider_limits).pack()

update_slider_limits()  # Initialize slider limits
# Create text boxes for speed and offset with placeholders and units
speed_placeholder = 'Enter speed (mm/sec)'
speed_entry = Entry(window, fg='grey')
speed_entry.pack()
speed_entry.insert(0, speed_placeholder)
speed_entry.bind("<FocusIn>", lambda event: focus_in(event, speed_entry, speed_placeholder))
speed_entry.bind("<FocusOut>", lambda event: focus_out(event, speed_entry, speed_placeholder))

offset_placeholder = 'Enter offset (mm)'
offset_entry = Entry(window, fg='grey')
offset_entry.pack()
offset_entry.insert(0, offset_placeholder)
offset_entry.bind("<FocusIn>", lambda event: focus_in(event, offset_entry, offset_placeholder))
offset_entry.bind("<FocusOut>", lambda event: focus_out(event, offset_entry, offset_placeholder))

# Initialize MQTT client
client = connect_mqtt()
client.loop_start()

# Define function for button click
def on_button_click(action_type):
    axis = axis_var.get()
    speed = abs(int(float(speed_entry.get()) * 1000))
    offset = int(float(offset_entry.get()) * 1000)

    if action_type == 'move':
        publish_command(client, axis, speed, offset, action_type)
    elif action_type == 'moveToZero':
        publish_command(client, axis, speed, 0, action_type)
    elif action_type == 'stop':
        publish_command(client, axis, 0, 0, action_type)

# Define functions for jog button press and release
def on_jog_press(direction):
    axis = axis_var.get()
    speed = abs(int(float(speed_entry.get()) * 1000))  # Convert from mm/sec to um/sec and ensure it's positive

    # Get the maximum and minimum offset values for the selected axis
    max_offset = axis_limits[axis]['max_dist'] * 1000  # Convert from mm to um
    min_offset = axis_limits[axis]['min_dist'] * 1000  # Convert from mm to um

    # Use the maximum or minimum offset value based on the direction
    offset = max_offset if direction == '+' else min_offset

    publish_command(client, axis, speed, offset, 'move')

def on_jog_release():
    axis = axis_var.get()
    publish_command(client, axis, 0, 0, 'stop')

# Create jog buttons
jog_plus_button = Button(window, text="Jog +", width=20)
jog_plus_button.pack(side=TOP)
jog_plus_button.bind("<ButtonPress-1>", lambda event: on_jog_press('+'))
jog_plus_button.bind("<ButtonRelease-1>", lambda event: on_jog_release())

# Create Stop button
stop_button = Button(window, text="Stop", width=20, command=lambda: on_button_click("stop"))
stop_button.pack(side=TOP)

# Create jog minus button
jog_minus_button = Button(window, text="Jog -", width=20)
jog_minus_button.pack(side=TOP)
jog_minus_button.bind("<ButtonPress-1>", lambda event: on_jog_press('-'))
jog_minus_button.bind("<ButtonRelease-1>", lambda event: on_jog_release())

# Create Move To Zero button
move_to_zero_button = Button(window, text="Move To Zero", width=20, command=lambda: on_button_click("moveToZero"))
move_to_zero_button.pack(side=TOP)

# Create Move button
move_button = Button(window, text="Move", width=20, command=lambda: on_button_click("move"))
move_button.pack(side=TOP)

# Run the application
window.mainloop()

# Stop the MQTT client loop
client.loop_stop()
