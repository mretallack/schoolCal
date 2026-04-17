# schoolCal

Fetches calendar events from the [St Mary & St Joseph's School](https://www.smsjwool.dorset.sch.uk/calendar) website and exports them as an iCal (`.ics`) file that can be imported into Google Calendar, Apple Calendar, Outlook, etc.

## How It Works

The school website exposes a JSON API at `/events`. This tool:

1. Queries the API for events within a configurable date range (default: 360 days ahead)
2. Parses the JSON response
3. Generates a standards-compliant iCal file with proper all-day event handling and timezone support

## Requirements

- Python 3.10+

## Quick Start

```bash
# Set up the virtual environment and install dependencies
make

# Fetch events and write to a file
make run                                          # writes output.ics
venv/bin/python3 ical_schoolcal.py my_calendar.ics  # custom output path
```

## Makefile Targets

| Target       | Description                                      |
|--------------|--------------------------------------------------|
| `make`       | Create venv and install dependencies              |
| `make run`   | Fetch events and write `output.ics`               |
| `make test`  | Run unit tests                                    |
| `make lint`  | Run flake8 linter                                 |
| `make clean` | Remove venv, caches, and generated files          |

## Running Tests

```bash
make test
```

Tests are in the `tests/` directory:

- **Unit tests** — test date parsing, calendar building, and event conversion using mocked data (run by default)
- **Integration tests** — hit the live school API; skipped by default, run with:

```bash
venv/bin/python3 -m pytest tests/ -m integration -v
```

Integration tests are also run in CI to verify the upstream API hasn't changed.

## CI/CD

GitHub Actions runs on every push and pull request:

- **Lint** — flake8 style checks
- **Unit tests** — fast, no network required
- **Integration tests** — verifies the live API still returns valid data

## Project Structure

```
├── ical_schoolcal.py   # Main script
├── requirements.txt    # Runtime dependencies
├── Makefile            # Build/test/run automation
├── tests/
│   ├── test_unit.py    # Unit tests (mocked data)
│   └── test_integration.py  # Live API tests
└── .github/
    └── workflows/
        └── ci.yml      # GitHub Actions workflow
```

## License

[MIT](LICENSE)
