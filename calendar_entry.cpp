/*
 * calendar_entry.cpp
 *
 *  Created on: 25 Apr 2018
 *      Author: markretallack
 */

#include <string.h>

#include <iostream>
#include <algorithm>

#include "calendar_entry.hpp"

#include <libical/ical.h>


calendar_entry::calendar_entry(cJSON * subitem)
{
	this->id = getString(subitem,"id");
	this->title = getString(subitem,"title");

	allDay = getBoolean(subitem,"allDay");

	getTime(subitem,"start", this->start);
	getTime(subitem,"end", this->end);

}


calendar_entry& calendar_entry::operator=(const calendar_entry&)
{
	return(*this);
}


void calendar_entry::print()
{
	std::cout << "id: " + this->id + "\n";
}

bool calendar_entry::getTime(cJSON * subitem, const char *name, time_t &returnTime)
{
	bool blnRet = false;
	if (getTime(subitem, name, "%Y-%m-%dT%H:%M:%S", returnTime))
	{
		blnRet=true;
	}
	else if (getTime(subitem, name, "%Y-%m-%d", returnTime))
	{
		blnRet=true;
	}
	return(blnRet);
}

bool calendar_entry::getTime(cJSON * subitem, const char *name, const char *format, time_t &returnTime)
{
	bool blnRet = false;
	struct tm tmpTime;
	memset(&tmpTime,0, sizeof(tmpTime));
	// time is of the format, 2018-03-26T09:05:00
	char *res = strptime(getString(subitem,name).c_str(), format,&tmpTime);

	if ((res!=NULL)&&(*res == 0))
	{
		returnTime = mktime(&tmpTime);
		blnRet=true;
	}

	return(blnRet );
}

bool calendar_entry::getBoolean(cJSON * subitem, const char *name)
{
	bool blnRet=false;
	cJSON * boolVal = cJSON_GetObjectItem(subitem,name);

	if (cJSON_IsTrue(boolVal))
	{
		blnRet=true;
	}

	return(blnRet);
}

std::string calendar_entry::getString(cJSON * subitem, const char *name)
{
	std::string returnString;

	returnString = cJSON_GetStringValue(cJSON_GetObjectItem(subitem,name));

	return(returnString.c_str());
}

icalcomponent* calendar_entry::generate_ical()
{
	icalproperty* property;

	icalcomponent* event = icalcomponent_new(ICAL_VEVENT_COMPONENT);

	icalcomponent_add_property(
		event,
		icalproperty_new_summary((const char*)this->title.c_str())
		);

#if 0
	icalcomponent_add_property(
		event,
		icalproperty_new_description((const char*)sqlite3_column_text(stmt, colIndex))
		);
#endif

	// and add the start and end
	property = icalproperty_new_dtstart(icaltime_from_timet( this->start, this->allDay ? 1 : 0));
	icalcomponent_add_property(event,property);

	// and add the start and end
	property = icalproperty_new_dtend(icaltime_from_timet( this->end, this->allDay ? 1 : 0));
	icalcomponent_add_property(event,property);

	return(event);
}




