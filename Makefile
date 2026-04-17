VENV=venv
PYTHON=$(VENV)/bin/python3
PIP=$(VENV)/bin/pip

.PHONY: all run test lint clean

all: venv

venv: $(VENV)/bin/activate

$(VENV)/bin/activate: requirements.txt
	python3 -m venv $(VENV)
	$(PIP) install -r requirements.txt
	$(PIP) install pytest flake8
	touch $(VENV)/bin/activate

run: venv
	$(PYTHON) ical_schoolcal.py output.ics

test: venv
	$(PYTHON) -m pytest tests/ -v --ignore=tests/test_integration.py

lint: venv
	$(PYTHON) -m flake8 ical_schoolcal.py tests/

clean:
	rm -rf $(VENV) __pycache__ .pytest_cache
	rm -f output.ics
