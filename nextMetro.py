from datetime import datetime

class MetroTimetable:

    def __init__(self, hour, minute, freq):
        self.hour = hour
        self.minute = minute
        self.freq = freq


weekday = [MetroTimetable(4, 8, 10),  MetroTimetable(6, 8, 4), MetroTimetable(9, 8, 5), MetroTimetable(14, 8, 4), MetroTimetable(18, 40, 10), MetroTimetable(23, 50, 10)]
friday = [MetroTimetable(4, 8, 10),  MetroTimetable(6, 8, 4), MetroTimetable(9, 8, 5), MetroTimetable(14, 8, 4), MetroTimetable(18, 40, 10), MetroTimetable(0, 50, 10)]
saturday = [MetroTimetable(4, 8, 10),  MetroTimetable(0, 48, 10)]
sunday = [MetroTimetable(4, 8, 10),  MetroTimetable(22, 58, 0)]

def getTime():
    return datetime.now().time()

def getDay():
    return datetime.now().weekday()

def getSchedule():
    day = getDay()
    if day < 4:
        return weekday
    elif day == 4:
        return friday
    elif day == 5:
        return saturday
    elif day == 6:
        return sunday

def getNextMetro(hour, minute, schedule):
    
    if hour < schedule[0].hour or hour > schedule[len(schedule)-1].hour:
        nextMetro = [schedule[0].hour, schedule[0].minute]
    else:
        for i in range(len(schedule)):
            if hour > schedule[i].hour or hour == schedule[i].hour and minute > schedule[i].minute:
                if hour < schedule[i+1].hour or hour == schedule[i+1].hour and minute < schedule[i+1].minute:
                    nextMetro = findNextMetro(hour, minute, schedule[i])

    return nextMetro


def findNextMetro(hour, minute, timetable):
    metroHour = timetable.hour
    metroMinute = timetable.minute
    while metroHour < hour or metroMinute < minute:
        metroMinute += timetable.freq
        if metroMinute >= 60:
            metroMinute -= 60
            metroHour += 1

    return [metroHour, metroMinute]

time = [getTime().hour, getTime().minute]

print(str(getNextMetro(2, 30, getSchedule())[1]-30) + " mins")