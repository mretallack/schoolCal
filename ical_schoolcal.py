#!/usr/bin/env python3
"""Fetch school calendar events and output as an iCal file."""

import sys
from datetime import datetime, timedelta, timezone

import requests
from icalendar import Calendar, Event

URL = "https://www.smsjwool.dorset.sch.uk/events"
DAYS_AHEAD = 360
UID_DOMAIN = "school@retallack.org.uk"
TIMEZONE = "Europe/London"


def fetch_events(days):
    now = datetime.now(timezone.utc)
    end = now + timedelta(days=days)
    params = {
        "start": now.strftime("%Y-%m-%dT%H:%M:%S+00:00"),
        "end": end.strftime("%Y-%m-%dT%H:%M:%S+00:00"),
    }
    headers = {
        "User-Agent": "Mozilla/5.0",
        "Referer": "https://www.smsjwool.dorset.sch.uk/calendar",
    }
    resp = requests.get(URL, params=params, headers=headers, timeout=30)
    resp.raise_for_status()
    return resp.json()


def parse_dt(s):
    """Parse date string in 'YYYY-MM-DD' or 'YYYY-MM-DD HH:MM:SS' format."""
    for fmt in ("%Y-%m-%dT%H:%M:%S", "%Y-%m-%d %H:%M:%S", "%Y-%m-%d"):
        try:
            return datetime.strptime(s, fmt)
        except ValueError:
            continue
    return None


def build_calendar(events):
    cal = Calendar()
    cal.add("prodid", "-//RDU Software//NONSGML HandCal//EN")
    cal.add("version", "2.0")

    for ev in events:
        event = Event()
        event.add("uid", f"{ev['id']}-{UID_DOMAIN}")
        event.add("summary", ev.get("title", ""))

        ext = ev.get("extendedProps") or {}
        description = ext.get("description") or ev.get("title", "")
        event.add("description", description)

        event.add("tzid", TIMEZONE)
        event.add("dtstamp", datetime.now(timezone.utc))

        all_day = ev.get("allDay", False)
        dt_start = parse_dt(ev.get("start", ""))
        dt_end = parse_dt(ev.get("end", ""))

        if dt_start:
            if all_day:
                event.add("dtstart", dt_start.date())
            else:
                event.add("dtstart", dt_start)

        if dt_end:
            if all_day:
                end_date = dt_end.date() + timedelta(days=1)
                event.add("dtend", end_date)
            else:
                event.add("dtend", dt_end)

        cal.add_component(event)

    return cal


def main():
    if len(sys.argv) != 2:
        print("Please specify the output ica file")
        sys.exit(1)

    output_file = sys.argv[1]

    events = fetch_events(DAYS_AHEAD)
    if not events:
        print("Failed to fetch the calendar data")
        sys.exit(1)

    cal = build_calendar(events)

    with open(output_file, "wb") as f:
        f.write(cal.to_ical())


if __name__ == "__main__":
    main()
