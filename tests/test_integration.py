"""Integration tests — hit the live school API.

Skipped by default. Run with:
    pytest tests/test_integration.py -m integration -v
"""

import pytest
from icalendar import Calendar

from ical_schoolcal import build_calendar, fetch_events

pytestmark = pytest.mark.integration


class TestLiveAPI:
    def test_fetch_returns_list(self):
        events = fetch_events(90)
        assert isinstance(events, list)

    def test_events_have_required_fields(self):
        events = fetch_events(90)
        if not events:
            pytest.skip("No events in the next 90 days")
        ev = events[0]
        assert "id" in ev
        assert "title" in ev
        assert "start" in ev
        assert "end" in ev
        assert "allDay" in ev

    def test_full_pipeline(self):
        """Fetch live data, build calendar, verify it parses."""
        events = fetch_events(90)
        if not events:
            pytest.skip("No events in the next 90 days")
        cal = build_calendar(events)
        raw = cal.to_ical()
        parsed = Calendar.from_ical(raw)
        vevents = [c for c in parsed.walk() if c.name == "VEVENT"]
        assert len(vevents) == len(events)
