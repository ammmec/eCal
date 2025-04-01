import requests
import icalendar
from datetime import datetime
from datetime import timedelta
from data_secure import CLIENT_ID

FORMAT = "ics"  # ics is easier to treat for calendars

# Get today's schedule (start_date <= today < end_date)
today = datetime.now() # - timedelta(days=4) # Testing in the weekend
START = today.strftime("%Y-%m-%d")
END = (today + timedelta(days=1)).strftime("%Y-%m-%d")

# API URL with the necessary additions
url = f"https://api.fib.upc.edu/v2/aules/?client_id={CLIENT_ID}&format=json"

response = requests.get(url) # Obtain the classroom list from API

if response.status_code == 200:
    classrooms = response.json().get("results")
    for room in classrooms: # Parse clasrooms
        print(room.get('id'))

        # Specific classroom schedule
        url = room.get('reserves').replace('json', FORMAT) + f"&client_id={CLIENT_ID}&data_inici={START}&data_fi={END}"
        response = requests.get(url) # Obtain the schedule from API

        if response.status_code == 200:
            calendar = icalendar.Calendar.from_ical(response.text)
            classes = []
            # Extract classes
            for event in calendar.walk("VEVENT"):
                start_time = int(event.get("DTSTART").dt.strftime("%#H"))                   # Get start time
                end_time = int((event.get("DTEND").dt + timedelta(minutes=1)).strftime("%#H"))   # Get end time (rounded to the hour)
                name = str(event.get("DESCRIPTION")) if event.get("DESCRIPTION") != None else str(event.get("SUMMARY"))         # Class' name
                #location = event.get("LOCATION")       # Get class location (already known)

                classes.append((name, start_time, end_time))
            classes = sorted(classes, key=lambda x: x[1]) # Sort by start time of class
            print(classes)
        else:
            print(f"Error {response.status_code}: {response.text}")

else:
    print(f"Error {response.status_code}: {response.text}")