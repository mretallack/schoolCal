/*
 * calendar_entry.cpp
 *
 *  Created on: 25 Apr 2018
 *      Author: markretallack
 */

#include <string.h>

#include <iostream>
#include <algorithm>

#include <libical/ical.h>

#include "calendar_entry.hpp"

calendar_entry::calendar_entry(cJSON * subitem)
{
	this->id = getString(subitem,"id");
	this->title = getString(subitem,"title");
	this->details = getString(subitem,"details");

	allDay = getBoolean(subitem,"allDay");

	getTime(subitem,"start", this->start);
	getTime(subitem,"end", this->end);

    // the src sets the end to the same day if an all day event
    // so we want to "tip" it over the edge into a new day
    if (allDay)
    {
        this->end+=60;
    }
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
		returnTime = timegm(&tmpTime);
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

	std::string newUID = this->id + "-school@retallack.org.uk";

	property = icalproperty_new_uid(newUID.c_str());
	icalcomponent_add_property(event, property);


	icalcomponent_add_property(
		event,
		icalproperty_new_summary((const char*)this->title.c_str())
		);

#if 1
	std::string description=details;
	if (description.empty()) 
	{
		description=this->title;
	}
	icalcomponent_add_property(
		event,
		icalproperty_new_description(description.c_str())
		);
#endif

	// for now the timezone from the JSON is always UK, we could
	// use the timezone entry, but I dont expect things to move
	//icaltime_from_timet
	//const char *line = "Europe/London";
	//icaltimezone *local_timezone = icaltimezone_get_builtin_timezone(line);

	property = icalproperty_new_tzid("Europe/London");
	icalcomponent_add_property( event, property );


	// the DSTAMP must exist
	property = icalproperty_new_dtstamp( icaltime_from_timet_with_zone(time(NULL),0,NULL) );
	icalcomponent_add_property( event, property );

	// and add the start and end
	property = icalproperty_new_dtstart(icaltime_from_timet_with_zone( this->start, this->allDay ? 1 : 0,NULL));
	icalcomponent_add_property(event,property);

	// and add the start and end
	property = icalproperty_new_dtend(icaltime_from_timet_with_zone( this->end, this->allDay ? 1 : 0, NULL));
	icalcomponent_add_property(event,property);


	return(event);
}




