import paho.mqtt.client as paho
import time


def on_publish(client, userdata, mid):
    print("mid: "+str(mid))


client = paho.Client()
client.on_publish = on_publish
client.connect("127.0.0.1", 1883)
client.loop_start()

while True:
    # temperature = read_from_imaginary_thermometer()
    (rc, mid) = client.publish("test/lol", "on", qos=2)
    time.sleep(2)
