#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <parser.h>
#include <parserInternals.h>
#include "main.h"

static xmlSAXHandler mySAXParseCallbacks;

int main(void)
{
	xmlParserCtxtPtr ctxt_ptr;
	condition_parse_state parsing_state;

	memset(&mySAXParseCallbacks, sizeof(mySAXParseCallbacks), 0);
	mySAXParseCallbacks.startDocument = start_document;
	mySAXParseCallbacks.endDocument = end_document;
	mySAXParseCallbacks.startElement = start_element;
	mySAXParseCallbacks.endElement = end_element;
	mySAXParseCallbacks.characters = chars_found;

	ctxt_ptr = xmlCreateFileParserCtxt("ch3a.xml");
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
	condition_parse_state *state_ptr;

	state_ptr = (condition_parse_state *)ctx;

	state_ptr->current_state = parse_start_s;
	state_ptr->condition_count = 0;
	printf("Document start\n");
}

static void end_document(void *ctx) {
	condition_parse_state *state_ptr;

	state_ptr = (condition_parse_state *)ctx;

	state_ptr->current_state = parse_finish_s;
	printf("Document end\n");
}

static void start_element(void *ctx, const CHAR *name, const CHAR **attrs) {
	const char *attr_ptr;
	int curr_attr = 0;
	condition_parse_state *state_ptr;
	parse_state curr_state;
	parse_event curr_event;

	state_ptr = (condition_parse_state *)ctx;
	curr_state = state_ptr->current_state;
	curr_event = get_event_from_name(name);

	state_ptr->current_state = state_event_machine(curr_state, curr_event);

	if(curr_event == parse_condition_e) {
		state_ptr->condition_count++;
	}
	if(curr_event == parse_safetylist_e
		|| curr_event == parse_finishlist_e
		|| curr_event == parse_recordlist_e
		) {
		state_ptr->condition_count = 0;
	}

	if(state_ptr->current_state == parse_safetylist_s
		|| state_ptr->current_state == parse_finishlist_s
		|| state_ptr->current_state == parse_recordlist_s) {
		printf("\nElement %s started\n", name);
		if(attrs) {
			attr_ptr = *attrs;
			while(attr_ptr) {
				printf("\tAttribute %s\n", attr_ptr);
				curr_attr++;
				attr_ptr = *(attrs + curr_attr);
			}
		}
	}

	if(state_ptr->current_state == parse_child_s) {
		printf("\nElement %s started\n", name);
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
	condition_parse_state *state_ptr;
	parse_state curr_state;
	parse_event curr_event;

	state_ptr = (condition_parse_state *)ctx;
	curr_state = state_ptr->current_state;
	//curr_event = parse_end_element_e;
	curr_event = get_event_from_name(name);

	state_ptr->current_state = state_event_machine(curr_state, curr_event);

	if(state_ptr->current_state == parse_safetylist_s
		|| state_ptr->current_state == parse_finishlist_s
		|| state_ptr->current_state == parse_recordlist_s) {
			printf("kjg %d ", state_ptr->condition_count);
	}
	//printf("Element %s ended %d\n", name, state_ptr->current_state);
}

static void chars_found(void *ctx, const CHAR *chars, int len) {
	char buff[CHAR_BUFFER+1];
	condition_parse_state *state_ptr;

	state_ptr = (condition_parse_state *)ctx;

	if(len > CHAR_BUFFER) len = CHAR_BUFFER;
	strncpy(buff, chars, len);
	buff[len] = '\0';

//	printf("Found %d characters: %s\n", len, buff);
/*
	switch(state_ptr->current_state) {
		case parse_start_s:
		case parse_finish_s:
			break;
		case parse_safetylist_s:
			printf("safetylist %s (%d)\n", buff, state_ptr->condition_count);
			break;
		case parse_finishlist_s:
			printf("finishlist %s (%d)\n", buff, state_ptr->condition_count);
			break;
		case parse_recordlist_s:
			printf("recordlist %s (%d)\n", buff, state_ptr->condition_count);
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
	}*/
}

const struct {
	const char *name;
	parse_event event;
} events[] = {
	{"CONDITION", parse_condition_e},
	{"CHILD", parse_child_e},
	{"SAFETYLIST", parse_safetylist_e},
	{"FINISHLIST", parse_finishlist_e},
	{"RECORDLIST", parse_recordlist_e}
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
	{parse_channelmap_e, parse_valid_string_s},
	{parse_schedulelist_e, parse_valid_string_s},
	{parse_schedule_e, parse_skip_string_s},
	{parse_safetylist_e, parse_safetylist_s},
	{parse_condition_e, parse_valid_string_s},
	{parse_child_e, parse_child_s},
	{parse_looplist_e, parse_skip_string_s},
	{parse_steplist_e, parse_valid_string_s},
	{parse_step_e, parse_skip_string_s},
	{parse_finishlist_e, parse_finishlist_s},
	{parse_recordlist_e, parse_recordlist_s},
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
