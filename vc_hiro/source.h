extern MESSAGE *new_message(SOURCE *source) ;
extern void source_event_funct(char *src) ;
extern void schedule_source_event_at_time(SOURCE *source, int time) ;
extern void schedule_initial_source_events() ; 
extern void free_messages_in_queue(SOURCE *source) ;
extern MESSAGE *new_source_message(SOURCE *source) ;
extern MESSAGE *create_new_message(SOURCE *source) ;
extern LANE *select_init_lane_func1(SOURCE *source) ;
extern int chaeck_new_message_can_enter(LANE *lane) ;
