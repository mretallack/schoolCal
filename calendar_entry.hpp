/*
 * calendar_entry.hpp
 *
 *  Created on: 25 Apr 2018
 *      Author: markretallack
 */

#ifndef CALENDAR_ENTRY_HPP_
#define CALENDAR_ENTRY_HPP_

#include <string>
#include <vector>

#include "cJSON.h"

#include <libical/ical.h>


class calendar_entry
{
public:
	calendar_entry(cJSON * subitem);


	void print();

	icalcomponent* generate_ical();

private:

	std::string id;
	std::string title;
	std::string details;
	std::string start_obj;

	bool allDay;

	time_t start;
	std::string end_obj;
	time_t end;
	std::string event_private;

	bool getBoolean(cJSON * subitem, const char *name);
	std::string getString(cJSON * subitem, const char *name);
	bool getTime(cJSON * subitem, const char *name, time_t &returnTime);
	bool getTime(cJSON * subitem, const char *name, const char *format, time_t &returnTime);

	calendar_entry(const calendar_entry&);

	calendar_entry& operator=(const calendar_entry&);
};



#endif /* CALENDAR_ENTRY_HPP_ */
