from paho.mqtt import client as mqtt_client
import random
import ssl
from data_secure import username, password, broker, topic, port



# Generate a Client ID with the publish prefix.
client_id = f'publish-{random.randint(0, 1000)}'

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    #client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2, client_id)
    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.tls_set("emqxsl-ca.crt", cert_reqs=ssl.CERT_REQUIRED)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def publish(client, msg):
    result = client.publish(topic, msg, retain=True)
    # result: [0, 1]
    status = result[0]
    if status == 0:
        print(f"Sent `{msg}` to topic `{topic}`")
    else:
        print(f"Failed to send message to topic {topic}")

def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")

    client.subscribe(topic)
    client.on_message = on_message

def run():
    client = connect_mqtt()
    # client.loop_start()
    # publish(client, "Good morning")
    # client.loop_stop()
    subscribe(client)
    client.loop_forever()


if __name__ == '__main__':
    run()
