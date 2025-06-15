import requests
import icalendar
from datetime import datetime, timedelta
from data_secure import CLIENT_ID, username, password, broker, port
from os.path import commonprefix
from collections import defaultdict
from paho.mqtt import client as mqtt_client
import random
import ssl
import time

startHour = 8       # Classes start at 8 in the morning...
endHour = 21        # ... and end at 21h

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

# Get today's date (start_date <= today < end_date)
def getToday():
    today = datetime.now()
    startDate = (today - timedelta(days=17)).strftime("%Y-%m-%d")
    endDate = (today - timedelta(days=16)).strftime("%Y-%m-%d")
    return startDate, endDate, today.strftime("%d%m%Y")

# API call to get list of classrooms
def getClassrooms():
    url = f"https://api.fib.upc.edu/v2/aules/?client_id={CLIENT_ID}&format=json"
    response = requests.get(url)
    if response.status_code == 200:
        return response.json().get("results", [])
    else:
        print(f"Error fetching classrooms: {response.status_code} - {response.text}")
        return []

# Extract the schedule of one classroom
def getSchedule(url):
    response = requests.get(url)
    if response.status_code == 200:
        return icalendar.Calendar.from_ical(response.text)
    else:
        print(f"Error fetching schedule: {response.status_code} - {response.text}")
        return None

# Obtain list of scheduled classes in that classroom in that day
def extractEvents(calendar):
    classList = []
    for event in calendar.walk("VEVENT"):
        start = int(event.get("DTSTART").dt.strftime("%#H")) # Get start time 
        end = int((event.get("DTEND").dt + timedelta(minutes=1)).strftime("%#H")) # Get end time
        name = str(event.get("DESCRIPTION")) if event.get("DESCRIPTION") else str(event.get("SUMMARY")) # Get name of subject
        classList.append((name, start, end))
    return classList

# Combine names of classes if two are happening at the same time
# this is useful for cases where two different majors have the same class, so they have the same name except for the major
# also useful in case the classroom is shared by different groups doing the same subject
def combineStrings(strings):
    if not strings:
        return ""
    if all(s == strings[0] for s in strings):
        return strings[0]

    prefix = commonprefix(strings)
    reversedStrings = [s[::-1] for s in strings]
    suffix = commonprefix(reversedStrings)[::-1]
    variableParts = [s[len(prefix):len(s) - len(suffix)] for s in strings]
    joined = "/".join(sorted(set(part.strip() for part in variableParts)))

    return f"{prefix}{joined}{suffix}"

# Format classes to send to ESP32
def formatClasses(classList, start=startHour, end=endHour):
    grouped = defaultdict(list)
    for name, classStart, classEnd in classList:
        grouped[(classStart, classEnd)].append(name)

    merged = []
    for (classStart, classEnd), names in grouped.items():
        mergedName = combineStrings(names)
        merged.append((mergedName, classStart, classEnd))

    mergedClasses = sorted(merged, key=lambda x: x[1])
            
    payload = bytearray()

    for c in mergedClasses:
        start = ((c[1]-startHour)&0x0F)
        duration = (c[2]-c[1])&0x0F
        payload.append((int)(start<<4 | duration)) # append class start-start hour and duration of class (4b and 4b)
        name_bytes = c[0].encode("utf-8")  # full string to bytes
        payload.append(len(name_bytes))    # Put length of classname for parsing (1B)
        payload.extend(name_bytes)         # name itself
    if (len(payload)==0): payload = b'\x00'
    return payload

# Obtain class information
def processClassroomSchedule(room, startDate, endDate):
    scheduleUrl = (
        room.get("reserves")       # Get classroom url
        .replace("json", "ics")    # Change requested format to iCal
        + f"&client_id={CLIENT_ID}&data_inici={startDate}&data_fi={endDate}"
    )

    calendar = getSchedule(scheduleUrl)
    if not calendar:
        return

    events = extractEvents(calendar)
    print(events)
    return formatClasses(events, start=startHour, end=endHour)

messageReceived = False
receivedMessage = None

TIMEOUT = 3 # 3 seconds for each topic to receive something
nothing  = b'\x00'

def subscribe(client: mqtt_client, topic):
    global messageReceived, receivedMessage  # Needed to update globals

    def on_message(client, userdata, msg):
        global messageReceived, receivedMessage  # Needed inside the callback too
        print(f"Received `{msg.payload.hex()}` from `{msg.topic}` topic")
        messageReceived = True
        receivedMessage = msg.payload

    client.on_message = on_message  # Assign the callback
    client.subscribe(topic)

def waitForMessage():
    global messageReceived, receivedMessage

    messageReceived = False
    receivedMessage = None
    start_time = time.time()
    while time.time() - start_time < TIMEOUT:
        if messageReceived:
            return receivedMessage
        time.sleep(0.1)  # Wait a bit
    return None

def checkPrevious(type, client, id, today):
    topic = type+"/"+id+"/"+today

    subscribe(client, topic)
    prevMsg = waitForMessage()
    client.unsubscribe(topic)
    publish(client, "", topic) # clear previous topic


    topic = type+"/"+id
    subscribe(client, topic)
    currMsg = waitForMessage()
    client.unsubscribe(topic)
    if (currMsg[0]==0x00): currMsg = None

    '''
    |            sub      |         |
    | class/today | A6001 |   pub   |
    ---------------------------------
    |    0           0    |   0x00  |
    |    0           1    |    -    |
    |    1           0    |   prev  |
    |    1           1    |  concat |

    '''
    if (prevMsg == None and currMsg == None):
        print("No messages at all!")
        publish(client, nothing, topic)
        return False
    elif (prevMsg != None and currMsg == None):
        print("Messages at dated topic")
        publish(client, prevMsg, topic)
    elif (prevMsg != None and currMsg != None):
        print("Messages in both topics")
        publish(client, prevMsg+currMsg, topic)
    else:
        print("Only at current topic")
    return True


def sendMeta(newAnn, newChanges, client, id):
    topic = "meta/"+id
    subscribe(client, topic)
    currMeta = waitForMessage()[0]
    client.unsubscribe(topic)
    if (newAnn):
        currMeta = currMeta | 0x08

    if (newChanges):
        currMeta = currMeta | 0x010

    publish(client, bytes([currMeta]), topic)

def main():
    startDate, endDate, today = getToday()
    classrooms = getClassrooms()
    client = connectMQTT()
    client.loop_start()

    for room in classrooms:
        id = room.get('id')
        topic = "schedule/"+id
        if (room.get('id') == "A6001"):
            payload = processClassroomSchedule(room, startDate, endDate)
            publish(client, payload, topic)

            newAnn = checkPrevious("announcements", client, id, today)
            newChanges = checkPrevious("changes", client, id, today)
            sendMeta(newAnn, newChanges, client, id)

    client.loop_stop()

if __name__ == "__main__":
    main()
