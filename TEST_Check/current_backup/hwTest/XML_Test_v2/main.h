#ifndef __MAIN_H__
#define __MAIN_H__

#define CHAR_BUFFER 1024

typedef enum {
	parse_start_e = 0,
	parse_finish_e,
	parse_channelmap_e,
	parse_schedulelist_e,
	parse_schedule_e,
	parse_safetylist_e,
	parse_condition_e,
	parse_child_e,
	parse_looplist_e,
	parse_steplist_e,
	parse_step_e,
	parse_finishlist_e,
	parse_recordlist_e,
	parse_end_element_e,
	parse_other_e
} parse_event;

typedef enum {
	parse_start_s = 0,
	parse_finish_s,
	parse_safetylist_s,
	parse_finishlist_s,
	parse_recordlist_s,
	parse_child_s,
	parse_valid_string_s,
	parse_skip_string_s,
	parse_unknown_s
} parse_state;

typedef struct {
	parse_state current_state;
	int	condition_count;
} condition_parse_state;

int 	main(void);
static void start_document(void *);
static void end_document(void *);
static void start_element(void *, const CHAR *, const CHAR **);
static void end_element(void *, const CHAR *);
static void chars_found(void *, const CHAR *, int);
static parse_event get_event_from_name(const char *);
static parse_state state_event_machine(parse_state, parse_event);

#endif
