import bluetooth
from bluetoothRSSIFile import BluetoothRSSI
import time
import functools
from paho.mqtt import client as mqtt_client
import random

broker = 'localhost'
port = 1883
topic = "IoTlab/Bluetooth"
client_id = f'python-mqtt-{random.randint(0,1000)}'
# username = 'emqx'
# password = 'public'

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)
    # Set Connecting Client ID
    client = mqtt_client.Client(client_id)
    #client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

def publish(client):
    msg_count = 0
    while True:
        devices = bluetooth.discover_devices(lookup_names=True)
    
        result = ''
        for x in devices:
            device = BluetoothRSSI(x[0])
            rssi_q = device.request_rssi()
            rssi1 = functools.reduce(lambda sub, ele: sub * 10 + ele, rssi_q)
            result += str(x)
            result += ' => RSSI: ' + str(rssi1)
        client.publish(topic, result)
        time.sleep(5)

def run():
    client = connect_mqtt()
    client.loop_start()
    publish(client)
    
if __name__ == '__main__':
    run()
        