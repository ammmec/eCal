from data_secure import CLIENT_ID, username, password, broker, port
from paho.mqtt import client as mqtt_client
import random
import ssl


# Generate a Client ID with the publish prefix.
client_id = f'publish-{random.randint(0, 1000)}'

# Connect to MQTT
def connectMQTT():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    #client = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION2, client_id)
    # Generate a Client ID with the publish prefix.
    client_id = f'publish-{random.randint(0, 1000)}'
    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.tls_set("emqxsl-ca.crt", cert_reqs=ssl.CERT_REQUIRED)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def publish(client, msg, topic):
    result = client.publish(topic, msg, retain=True)
    # result: [0, 1]
    status = result[0]
    if status == 0:
        print(f"Sent `{msg}` to topic `{topic}`")
    else:
        print(f"Failed to send message to topic {topic}")

client = connectMQTT()
client.loop_start()
nothing  = b'\x00'
publish(client, nothing, "schedule/A6001")
publish(client, nothing, "changes/A6001")

input("enter to send schedule: ")

payload = b'\x22\x0b\x47\x45\x49\x20\x49\x45\x53\x20\x31\x30\x54\x62\x0b\x47\x45\x49\x20\x50\x41\x52\x20\x34\x30\x54\x82\x0c\x47\x45\x49\x20\x50\x52\x4f\x50\x20\x34\x30\x54' # Schedule
publish(client, payload, "schedule/A6001")

input("enter to send SOME changes: ")
payload = b'\x22\x00\x02\x90\x0a\x47\x45\x49\x20\x46\x4d\x20\x31\x30\x54\x62\x20' # changes 1
publish(client, payload, "changes/A6001")

input("enter for last change: ")
payload = b"\x22\x20"
publish(client, payload, "changes/A6001")

client.loop_stop()
