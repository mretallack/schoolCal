#include <iostream>
#include <fstream>

#include <string.h>
#include <vector>

#include <libical/ical.h>
#include <curl/curl.h>

#include "calendar_entry.hpp"


icalcomponent* create_new_calendar()
{

    /* variable definitions */
    icalcomponent* calendar;
   // struct icaltimetype atime = icaltime_from_timet( time(0),0);
    struct icalperiodtype rtime;

    /* Define a time type that will use as data later. */
    rtime.start = icaltime_from_timet( time(0),0);
    rtime.end = icaltime_from_timet( time(0),0);
    rtime.end.hour++;

    /* Create calendar and add properties */

    calendar = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);

    /* Nearly every libical function call has the same general
       form. The first part of the name defines the 'class' for the
       function, and the first argument will be a pointer to a struct
       of that class. So, icalcomponent_ functions will all take
       icalcomponent* as their first argument. */

    /* The next call creates a new proeprty and immediately adds it to the
       'calendar' component. */

    icalcomponent_add_property(
        calendar,
        icalproperty_new_version("2.0")
        );


    /* Here is the short version of the memory rules:
         If the routine name has "new" in it:
             Caller owns the returned memory.
             If you pass in a string, the routine takes the memory.
         If the routine name has "add" in it:
             The routine takes control of the component, property,
             parameter or value memory.
         If the routine returns a string ( "get" and "as_ical_string" )
             The library owns the returned memory.
          There are more rules, so refer to the documentation for more
          details.
    */

    icalcomponent_add_property(
        calendar,
        icalproperty_new_prodid("-//RDU Software//NONSGML HandCal//EN")
        );

    return calendar;
}


std::vector<calendar_entry*> json_parse(cJSON  * jobj)
{
	std::vector<calendar_entry*> returnList;
	int i;

	for (i = 0 ; i < cJSON_GetArraySize(jobj) ; i++)
	{
		cJSON * subitem = cJSON_GetArrayItem(jobj, i);
		calendar_entry *calEntry = new calendar_entry(subitem);

		returnList.push_back(calEntry);
	}

	return(returnList);
}

struct write_result
{
	char *data;
	int pos;
};


static const size_t BUFFER_SIZE = 100000;

static size_t curl_write( void *ptr, size_t size, size_t nmemb, void *stream) {

	struct write_result *result = (struct write_result *)stream;

	/* Will we overflow on this write? */
	if(result->pos + size * nmemb >= BUFFER_SIZE - 1)
	{
		fprintf(stderr, "curl error: too small buffer\n");
		return 0;
	}

	/* Copy curl's stream buffer into our own buffer */
	memcpy(result->data + result->pos, ptr, size * nmemb);

	/* Advance the position */
	result->pos += size * nmemb;

	return size * nmemb;
}


bool loadInEvents(int days, std::string &returnData)
{
	bool blnReturn = false;
	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if(curl)
	{
		char buff[200];
		// ok, use now as the start date
		time_t startDate = time(NULL);
		time_t endDate = startDate+ (days * 24 * 60 * 60);

		snprintf(buff, sizeof(buff), "start=%lld&end=%lld", (long long)startDate, (long long)endDate);
		std::string postRequest = buff;

		curl_easy_setopt(curl, CURLOPT_URL, "http://www.stmaryjosephswool.dorset.sch.uk/cms/cms_pages/get_calendar_source/x/x/0");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postRequest.c_str());

		/* Create the write buffer */
		char *data = new char[BUFFER_SIZE+1];

		struct write_result write_result = {
			.data = data,
			.pos = 0
		};

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);

		res = curl_easy_perform(curl);

		if(res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				  curl_easy_strerror(res));
		}
		else
		{
			// add null
			write_result.data[write_result.pos] = 0;
			returnData = write_result.data;
			blnReturn=true;
		}

		delete [] data;

		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	return(blnReturn);
}


int main(int argc, char *argv[])
{
	// ok, for now assume we have a JSON string
	std::string jsonString = "";
	const char *pcaOutputFile;

	if (argc != 2)
	{
		std::cout << "Please specify the output ica file\n";
	}
	else if (loadInEvents(90, jsonString)==false)
	{
		std::cout << "Failed to fetch the calendar data\n";
	}
	else
	{
		cJSON  * json;

		pcaOutputFile=argv[1];

		json = cJSON_Parse(jsonString.c_str());

		if (json!=NULL)
		{
			std::vector<calendar_entry*> eventList;
			eventList =json_parse (json);

			icalcomponent* newCal = create_new_calendar();

		    for(calendar_entry *entry : eventList)
		    {
		    	icalcomponent *newEntry = entry->generate_ical();

		    	icalcomponent_add_component(newCal,newEntry);

		    }

		    char*pcaTxt = icalcomponent_as_ical_string(newCal);

		    // ok, output the data
		    std::ofstream myfile;
		    myfile.open (pcaOutputFile, std::ios::out);
		    myfile << pcaTxt;
		    myfile.close();
		}
	}
}
