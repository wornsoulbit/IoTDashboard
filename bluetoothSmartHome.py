import bluetooth
import time
        
while True:
    devices = bluetooth.discover_devices(lookup_names=True)
    result=""
    for device in devices:
        device_mac = BluetoothRSSI(device[0])
        rssi_tuple = device_mac.request_rssi()
        print(rssi_tuple)
        #result += str(x)
        #result += "=> RSSI:" + str(rssi_tuple)
    time.sleep(1)