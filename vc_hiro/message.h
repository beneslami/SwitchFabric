extern void init_messages() ;
extern MESSAGE *new_message(NODE *dest, int length) ;
extern void check_messages_and_requests() ;
extern void inject_test_message(NODE *dest, int length, int src_node_nr) ;
extern void request_next_lanes() ;
extern void allocate_next_lanes() ;
extern void request_all_channels_of_messages() ;
extern void allocate_channels_for_flits() ;
extern void print_message(MESSAGE *message) ; 
extern void print_lane(LANE *lane) ;
extern void activate_message(MESSAGE *message) ;
extern void clean_messages() ;
