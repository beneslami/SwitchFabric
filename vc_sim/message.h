extern void init_messages() ;
extern MESSAGE *new_message(int dest, int length) ;
extern void check_messages_and_requests() ;
extern void inject_test_message(int dest, int length, int src_node_nr) ;
extern void route_active_messages() ;
extern void advance_active_messages() ;
extern void route_active_channels() ;
extern void advance_active_channels() ;
extern void print_message(MESSAGE *message) ; 
extern void print_lane(LANE *lane) ;
extern void activate_message(MESSAGE *message) ;
extern void clean_messages() ;
extern int nr_lanes_busy(CHANNEL *channel) ;
