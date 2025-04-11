import requests
import icalendar
from datetime import datetime, timedelta
from data_secure import CLIENT_ID
import re
from os.path import commonprefix
from collections import defaultdict

FORMAT = "ics"  # ics is easier to treat for calendars

# Get today's schedule (start_date <= today < end_date)
today = datetime.now()  # - timedelta(days=4) # Testing in the weekend
START = (today-timedelta(days=45)).strftime("%Y-%m-%d")
END = (today - timedelta(days=44)).strftime("%Y-%m-%d")

# API URL with the necessary additions
url = f"https://api.fib.upc.edu/v2/aules/?client_id={CLIENT_ID}&format=json"

response = requests.get(url)  # Obtain the classroom list from API

def combine_similar_strings(strings):
    if not strings:
        return ""
    
    # If all strings are the same, return the first
    if all(s == strings[0] for s in strings):
        return strings[0]

    # Step 1: Get common prefix
    prefix = commonprefix(strings)

    # Step 2: Get common suffix (reverse and use commonprefix)
    reversed_strings = [s[::-1] for s in strings]
    suffix = commonprefix(reversed_strings)[::-1]

    # Step 3: Extract variable parts
    variable_parts = [s[len(prefix):len(s)-len(suffix)] for s in strings]

    # Step 4: Join and clean
    joined = "/".join(sorted(set(part.strip() for part in variable_parts)))

    # Step 5: Return the result
    return f"{prefix}{joined}{suffix}"

def expand_to_hour_slots(classes, start_hour=8, end_hour=20):
    total_slots = end_hour - start_hour + 1
    class_names = [""] * total_slots
    durations = [0] * total_slots

    for name, start, end in classes:
        duration = end - start
        for i in range(duration):
            index = start - start_hour + i
            if 0 <= index < total_slots:
                class_names[index] = name
                durations[index] = duration if i == 0 else -i
        # Mark the end with 0 if within bounds
        end_index = end - start_hour
        if 0 <= end_index < total_slots:
            durations[end_index] = 0

    return class_names, durations

if response.status_code == 200:
    classrooms = response.json().get("results")
    for room in classrooms:  # Parse classrooms
        # Specific classroom schedule
        url = room.get('reserves').replace('json', FORMAT) + f"&client_id={CLIENT_ID}&data_inici={START}&data_fi={END}"
        response = requests.get(url)  # Obtain the schedule from API

        if response.status_code == 200:
            calendar = icalendar.Calendar.from_ical(response.text)
            classes = []

            # Extract classes
            for event in calendar.walk("VEVENT"):
                start_time = int(event.get("DTSTART").dt.strftime("%#H"))  # Get start time
                end_time = int((event.get("DTEND").dt + timedelta(minutes=1)).strftime("%#H"))  # Round end time
                name = str(event.get("DESCRIPTION")) if event.get("DESCRIPTION") is not None else str(event.get("SUMMARY"))

                classes.append((name, start_time, end_time))

            # Merge classes with same time window
            grouped = defaultdict(list)
            for name, start, end in classes:
                grouped[(start, end)].append(name)

            merged_classes = []
            for (start, end), names in grouped.items():
                merged_name = combine_similar_strings(names)
                merged_classes.append((merged_name, start, end))

            classes = sorted(merged_classes, key=lambda x: x[1])  # Sort by start time
            
            print(room.get('id'))
            print(expand_to_hour_slots(classes, start_hour=8, end_hour=21))
            print(classes)

        else:
            print(f"Error {response.status_code}: {response.text}")

else:
    print(f"Error {response.status_code}: {response.text}")
