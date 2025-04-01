#ifndef __MAIN_H__
#define __MAIN_H__

#define CHAR_BUFFER 1024

typedef enum {
	parse_start_e = 0,
	parse_finish_e,
	parse_catalog_e,
	parse_dvd_e,
	parse_title_e,
	parse_price_e,
	parse_director_e,
	parse_actors_e,
	parse_actor_e,
	parse_year_made_e,
	parse_end_element_e,
	parse_other_e
} parse_event;

typedef enum {
	parse_start_s = 0,
	parse_finish_s,
	parse_dvd_s,
	parse_price_s,
	parse_actor_s,
	parse_year_made_s,
	parse_valid_string_s,
	parse_skip_string_s,
	parse_unknown_s
} parse_state;

int 	main(void);
static void start_document(void *);
static void end_document(void *);
static void start_element(void *, const CHAR *, const CHAR **);
static void end_element(void *, const CHAR *);
static void chars_found(void *, const CHAR *, int);
static parse_event get_event_from_name(const char *);
static parse_state state_event_machine(parse_state, parse_event);

#endif
