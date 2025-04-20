import requests
import icalendar
from datetime import datetime, timedelta
from data_secure import CLIENT_ID
from os.path import commonprefix
from collections import defaultdict

startHour = 8       # Classes start at 8 in the morning...
endHour = 21        # ... and end at 21h

# Get today's date (start_date <= today < end_date)
def getToday():
    today = datetime.now()
    startDate = (today + timedelta(days=9)).strftime("%Y-%m-%d")
    endDate = (today + timedelta(days=10)).strftime("%Y-%m-%d")
    return startDate, endDate

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

    totalSlots = end - start + 1
    classNames = [""] * totalSlots
    durations = [0] * totalSlots

    for name, classStart, classEnd in mergedClasses:
        duration = classEnd - classStart
        for i in range(duration):
            index = classStart - start + i
            if 0 <= index < totalSlots:
                classNames[index] = name
                durations[index] = duration if i == 0 else -i
        endIndex = classEnd - start
        if 0 <= endIndex < totalSlots:
            durations[endIndex] = 0

    return classNames, durations

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
    slots = formatClasses(events, start=startHour, end=endHour)

    print(room.get("id"))
    print(slots)

def main():
    startDate, endDate = getToday()
    classrooms = getClassrooms()

    for room in classrooms:
        processClassroomSchedule(room, startDate, endDate)

if __name__ == "__main__":
    main()
