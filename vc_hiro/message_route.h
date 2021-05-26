extern void init_messages() ;
extern void check_messages_and_requests() ;
extern void inject_test_message(int src_node_nr, int length) ;
extern void print_message(MESSAGE *message) ; 
extern void print_lane(LANE *lane) ;
extern void activate_message(MESSAGE *message) ;
extern void clean_messages() ;
extern int  source_message_pending(SOURCE *source) ;
extern void print_message_A() ;
extern void print_message_B() ;

