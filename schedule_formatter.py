import requests
import icalendar
from datetime import datetime
from datetime import timedelta

CLIENT_ID = "t7SuHCjdalstLwG6ElnrJgn8URLbMYIwAOrkzcIa" # Public application of the Racó API
FORMAT = "ics"  # ics is easier to treat

# Classroom's schedule. To get all of them they should be added in a list and do an API call for each, or not add a filter for classroom 
CLASS = "A6001"

# Get today's schedule (start_date <= today < end_date)
today = datetime.now() - timedelta(days=4)
START = today.strftime("%Y-%m-%d")
END = (today + timedelta(days=1)).strftime("%Y-%m-%d")

# API URL with the necessary additions
url = f"https://api.fib.upc.edu/v2/aules/{CLASS}/reserves/?format={FORMAT}&client_id={CLIENT_ID}&data_inici={START}&data_fi={END}"

response = requests.get(url) # Obtain the result from API

if response.status_code == 200:
    # Parse the ICS content
    calendar = icalendar.Calendar.from_ical(response.text)
    classes = []

    # Extract classes
    for event in calendar.walk("VEVENT"):
        start_time = int(event.get("DTSTART").dt.strftime("%#H"))                   # Get start time
        end_time = int((event.get("DTEND").dt + timedelta(minutes=1)).strftime("%#H"))   # Get end time (rounded to the hour)
        name = str(event.get("DESCRIPTION")) if event.get("DESCRIPTION") != None else str(event.get("SUMMARY"))         # Class' name
        #location = event.get("LOCATION")       # Get class location (already known)

        #print(f"Class: {name}")
        #print(f"Time: {start_time}-{end_time}")
        # print(f"Location: {location}")
        #print("-" * 40)

        classes.append((name, start_time, end_time))
    classes = sorted(classes, key=lambda x: x[1])
    print(classes)

else:
    print(f"Error {response.status_code}: {response.text}")