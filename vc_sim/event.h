extern EVENT *new_event(void (*funct)(), char *data) ;
extern void init_event_list()  ;
extern void schedule_event(int time, EVENT *event) ;
extern void process_events() ;
extern void reset_event_list() ;

