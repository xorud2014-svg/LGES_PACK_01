#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <parser.h>
#include <parserInternals.h>
#include "main.h"

typedef struct {
	parse_state current_state;
	int	actors_this_movie;
} catalog_parse_state;

static xmlSAXHandler mySAXParseCallbacks;

int main(void)
{
	xmlParserCtxtPtr ctxt_ptr;
	catalog_parse_state parsing_state;

	memset(&mySAXParseCallbacks, sizeof(mySAXParseCallbacks), 0);
	mySAXParseCallbacks.startDocument = start_document;
	mySAXParseCallbacks.endDocument = end_document;
	mySAXParseCallbacks.startElement = start_element;
	mySAXParseCallbacks.endElement = end_element;
	mySAXParseCallbacks.characters = chars_found;

	ctxt_ptr = xmlCreateFileParserCtxt("dvdcatalog.xml");
	if(!ctxt_ptr) {
		fprintf(stderr, "Failed to create file parser\n");
		exit(EXIT_FAILURE);
	}

	ctxt_ptr->sax = &mySAXParseCallbacks; //Set callback map
	ctxt_ptr->userData = &parsing_state;

	xmlParseDocument(ctxt_ptr);
	if(!ctxt_ptr->wellFormed) {
		fprintf(stderr, "Document not well formed\n");
	}
	ctxt_ptr->sax = NULL;

	printf("XML version %s, encoding %s\n",
		ctxt_ptr->version, ctxt_ptr->encoding);

	xmlFreeParserCtxt(ctxt_ptr);

	printf("Parsing complete\n");
	exit(EXIT_SUCCESS);
}

static void start_document(void *ctx) {
	catalog_parse_state *state_ptr;

	state_ptr = (catalog_parse_state *)ctx;

	state_ptr->current_state = parse_start_s;
	state_ptr->actors_this_movie = 0;
	printf("Document start\n");
}

static void end_document(void *ctx) {
	catalog_parse_state *state_ptr;

	state_ptr = (catalog_parse_state *)ctx;

	state_ptr->current_state = parse_finish_s;
	printf("Document end\n");
}

static void start_element(void *ctx, const CHAR *name, const CHAR **attrs) {
	const char *attr_ptr;
	int curr_attr = 0;
	catalog_parse_state *state_ptr;
	parse_state curr_state;
	parse_event curr_event;

	state_ptr = (catalog_parse_state *)ctx;
	curr_state = state_ptr->current_state;
	curr_event = get_event_from_name(name);

	state_ptr->current_state = state_event_machine(curr_state, curr_event);

	if(curr_event == parse_actor_e) {
		state_ptr->actors_this_movie++;
	}
	if(curr_event == parse_actors_e) {
		state_ptr->actors_this_movie = 0;
	}

	if(state_ptr->current_state == parse_dvd_s) {
		printf("Element %s started\n", name);
		if(attrs) {
			attr_ptr = *attrs;
			while(attr_ptr) {
				printf("\tAttribute %s\n", attr_ptr);
				curr_attr++;
				attr_ptr = *(attrs + curr_attr);
			}
		}
	}
}

static void end_element(void *ctx, const CHAR *name) {
	catalog_parse_state *state_ptr;
	parse_state curr_state;
	parse_event curr_event;

	state_ptr = (catalog_parse_state *)ctx;
	curr_state = state_ptr->current_state;
	curr_event = parse_end_element_e;

	state_ptr->current_state = state_event_machine(curr_state, curr_event);

//	printf("Element %s ended\n", name);
}

static void chars_found(void *ctx, const CHAR *chars, int len) {
	char buff[CHAR_BUFFER+1];
	catalog_parse_state *state_ptr;

	state_ptr = (catalog_parse_state *)ctx;

	if(len > CHAR_BUFFER) len = CHAR_BUFFER;
	strncpy(buff, chars, len);
	buff[len] = '\0';

//	printf("Found %d characters: %s\n", len, buff);

	switch(state_ptr->current_state) {
		case parse_start_s:
		case parse_finish_s:
		case parse_dvd_s:
			break;
		case parse_price_s:
			printf("Price %s\n", buff);
			break;
		case parse_actor_s:
			printf("Actor %s (%d)\n", buff, state_ptr->actors_this_movie);
			break;
		case parse_year_made_s:
			printf("Year %s\n", buff);
			break;
		case parse_valid_string_s:
			printf("Other valid %s\n", buff);
			break;
		case parse_skip_string_s:
			break;
		case parse_unknown_s:
			break;
		default:
			printf("DEBUG default case in chars_found %d\n",
				state_ptr->current_state);
			break;
	}
}

const struct {
	const char *name;
	parse_event event;
} events[] = {
	{"catalog", parse_catalog_e},
	{"dvd", parse_dvd_e},
	{"title", parse_title_e},
	{"price", parse_price_e},
	{"director", parse_director_e},
	{"actor", parse_actor_e},
	{"actors", parse_actors_e},
	{"year_made", parse_year_made_e}
};

static parse_event get_event_from_name(const char *name) {
	int i;

	for(i=0; i < sizeof(events) / sizeof(*events); i++) {
		if(!strcmp(name, events[i].name)) return events[i].event;
	}

	return parse_other_e;
}

const struct {
	const parse_event pe;
	parse_state ns;
} event_state[] = {
	{parse_start_e, parse_start_s},
	{parse_finish_e, parse_finish_s},
	{parse_dvd_e, parse_dvd_s},
	{parse_price_e, parse_price_s},
	{parse_actor_e, parse_actor_s},
	{parse_year_made_e, parse_year_made_s},
	{parse_title_e, parse_valid_string_s},
	{parse_director_e, parse_valid_string_s},
	{parse_catalog_e, parse_skip_string_s},
	{parse_actors_e, parse_skip_string_s},
	{parse_other_e, parse_unknown_s},
	{parse_end_element_e, parse_skip_string_s}
};

static parse_state state_event_machine(parse_state curr_state,
	parse_event curr_event) {
	int i;

	for(i=0; i < sizeof(event_state) / sizeof(*event_state); i++) {
		if(curr_event == event_state[i].pe) return event_state[i].ns;
	}

	return parse_unknown_s;
}
