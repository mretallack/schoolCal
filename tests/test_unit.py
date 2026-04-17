"""Unit tests for ical_schoolcal — no network access required."""

from datetime import date, datetime
from unittest.mock import patch

from icalendar import Calendar

from ical_schoolcal import build_calendar, fetch_events, parse_dt


# -- parse_dt -----------------------------------------------------------------

class TestParseDt:
    def test_date_only(self):
        assert parse_dt("2026-05-04") == datetime(2026, 5, 4)

    def test_datetime_with_space(self):
        assert parse_dt("2026-05-04 12:00:00") == datetime(2026, 5, 4, 12)

    def test_datetime_iso(self):
        assert parse_dt("2026-05-04T09:30:00") == datetime(2026, 5, 4, 9, 30)

    def test_invalid_returns_none(self):
        assert parse_dt("not-a-date") is None

    def test_empty_returns_none(self):
        assert parse_dt("") is None


# -- build_calendar -----------------------------------------------------------

SAMPLE_EVENTS = [
    {
        "id": "abc-123",
        "title": "Sports Day",
        "allDay": True,
        "start": "2026-06-15",
        "end": "2026-06-15 12:00:00",
        "extendedProps": {
            "description": None,
        },
    },
    {
        "id": "def-456",
        "title": "Parents Evening",
        "allDay": False,
        "start": "2026-06-20T18:00:00",
        "end": "2026-06-20T20:00:00",
        "extendedProps": {"description": "Year 3 & 4"},
    },
]


class TestBuildCalendar:
    def test_returns_valid_ical(self):
        cal = build_calendar(SAMPLE_EVENTS)
        raw = cal.to_ical()
        assert raw.startswith(b"BEGIN:VCALENDAR")
        assert b"END:VCALENDAR" in raw

    def test_event_count(self):
        cal = build_calendar(SAMPLE_EVENTS)
        events = [c for c in cal.walk() if c.name == "VEVENT"]
        assert len(events) == 2

    def test_all_day_event_uses_date(self):
        cal = build_calendar(SAMPLE_EVENTS[:1])
        event = [c for c in cal.walk() if c.name == "VEVENT"][0]
        assert event.get("dtstart").dt == date(2026, 6, 15)

    def test_all_day_end_bumped(self):
        """iCal DTEND for all-day events is exclusive, so should be day+1."""
        cal = build_calendar(SAMPLE_EVENTS[:1])
        event = [c for c in cal.walk() if c.name == "VEVENT"][0]
        assert event.get("dtend").dt == date(2026, 6, 16)

    def test_timed_event_uses_datetime(self):
        cal = build_calendar(SAMPLE_EVENTS[1:])
        event = [c for c in cal.walk() if c.name == "VEVENT"][0]
        assert event.get("dtstart").dt == datetime(2026, 6, 20, 18, 0)
        assert event.get("dtend").dt == datetime(2026, 6, 20, 20, 0)

    def test_uid_format(self):
        cal = build_calendar(SAMPLE_EVENTS[:1])
        event = [c for c in cal.walk() if c.name == "VEVENT"][0]
        assert "abc-123-school@retallack.org.uk" in str(event.get("uid"))

    def test_description_from_extended_props(self):
        cal = build_calendar(SAMPLE_EVENTS[1:])
        event = [c for c in cal.walk() if c.name == "VEVENT"][0]
        assert "Year 3 & 4" in str(event.get("description"))

    def test_description_falls_back_to_title(self):
        cal = build_calendar(SAMPLE_EVENTS[:1])
        event = [c for c in cal.walk() if c.name == "VEVENT"][0]
        assert "Sports Day" in str(event.get("description"))

    def test_empty_events_list(self):
        cal = build_calendar([])
        events = [c for c in cal.walk() if c.name == "VEVENT"]
        assert len(events) == 0

    def test_prodid(self):
        cal = build_calendar([])
        assert b"HandCal" in cal.to_ical()

    def test_roundtrip_parse(self):
        """Output should be parseable by icalendar."""
        cal = build_calendar(SAMPLE_EVENTS)
        parsed = Calendar.from_ical(cal.to_ical())
        events = [c for c in parsed.walk() if c.name == "VEVENT"]
        assert len(events) == 2


# -- fetch_events (mocked) ----------------------------------------------------

class TestFetchEvents:
    @patch("ical_schoolcal.requests.get")
    def test_passes_date_range(self, mock_get):
        mock_get.return_value.json.return_value = []
        mock_get.return_value.raise_for_status = lambda: None

        fetch_events(30)

        args, kwargs = mock_get.call_args
        assert "start" in kwargs["params"]
        assert "end" in kwargs["params"]

    @patch("ical_schoolcal.requests.get")
    def test_returns_json(self, mock_get):
        mock_get.return_value.json.return_value = SAMPLE_EVENTS
        mock_get.return_value.raise_for_status = lambda: None

        result = fetch_events(30)
        assert len(result) == 2
