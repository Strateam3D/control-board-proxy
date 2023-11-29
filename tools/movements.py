#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import random
from tkinter import *
from paho.mqtt import client as mqtt_client
from tkinter import ttk
from smb.SMBConnection import SMBConnection
from smb import smb_structs
import json

# broker = '192.168.1.185'
broker = 'localhost'
port = 1883
topic = "/strateam/pyclient-tool/control-board"
client_id = f'publish-{random.randint(0, 1000)}'

height = 2
width = 3
axis_data_cells = []

# Define max and min limits for distances and speeds for each axis
axis_limits = {
    'beam': {'max_dist': 200, 'min_dist': -200, 'max_speed': 40, 'min_speed': 1, 'max_home_offset' : 5000, 'min_home_offset' : -5000},
    'z': {'max_dist': 150, 'min_dist': -150, 'max_speed': 20, 'min_speed': 0.5, 'max_home_offset' : 5000, 'min_home_offset' : -5000},
    'h1': {'max_dist': 100, 'min_dist': -100, 'max_speed': 10, 'min_speed': 0.01, 'max_home_offset' : 5000, 'min_home_offset' : -5000},
    'h2': {'max_dist': 100, 'min_dist': -100, 'max_speed': 10, 'min_speed': 0.01, 'max_home_offset' : 5000, 'min_home_offset' : -5000},
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
        print(f"Sent {action_type} command to topic {topic}")
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
    home_offset_scale.config(from_=limits['min_home_offset'], to=limits['max_home_offset'])


def jobSelectionChanged( event ):
    print( "selected job {}".format( currentJob.get() ) )

def imgSelectionChanged(event):
    print( "selected img {}".format( currentImage.get() ) )

# Create Speed slider
speed_scale = Scale(window, from_=0, to=100, orient=HORIZONTAL, label="Speed (mm/sec)", length=300)
speed_scale.pack()

# Create Offset slider
offset_scale = Scale(window, from_=0, to=1000, orient=HORIZONTAL, label="Offset (mm)", length=300)
offset_scale.pack()

# Create home Offset slider
home_offset_scale = Scale(window, from_=0, to=1000, orient=HORIZONTAL, label="Home Offset (um)", length=300)
home_offset_scale.pack()

# Update Entry widgets whenever the sliders are moved
def update_entries(val):
    speed_entry.delete(0, END)
    speed_entry.insert(0, str(speed_scale.get()))
    offset_entry.delete(0, END)
    offset_entry.insert(0, str(offset_scale.get()))

    home_offset_entry.delete(0, END)
    home_offset_entry.insert(0, str(home_offset_scale.get()))

speed_scale.bind("<Motion>", update_entries)
offset_scale.bind("<Motion>", update_entries)
home_offset_scale.bind("<Motion>", update_entries)

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

home_offset_placeholder = 'Enter home offset (mm)'
home_offset_entry = Entry(window, fg='grey')
home_offset_entry.pack()
home_offset_entry.insert(0, home_offset_placeholder)
home_offset_entry.bind("<FocusIn>", lambda event: focus_in(event, offset_entry, offset_placeholder))
home_offset_entry.bind("<FocusOut>", lambda event: focus_out(event, offset_entry, offset_placeholder))

# Initialize MQTT client

def send_home_offset():
    homeOffset = int(float(home_offset_entry.get()))
    axis = axis_var.get()
    print( "home offset {}".format( homeOffset ) )

    payload = f"""
        {{
            "equipment": {{
                "axis": {{
                    "{axis}": {{
                        "homePosition": {homeOffset}
                    }}
                }}
            }}
        }}"""

    result = client.publish(topic, payload)
    status = result[0]
    
    if status == 0:
        print(f"Sent home offset {homeOffset} to topic {topic}")
    else:
        print(f"Failed to send message to topic {topic}")


# Define function for button click
def on_button_click(action_type):
    axis = axis_var.get()
    speed = abs(int(float(speed_entry.get()) * 1000))
    offset = int(float(offset_entry.get()) * 1000)

    if action_type == 'move':
        publish_command(client, axis, speed, offset, action_type)
    elif action_type == 'moveToZero':
        # homeOffset = int(float(home_offset_entry.get()))
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


def fetchJobList(jobsList):
    # try:
        # smb_structs.SUPPORT_SMB2 = False
        conn = SMBConnection( 'admin', 'admin', 'test client', broker )
        conn.connect( broker, 139 )
        results = conn.listPath('Job', '\\', pattern="Paste2Print*")
        jobs = []
        
        if len(results) > 0:
            for f in results:
                print(f.filename)
                # jobsList['values'].append(f.filename)
                jobs.append(f.filename)
    
        jobsList[ 'values' ] = jobs
        jobsList.set(jobs[0])

def fetchImageList(imgList):
    conn = SMBConnection( 'admin', 'admin', 'test client', broker )
    conn.connect( broker, 139 )
    results = conn.listPath('Job', '\\', pattern="*.png")
    images = []
    
    if len(results) > 0:
        for f in results:
            print(f.filename)
            # jobsList['values'].append(f.filename)
            images.append(f.filename)

    print(images)
    imgList[ 'values' ] = images

    if len(images) > 0:
        imgList.set(images[0])

def on_print_button_click(jobName, doHoming):
    #payload = """
    #{{
    #    "command": {{
    #        "start" : {{
    #            "doHoming" : {},
    #            "path" : "/strateam/Job/{}"
    #        }}
    #    }}
    #}}""".format(doHoming, jobName.get())
    payload = {
        "command": {
            "start" : {
                "doHoming" : doHoming,
                "path" : "/strateam/Job/" + jobName.get()
            }
        }
    }
    req = json.dumps(payload)
    print( "req {}".format(req) )
    printtopic="/strateam/pyclient-tool/print_controller"
    result = client.publish(printtopic, req)
    status = result[0]
    
    if status == 0:
        print(f"Sent print start print command for job {jobName.get()}")
    else:
        print(f"Failed to send cancel message for job {jobName.get()}")


def on_display_button_click(imgName):
    payload_display_img = """
    {{
        "projector": {{
            "display" : {{
                "path" : "/strateam/Job/{}",
                "time" : 0
            }}
        }}
    }}""".format(imgName.get())
    # print( "req {}".format(payload) )
    display_topic="/strateam/pyclient-tool/projector-equipment"
    result = client.publish(display_topic, payload_display_img)
    status = result[0]
    
    if status == 0:
        print(f"Sent display command for job {imgName.get()}")
    else:
        print(f"Failed to send display message for job {imgName.get()}")

def on_led_off_button_click():
    payload_led_poweroff = """
    {
        "projector": {
            "ledPowerOn" : false
        }
    }"""

    # print( "req {}".format(payload) )
    display_topic="/strateam/pyclient-tool/projector-equipment"
    result = client.publish(display_topic, payload_led_poweroff)
    status = result[0]
    
    if status == 0:
        print(f"Sent display command for led off")
    else:
        print(f"Failed to send  message for led off")

def on_cancel_button_click(jobName):
    payload = """
    {{
        "command": {{
            "cancel" : "Cancelled by user"
        }}
    }}"""
    # print( "req {}".format(payload) )
    result = client.publish(topic, payload)
    status = result[0]
    
    if status == 0:
        print(f"Sent cancel command for job {jobName.get()}")
    else:
        print(f"Failed to send cancel message for job {jobName.get()}")

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

# Set home position
set_home_pos_button = Button(window, text="Set Home offset", width=20, command=lambda: send_home_offset)
set_home_pos_button.pack(side=TOP)

# Create Move To Zero button
move_to_zero_button = Button(window, text="Move To Zero", width=20, command=lambda: on_button_click("moveToZero"))
move_to_zero_button.pack(side=TOP)



# Create Move button
move_button = Button(window, text="Move", width=20, command=lambda: on_button_click("move"))
move_button.pack(side=TOP)

# jobs
currentJob = StringVar()
jobsCombobox = ttk.Combobox(window, textvariable=currentJob)
jobsCombobox['state'] = 'readonly'
jobsCombobox.pack(side=TOP)
jobsCombobox.bind( '<<ComboboxSelected>>', jobSelectionChanged )
# fetchJobList(jobsCombobox)
print_button = Button(window, text="Print", width=40, command=lambda: on_print_button_click(currentJob, True))
print_button.pack(side=TOP)
continue_button = Button(window, text="Continue", width=40, command=lambda: on_print_button_click(currentJob, False))
continue_button.pack(side=TOP)
stop_button = Button(window, text="Stop", width=40, command=lambda: on_cancel_button_click(currentJob))
stop_button.pack(side=TOP)

#images
# jobs
currentImage = StringVar()
imageCombobox = ttk.Combobox(window, textvariable=currentImage)
imageCombobox['state'] = 'readonly'
imageCombobox.pack(side=TOP)
imageCombobox.bind( '<<ComboboxSelected>>', imgSelectionChanged )
# fetchImageList(imageCombobox)
display_button = Button(window, text="Display", width=40, command=lambda: on_display_button_click(currentImage))
display_button.pack(side=TOP)
led_off_button = Button(window, text="LedOff", width=40, command=lambda: on_led_off_button_click())
led_off_button.pack(side=TOP)

status = Frame(window)
status.pack(side=TOP)

b = Label(status, text="Z")
b.grid(row=0,column=0)

b = Label(status, text="H1")
b.grid(row=0,column=1)

b = Label(status, text="H2")
b.grid(row=0,column=2)

b = Label(status, text="Beam")
b.grid(row=0,column=3)

zLbl = Label(status, text="101")
zLbl.grid(row=1,column=0)


h1Lbl = Label(status, text="102")
h1Lbl.grid(row=1,column=1)


h2Lbl = Label(status, text="103")
h2Lbl.grid(row=1,column=2)


beamLbl = Label(status, text="104")
beamLbl.grid(row=1,column=3)

def on_message_status(client, userdata, msg):
        data = msg.payload.decode()
        # print(f"Received `{data}` from `{msg.topic}` topic")
        doc = json.loads(data)
        print(doc)
        
        try:
            zLbl.config(text=str( round( doc['axis']['z']['pos'] * 0.001, 3 ) )) 
            h1Lbl.config(text=str( round( doc["axis"]["h1"]["pos"] * 0.001, 3 ) ))
            h2Lbl.config(text=str( round( doc["axis"]["h2"]["pos"] * 0.001, 3 ) ))
            beamLbl.config(text=str(doc["axis"]["beam"]["pos"]))
        except TypeError as e:
            print("status msg err:{}".format(e))

client = connect_mqtt()

print( ">>> subscribe topics" )
client.subscribe("/strateam/control-board/notify")
client.on_message = on_message_status
client.loop_start()
# Run the application
window.mainloop()

# Stop the MQTT client loop
client.loop_stop()
