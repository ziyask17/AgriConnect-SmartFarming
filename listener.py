import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    print("Connected with code", rc)
    client.subscribe("agriConnect/#")

def on_message(client, userdata, msg):
    print(msg.topic, msg.payload.decode())

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)
print("Listening for MQTT messages...")
client.loop_forever()
