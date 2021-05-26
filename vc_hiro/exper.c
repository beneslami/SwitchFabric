/*---------------------------------------------------------------------------*/
/*
 |    network simulator commands
 |    Hiromichi Aoki April 20 1990
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "netdef.h"
#include "defs.h"
/*---------------------------------------------------------------------------*/
void set_FIX_only_VC_category_type_1() 
{
  int i, cure_nr ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 1 ;                 /* FIXED only */
  para_c->nr_special_lanes = 1 ;            /* <--------- */
  para_c->nr_fixed_lanes = 15 ;             /* <--------- */
  set_FIX_ONLY_boundary() ;
  para_c->max_class = 14 ;                  /* <--------- */
  cure_nr = 0 ;
  for(i=0; i<(para_c->max_class+1); i++) {
    set_class_i(i, 1, cure_nr) ;
    cure_nr = i+1 ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_only_VC_category_type_2() 
{
  int i, cure_nr ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 1 ;                 /* FIXED only */
  para_c->nr_special_lanes = 2 ;            /* <--------- */
  para_c->nr_fixed_lanes = 14 ;             /* <--------- */
  set_FIX_ONLY_boundary() ;
  para_c->max_class = 6 ;                   /* <--------- */
  cure_nr = 0 ;
  for(i=0; i<(para_c->max_class+1); i++) {
    set_class_i(i, 2, cure_nr) ;
    cure_nr = 2*(i+1) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_only_VC_category_type_3() 
{
  int i, cure_nr ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 1 ;                 /* FIXED only */
  para_c->nr_special_lanes = 1 ;            /* <--------- */
  para_c->nr_fixed_lanes = 15 ;             /* <--------- */
  set_FIX_ONLY_boundary() ;
  para_c->max_class = 4 ;                   /* <--------- */
  cure_nr = 0 ;
  for(i=0; i<(para_c->max_class+1); i++) {
    set_class_i(i, 3, cure_nr) ;
    cure_nr = 3*(i+1) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_only_VC_category_type_4() 
{
  int i, cure_nr ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 1 ;                 /* FIXED only */
  para_c->nr_special_lanes = 2 ;            /* <--------- */
  para_c->nr_fixed_lanes = 14 ;             /* <--------- */
  set_FIX_ONLY_boundary() ;
  para_c->max_class = 4 ;                   /* <--------- */
  cure_nr = 0 ;
  for(i=0; i<(para_c->max_class); i++) {
    set_class_i(i, 3, cure_nr) ;
    cure_nr = 3*(i+1) ;
  }
  set_class_i(4, 2, 12) ;
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_only_VC_category_type_5() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 1 ;                 /* FIXED only */
  para_c->nr_special_lanes = 1 ;            /* <--------- */
  para_c->nr_fixed_lanes = 15 ;             /* <--------- */
  set_FIX_ONLY_boundary() ;
  para_c->max_class = 3 ;                   /* <--------- */
  set_class_i(0, 4, 0) ;
  set_class_i(1, 4, 4) ;
  set_class_i(2, 4, 8) ;
  set_class_i(3, 3, 12) ;
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_only_VC_category_type_6() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 1 ;                 /* FIXED only */
  para_c->nr_special_lanes = 2 ;            /* <--------- */
  para_c->nr_fixed_lanes = 14 ;             /* <--------- */
  set_FIX_ONLY_boundary() ;
  para_c->max_class = 3 ;                   /* <--------- */
  set_class_i(0, 4, 0) ;
  set_class_i(1, 4, 4) ;
  set_class_i(2, 4, 8) ;
  set_class_i(3, 2, 12) ;
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_only_VC_category_type_7() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 1 ;                 /* FIXED only */
  para_c->nr_special_lanes = 2 ;            /* <--------- */
  para_c->nr_fixed_lanes = 14 ;             /* <--------- */
  set_FIX_ONLY_boundary() ;
  para_c->max_class = 3 ;                   /* <--------- */
  set_class_i(0, 4, 0) ;
  set_class_i(1, 4, 4) ;
  set_class_i(2, 3, 8) ;
  set_class_i(3, 3, 11) ;
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_only_VC_category_type_8() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 1 ;                 /* FIXED only */
  para_c->nr_special_lanes = 1 ;            /* <--------- */
  para_c->nr_fixed_lanes = 15 ;             /* <--------- */
  set_FIX_ONLY_boundary() ;
  para_c->max_class = 5 ;                   /* <--------- */
  set_class_i(0, 4, 0) ;
  set_class_i(1, 4, 4) ;
  set_class_i(2, 3, 8) ;
  set_class_i(3, 2, 11) ;
  set_class_i(4, 1, 13) ;
  set_class_i(5, 1, 14) ;
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_only_VC_category_type_9() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 1 ;                 /* FIXED only */
  para_c->nr_special_lanes = 2 ;            /* <--------- */
  para_c->nr_fixed_lanes = 14 ;             /* <--------- */
  set_FIX_ONLY_boundary() ;
  para_c->max_class = 3 ;                   /* <--------- */
  set_class_i(0, 5, 0) ;
  set_class_i(1, 4, 5) ;
  set_class_i(2, 3, 9) ;
  set_class_i(3, 2, 12) ;
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_only_VC_category_type_10() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 1 ;                 /* FIXED only */
  para_c->nr_special_lanes = 3 ;            /* <--------- */
  para_c->nr_fixed_lanes = 13 ;             /* <--------- */
  set_FIX_ONLY_boundary() ;
  para_c->max_class = 6 ;                   /* <--------- */
  set_class_i(0, 4, 0) ;
  set_class_i(1, 3, 4) ;
  set_class_i(2, 2, 7) ;
  for(i=0; i<5; i++) {
    set_class_i(i+3, 1, i+9) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_FREE_type_1() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 2 ;                 /* FIXED and FREE */
  para_c->nr_special_lanes = 1 ;            /* <--------- */
  para_c->nr_free_lanes = 10 ;              /* <--------- */
  para_c->nr_fixed_lanes = 5 ;              /* <--------- */
  set_FIX_FREE_boundary() ;
  para_c->max_class = 4 ;                   /* <--------- */
  for(i=0; i<5; i++) {
    set_class_i(i, 1, i) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_FREE_type_2() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 2 ;                 /* FIXED and FREE */
  para_c->nr_special_lanes = 2 ;            /* <--------- */
  para_c->nr_free_lanes = 9 ;               /* <--------- */
  para_c->nr_fixed_lanes = 5 ;              /* <--------- */
  set_FIX_FREE_boundary() ;
  para_c->max_class = 4 ;                   /* <--------- */
  for(i=0; i<5; i++) {
    set_class_i(i, 1, i) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_FREE_type_3() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 2 ;                 /* FIXED and FREE */
  para_c->nr_special_lanes = 3 ;            /* <--------- */
  para_c->nr_free_lanes = 8 ;              /* <--------- */
  para_c->nr_fixed_lanes = 5 ;              /* <--------- */
  set_FIX_FREE_boundary() ;
  para_c->max_class = 4 ;                   /* <--------- */
  for(i=0; i<5; i++) {
    set_class_i(i, 1, i) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_FREE_type_4() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 2 ;                 /* FIXED and FREE */
  para_c->nr_special_lanes = 4 ;            /* <--------- */
  para_c->nr_free_lanes = 7 ;              /* <--------- */
  para_c->nr_fixed_lanes = 5 ;              /* <--------- */
  set_FIX_FREE_boundary() ;
  para_c->max_class = 4 ;                   /* <--------- */
  for(i=0; i<5; i++) {
    set_class_i(i, 1, i) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_FREE_type_5() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 2 ;                 /* FIXED and FREE */
  para_c->nr_special_lanes = 1 ;            /* <--------- */
  para_c->nr_free_lanes = 9 ;               /* <--------- */
  para_c->nr_fixed_lanes = 6 ;              /* <--------- */
  set_FIX_FREE_boundary() ;
  para_c->max_class = 5 ;                   /* <--------- */
  for(i=0; i<6; i++) {
    set_class_i(i, 1, i) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_FREE_type_6() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 2 ;                 /* FIXED and FREE */
  para_c->nr_special_lanes = 2 ;            /* <--------- */
  para_c->nr_free_lanes = 8 ;               /* <--------- */
  para_c->nr_fixed_lanes = 6 ;              /* <--------- */
  set_FIX_FREE_boundary() ;
  para_c->max_class = 5 ;                   /* <--------- */
  for(i=0; i<6; i++) {
    set_class_i(i, 1, i) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_FREE_type_7() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 2 ;                 /* FIXED and FREE */
  para_c->nr_special_lanes = 3 ;            /* <--------- */
  para_c->nr_free_lanes = 7 ;               /* <--------- */
  para_c->nr_fixed_lanes = 6 ;              /* <--------- */
  set_FIX_FREE_boundary() ;
  para_c->max_class = 5 ;                   /* <--------- */
  for(i=0; i<6; i++) {
    set_class_i(i, 1, i) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_FREE_type_8() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 2 ;                 /* FIXED and FREE */
  para_c->nr_special_lanes = 4 ;            /* <--------- */
  para_c->nr_free_lanes = 6 ;               /* <--------- */
  para_c->nr_fixed_lanes = 6 ;              /* <--------- */
  set_FIX_FREE_boundary() ;
  para_c->max_class = 5 ;                   /* <--------- */
  for(i=0; i<6; i++) {
    set_class_i(i, 1, i) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_FREE_type_9() 
{
  int i ;
  para->nr_lanes = 16 ;             
  para_c->VC_category = 2 ;                 /* FIXED and FREE */
  para_c->nr_special_lanes = 2 ;            /* <--------- */
  para_c->nr_free_lanes = 10 ;               /* <--------- */
  para_c->nr_fixed_lanes = 4 ;              /* <--------- */
  set_FIX_FREE_boundary() ;
  para_c->max_class = 3 ;                   /* <--------- */
  for(i=0; i<4; i++) {
    set_class_i(i, 1, i) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_FIX_FREE_L32_type_9() 
{
  int i ;
  para->nr_lanes = 32 ;             
  para_c->VC_category = 2 ;                 /* FIXED only */
  para_c->nr_special_lanes = 4 ;            /* <--------- */
  para_c->nr_free_lanes = 18 ;               /* <--------- */
  para_c->nr_fixed_lanes = 10 ;              /* <--------- */
  set_FIX_FREE_boundary() ;
  para_c->max_class = 9 ;                   /* <--------- */
  for(i=0; i<10; i++) {
    set_class_i(i, 1, i) ;
  }
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_DYNAMIC() 
{
  para->nr_lanes = 16 ;             
  para_c->nr_special_lanes = 1 ;            /* <--------- */
  para_c->nr_dynamic_lanes = 15 ;           /* <--------- */
  para_c->max_class = MAXINT-1 ;            /* <--------- */
  set_DYNAMIC_boundary() ;
  setting_VCs() ;
  print_VCs() ;
}
/*---------------------------------------------------------------------------*/
void set_DYNAMIC_type_1() 
{
  para_c->VC_category = DYNAMIC ;           /* DYNAMIC */
  set_DYNAMIC() ;
}
/*---------------------------------------------------------------------------*/
void set_DYNAMIC_with_Throttling_type_1() 
{
  para_c->VC_category = HYBRID ; 
  para_c->throttling_boundary = 1 ;
  set_DYNAMIC() ;
}
/*---------------------------------------------------------------------------*/
void set_DYNAMIC_with_Throttling_type_2() 
{
  para_c->VC_category = HYBRID ;            /* HYBRID */
  para_c->throttling_boundary = 2 ;
  set_DYNAMIC() ;
}
/*---------------------------------------------------------------------------*/
void set_DYNAMIC_with_Throttling_type_3() 
{
  para_c->VC_category = HYBRID ;            /* HYBRID */
  para_c->throttling_boundary = 3 ;
  set_DYNAMIC() ;
}
/*---------------------------------------------------------------------------*/
void set_DYNAMIC_with_Throttling_type_4() 
{
  para_c->VC_category = HYBRID ;            /* HYBRID */
  para_c->throttling_boundary = 4 ;
  set_DYNAMIC() ;
}
/*---------------------------------------------------------------------------*/
void set_DYNAMIC_with_Throttling_type_5() 
{
  para_c->VC_category = HYBRID ;            /* HYBRID */
  para_c->throttling_boundary = 5 ;
  set_DYNAMIC() ;
}
/*---------------------------------------------------------------------------*/
void set_DYNAMIC_with_Throttling_type_6() 
{
  para_c->VC_category = HYBRID ;            /* HYBRID */
  para_c->throttling_boundary = 6 ;
  set_DYNAMIC() ;
}
/*---------------------------------------------------------------------------*/
void L_test_A_S_CSt1() /* EXPERIMENT 011 */
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 1 ;          /* next channel: shortest path only */
  para->chan_sel_priority_1 = 1 ;   /* Free buffer threshold */
  para->chan_sel_priority_2 = 4 ;   /* Lowest Dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_S_CSt2()
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 1 ;          /* next channel: shortest path only */
  para->chan_sel_priority_1 = 2 ;   /* max delta */
  para->chan_sel_priority_2 = 4 ;   /* Lowest Dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_S_CSt3()
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 1 ;          /* next channel: shortest path only */
  para->chan_sel_priority_1 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_2 = 4 ;   /* Lowest Dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_S_CSt4()
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 1 ;          /* next channel: shortest path only */
  para->chan_sel_priority_1 = 1 ;   /* free buffer shreshold */
  para->chan_sel_priority_2 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_3 = 2 ;   /* max delta */
  para->chan_sel_priority_4 = 4 ;   /* lowest dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_S_CSt5() 
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 1 ;          /* next channel: shortest path only */
  para->chan_sel_priority_1 = 1 ;   /* free buffer shreshold */
  para->chan_sel_priority_2 = 2 ;   /* max delta */
  para->chan_sel_priority_3 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_4 = 4 ;   /* lowest dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_S_CSt6() 
{
  para->depth = 1 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 1 ;          /* next channel: shortest path only */
  para->chan_sel_priority_1 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_2 = 1 ;   /* free buffer shreshold */
  para->chan_sel_priority_3 = 2 ;   /* max delta */
  para->chan_sel_priority_4 = 4 ;   /* lowest dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_CSt1()
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 1 ;   /* Free buffer threshold */
  para->chan_sel_priority_2 = 4 ;   /* Lowest Dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_CSt2() /* EXPERIMENT 022 */
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 2 ;   /* max delta */
  para->chan_sel_priority_2 = 4 ;   /* Lowest Dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_CSt3() /* EXPERIMENT 023 */
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_2 = 4 ;   /* Lowest Dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_CSt4()
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 1 ;   /* free buffer shreshold */
  para->chan_sel_priority_2 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_3 = 2 ;   /* max delta */
  para->chan_sel_priority_4 = 4 ;   /* lowest dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_CSt5() 
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 1 ;   /* free buffer shreshold */
  para->chan_sel_priority_2 = 2 ;   /* max delta */
  para->chan_sel_priority_3 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_4 = 4 ;   /* lowest dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_CSt6()
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_2 = 1 ;   /* free buffer shreshold */
  para->chan_sel_priority_3 = 2 ;   /* max delta */
  para->chan_sel_priority_4 = 4 ;   /* lowest dimension */
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_T5_CSt1() 
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 1 ;   /* Free buffer threshold */
  para->chan_sel_priority_2 = 4 ;   /* Lowest Dimension */
  para->timeout_mode = SHORTEST_PATH ;
  para->timeout_count = 5 ;
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_T5_CSt2() 
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 2 ;   /* max delta */
  para->chan_sel_priority_2 = 4 ;   /* Lowest Dimension */
  para->timeout_mode = SHORTEST_PATH ;
  para->timeout_count = 5 ;
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_T5_CSt3() 
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_2 = 4 ;   /* Lowest Dimension */
  para->timeout_mode = SHORTEST_PATH ;
  para->timeout_count = 5 ;
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_T5_CSt4() 
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 1 ;   /* free buffer shreshold */
  para->chan_sel_priority_2 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_3 = 2 ;   /* max delta */
  para->chan_sel_priority_4 = 4 ;   /* lowest dimension */
  para->timeout_mode = SHORTEST_PATH ;
  para->timeout_count = 5 ;
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_T5_CSt5() 
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 1 ;   /* free buffer shreshold */
  para->chan_sel_priority_2 = 2 ;   /* max delta */
  para->chan_sel_priority_3 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_4 = 4 ;   /* lowest dimension */
  para->timeout_mode = SHORTEST_PATH ;
  para->timeout_count = 5 ;
}
/*---------------------------------------------------------------------------*/
void L_test_A_MIS_T5_CSt6() 
{
  para->depth = 2 ;                 /* depth of each virtual channel */
  para->routing_scheme = ADAPTIVE ; /* routing scheme: adaptive */
  para->chan_finding = 2 ;          /* next channel: allowing misrouting */
  para->chan_sel_priority_1 = 3 ;   /* Minimize DR */
  para->chan_sel_priority_2 = 1 ;   /* free buffer shreshold */
  para->chan_sel_priority_3 = 2 ;   /* max delta */
  para->chan_sel_priority_4 = 4 ;   /* lowest dimension */
  para->timeout_mode = SHORTEST_PATH ;
  para->timeout_count = 5 ;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void select_VC_category(int algorithm_type, int VC_type)
{
  if(algorithm_type == STATIC) {
    if(VC_type == 1)  set_FIX_only_VC_category_type_1() ;
    else if(VC_type == 2)  set_FIX_only_VC_category_type_2() ;
    else if(VC_type == 3)  set_FIX_only_VC_category_type_3() ;
    else if(VC_type == 4)  set_FIX_only_VC_category_type_4() ;
    else if(VC_type == 5)  set_FIX_only_VC_category_type_5() ;
    else if(VC_type == 6)  set_FIX_only_VC_category_type_6() ;
    else if(VC_type == 7)  set_FIX_only_VC_category_type_7() ;
    else if(VC_type == 8)  set_FIX_only_VC_category_type_8() ;
    else if(VC_type == 9)  set_FIX_only_VC_category_type_9() ;
  }
  else if(algorithm_type == STATIC_DBP) {
    if(VC_type == 1)  set_FIX_FREE_type_1() ;
    else if(VC_type == 2)  set_FIX_FREE_type_2() ;
    else if(VC_type == 3)  set_FIX_FREE_type_3() ;
    else if(VC_type == 4)  set_FIX_FREE_type_4() ;
    else if(VC_type == 5)  set_FIX_FREE_type_5() ;
    else if(VC_type == 6)  set_FIX_FREE_type_6() ;
    else if(VC_type == 7)  set_FIX_FREE_type_7() ;
    else if(VC_type == 8)  set_FIX_FREE_type_8() ;
    else if(VC_type == 9)  set_FIX_FREE_type_9() ;
  }
  else if(algorithm_type == DYNAMIC) {
    set_DYNAMIC_type_1() ;
  }
  else if(algorithm_type == HYBRID) {
    if(VC_type == 1) set_DYNAMIC_with_Throttling_type_1() ;
    else if(VC_type == 2) set_DYNAMIC_with_Throttling_type_2() ;
    else if(VC_type == 3) set_DYNAMIC_with_Throttling_type_3() ;
    else if(VC_type == 4) set_DYNAMIC_with_Throttling_type_4() ;
    else if(VC_type == 5) set_DYNAMIC_with_Throttling_type_5() ;
    else if(VC_type == 6) set_DYNAMIC_with_Throttling_type_6() ;
  }
  else {error("undefined virtual channel algorithm type\n") ; exit(5) ;}
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void latency_test_nonadaptive_L1_D32()   /* EXPERIMENT 900 */
{
  para->depth = 32 ;                     /* depth of each virtual channel */
  para->routing_scheme = NONADAPTIVE ;   /* routing scheme: nonadaptive */
  para->nr_lanes = 1 ;                   /* number of lanes =1 */
}
/*---------------------------------------------------------------------------*/
void latency_test_nonadaptive_L1_D2()  /* EXPERIMENT 901 */
{
  para->depth = 2 ;                    /* depth of each virtual channel */
  para->routing_scheme = NONADAPTIVE ; /* routing scheme: nonadaptive */
  para->nr_lanes = 1 ;                 /* number of lanes =1 */
}
/*---------------------------------------------------------------------------*/
void latency_test_nonadaptive_L2_D16()  /* EXPERIMENT 902*/
{
  para->depth = 16 ;                    /* depth of each virtual channel */
  para->routing_scheme = NONADAPTIVE ; /* routing scheme: nonadaptive */
  para->nr_lanes = 2 ;                 /* number of lanes =1 */
}
/*---------------------------------------------------------------------------*/
void latency_test_nonadaptive_L4_D8()  /* EXPERIMENT 903 */
{
  para->depth = 8 ;                    /* depth of each virtual channel */
  para->routing_scheme = NONADAPTIVE ; /* routing scheme: nonadaptive */
  para->nr_lanes = 4 ;                 /* number of lanes =1 */
}
/*---------------------------------------------------------------------------*/
void latency_test_nonadaptive_L8_D4()    /* EXPERIMENT 904 */ 
{
  para->depth = 4 ;                    /* depth of each virtual channel */
  para->routing_scheme = NONADAPTIVE ; /* routing scheme: nonadaptive */
  para->nr_lanes = 8 ;                 /* number of lanes =8 */
}
/*---------------------------------------------------------------------------*/
void latency_test_nonadaptive_L16_D2()    /* EXPERIMENT 905 */ 
{
  para->depth = 2 ;                    /* depth of each virtual channel */
  para->routing_scheme = NONADAPTIVE ; /* routing scheme: nonadaptive */
  para->nr_lanes = 16 ;                /* number of lanes =16 */
}
/*---------------------------------------------------------------------------*/
int select_experiment(int experiment_nr, int a_type, int test_category)
{
  int routing_policy ;

  if(test_category == 3 || test_category == 4) { /* fault tolerency test */
    para->test_category = FAULT_TEST ;
    routing_policy = experiment_nr - 1000 ;
    if(routing_policy <200) return(0) ;
  }
  switch(experiment_nr) {
  case 1111:
    L_test_A_S_CSt1() ; select_VC_category(a_type,1); return(1) ;
  case 1112:
    L_test_A_S_CSt2() ; select_VC_category(a_type,1); return(1) ;
  case 1113:
    L_test_A_S_CSt3() ; select_VC_category(a_type,1); return(1) ;
  case 1114:
    L_test_A_S_CSt4() ; select_VC_category(a_type,1); return(1) ;
  case 1115:
    L_test_A_S_CSt5() ; select_VC_category(a_type,1); return(1) ;
  case 1116:
    L_test_A_S_CSt6() ; select_VC_category(a_type,1); return(1) ;

  case 1121:
    L_test_A_S_CSt1() ; select_VC_category(a_type,2); return(1) ;
  case 1122:
    L_test_A_S_CSt2() ; select_VC_category(a_type,2); return(1) ;
  case 1123:
    L_test_A_S_CSt3() ; select_VC_category(a_type,2); return(1) ;
  case 1124:
    L_test_A_S_CSt4() ; select_VC_category(a_type,2); return(1) ;
  case 1125:
    L_test_A_S_CSt5() ; select_VC_category(a_type,2); return(1) ;
  case 1126:
    L_test_A_S_CSt6() ; select_VC_category(a_type,2); return(1) ;

  case 1131:
    L_test_A_S_CSt1() ; select_VC_category(a_type,3); return(1) ;
  case 1132:
    L_test_A_S_CSt2() ; select_VC_category(a_type,3); return(1) ;
  case 1133:
    L_test_A_S_CSt3() ; select_VC_category(a_type,3); return(1) ;
  case 1134:
    L_test_A_S_CSt4() ; select_VC_category(a_type,3); return(1) ;
  case 1135:
    L_test_A_S_CSt5() ; select_VC_category(a_type,3); return(1) ;
  case 1136:
    L_test_A_S_CSt6() ; select_VC_category(a_type,3); return(1) ;

  case 1141:
    L_test_A_S_CSt1() ; select_VC_category(a_type,4); return(1) ;
  case 1142:
    L_test_A_S_CSt2() ; select_VC_category(a_type,4); return(1) ;
  case 1143:
    L_test_A_S_CSt3() ; select_VC_category(a_type,4); return(1) ;
  case 1144:
    L_test_A_S_CSt4() ; select_VC_category(a_type,4); return(1) ;
  case 1145:
    L_test_A_S_CSt5() ; select_VC_category(a_type,4); return(1) ;
  case 1146:
    L_test_A_S_CSt6() ; select_VC_category(a_type,4); return(1) ;

  case 1151:
    L_test_A_S_CSt1() ; select_VC_category(a_type,5); return(1) ;
  case 1152:
    L_test_A_S_CSt2() ; select_VC_category(a_type,5); return(1) ;
  case 1153:
    L_test_A_S_CSt3() ; select_VC_category(a_type,5); return(1) ;
  case 1154:
    L_test_A_S_CSt4() ; select_VC_category(a_type,5); return(1) ;
  case 1155:
    L_test_A_S_CSt5() ; select_VC_category(a_type,5); return(1) ;
  case 1156:
    L_test_A_S_CSt6() ; select_VC_category(a_type,5); return(1) ;

  case 1161:
    L_test_A_S_CSt1() ; select_VC_category(a_type,6); return(1) ;
  case 1162:
    L_test_A_S_CSt2() ; select_VC_category(a_type,6); return(1) ;
  case 1163:
    L_test_A_S_CSt3() ; select_VC_category(a_type,6); return(1) ;
  case 1164:
    L_test_A_S_CSt4() ; select_VC_category(a_type,6); return(1) ;
  case 1165:
    L_test_A_S_CSt5() ; select_VC_category(a_type,6); return(1) ;
  case 1166:
    L_test_A_S_CSt6() ; select_VC_category(a_type,6); return(1) ;
 
  case 1211:
    L_test_A_MIS_CSt1() ; select_VC_category(a_type,1); return(1) ;
  case 1212:
    L_test_A_MIS_CSt2() ; select_VC_category(a_type,1); return(1) ;
  case 1213:
    L_test_A_MIS_CSt3() ; select_VC_category(a_type,1); return(1) ;
  case 1214:
    L_test_A_MIS_CSt4() ; select_VC_category(a_type,1); return(1) ;
  case 1215:
    L_test_A_MIS_CSt5() ; select_VC_category(a_type,1); return(1) ;
  case 1216:
    L_test_A_MIS_CSt6() ; select_VC_category(a_type,1); return(1) ;

  case 1221:
    L_test_A_MIS_CSt1() ; select_VC_category(a_type,2); return(1) ;
  case 1222:
    L_test_A_MIS_CSt2() ; select_VC_category(a_type,2); return(1) ;
  case 1223:
    L_test_A_MIS_CSt3() ; select_VC_category(a_type,2); return(1) ;
  case 1224:
    L_test_A_MIS_CSt4() ; select_VC_category(a_type,2); return(1) ;
  case 1225:
    L_test_A_MIS_CSt5() ; select_VC_category(a_type,2); return(1) ;
  case 1226:
    L_test_A_MIS_CSt6() ; select_VC_category(a_type,2); return(1) ;

  case 1231:
    L_test_A_MIS_CSt1() ; select_VC_category(a_type,3); return(1) ;
  case 1232:
    L_test_A_MIS_CSt2() ; select_VC_category(a_type,3); return(1) ;
  case 1233:
    L_test_A_MIS_CSt3() ; select_VC_category(a_type,3); return(1) ;
  case 1234:
    L_test_A_MIS_CSt4() ; select_VC_category(a_type,3); return(1) ;
  case 1235:
    L_test_A_MIS_CSt5() ; select_VC_category(a_type,3); return(1) ;
  case 1236:
    L_test_A_MIS_CSt6() ; select_VC_category(a_type,3); return(1) ;

  case 1241:
    L_test_A_MIS_CSt1() ; select_VC_category(a_type,4); return(1) ;
  case 1242:
    L_test_A_MIS_CSt2() ; select_VC_category(a_type,4); return(1) ;
  case 1243:
    L_test_A_MIS_CSt3() ; select_VC_category(a_type,4); return(1) ;
  case 1244:
    L_test_A_MIS_CSt4() ; select_VC_category(a_type,4); return(1) ;
  case 1245:
    L_test_A_MIS_CSt5() ; select_VC_category(a_type,4); return(1) ;
  case 1246:
    L_test_A_MIS_CSt6() ; select_VC_category(a_type,4); return(1) ;

  case 1251:
    L_test_A_MIS_CSt1() ; select_VC_category(a_type,5); return(1) ;
  case 1252:
    L_test_A_MIS_CSt2() ; select_VC_category(a_type,5); return(1) ;
  case 1253:
    L_test_A_MIS_CSt3() ; select_VC_category(a_type,5); return(1) ;
  case 1254:
    L_test_A_MIS_CSt4() ; select_VC_category(a_type,5); return(1) ;
  case 1255:
    L_test_A_MIS_CSt5() ; select_VC_category(a_type,5); return(1) ;
  case 1256:
    L_test_A_MIS_CSt6() ; select_VC_category(a_type,5); return(1) ;

  case 1261:
    L_test_A_MIS_CSt1() ; select_VC_category(a_type,6); return(1) ;
  case 1262:
    L_test_A_MIS_CSt2() ; select_VC_category(a_type,6); return(1) ;
  case 1263:
    L_test_A_MIS_CSt3() ; select_VC_category(a_type,6); return(1) ;
  case 1264:
    L_test_A_MIS_CSt4() ; select_VC_category(a_type,6); return(1) ;
  case 1265:
    L_test_A_MIS_CSt5() ; select_VC_category(a_type,6); return(1) ;
  case 1266:
    L_test_A_MIS_CSt6() ; select_VC_category(a_type,6); return(1) ;
 
  case 1311:
    L_test_A_MIS_T5_CSt1() ; select_VC_category(a_type,1); return(1) ;
  case 1312:
    L_test_A_MIS_T5_CSt2() ; select_VC_category(a_type,1); return(1) ;
  case 1313:
    L_test_A_MIS_T5_CSt3() ; select_VC_category(a_type,1); return(1) ;
  case 1314:
    L_test_A_MIS_T5_CSt4() ; select_VC_category(a_type,1); return(1) ;
  case 1315:
    L_test_A_MIS_T5_CSt5() ; select_VC_category(a_type,1); return(1) ;
  case 1316:
    L_test_A_MIS_T5_CSt6() ; select_VC_category(a_type,1); return(1) ;

  case 1321:
    L_test_A_MIS_T5_CSt1() ; select_VC_category(a_type,2); return(1) ;
  case 1322:
    L_test_A_MIS_T5_CSt2() ; select_VC_category(a_type,2); return(1) ;
  case 1323:
    L_test_A_MIS_T5_CSt3() ; select_VC_category(a_type,2); return(1) ;
  case 1324:
    L_test_A_MIS_T5_CSt4() ; select_VC_category(a_type,2); return(1) ;
  case 1325:
    L_test_A_MIS_T5_CSt5() ; select_VC_category(a_type,2); return(1) ;
  case 1326:
    L_test_A_MIS_T5_CSt6() ; select_VC_category(a_type,2); return(1) ;

  case 1331:
    L_test_A_MIS_T5_CSt1() ; select_VC_category(a_type,3); return(1) ;
  case 1332:
    L_test_A_MIS_T5_CSt1() ; select_VC_category(a_type,3); return(1) ;
  case 1333:
    L_test_A_MIS_T5_CSt3() ; select_VC_category(a_type,3); return(1) ;
  case 1334:
    L_test_A_MIS_T5_CSt4() ; select_VC_category(a_type,3); return(1) ;
  case 1335:
    L_test_A_MIS_T5_CSt5() ; select_VC_category(a_type,3); return(1) ;
  case 1336:
    L_test_A_MIS_T5_CSt6() ; select_VC_category(a_type,3); return(1) ;

  case 1341:
    L_test_A_MIS_T5_CSt1() ; select_VC_category(a_type,4); return(1) ;
  case 1342:
    L_test_A_MIS_T5_CSt2() ; select_VC_category(a_type,4); return(1) ;
  case 1343:
    L_test_A_MIS_T5_CSt3() ; select_VC_category(a_type,4); return(1) ;
  case 1344:
    L_test_A_MIS_T5_CSt4() ; select_VC_category(a_type,4); return(1) ;
  case 1345:
    L_test_A_MIS_T5_CSt5() ; select_VC_category(a_type,4); return(1) ;
  case 1346:
    L_test_A_MIS_T5_CSt6() ; select_VC_category(a_type,4); return(1) ;

  case 1351:
    L_test_A_MIS_T5_CSt1() ; select_VC_category(a_type,5); return(1) ;
  case 1352:
    L_test_A_MIS_T5_CSt2() ; select_VC_category(a_type,5); return(1) ;
  case 1353:
    L_test_A_MIS_T5_CSt3() ; select_VC_category(a_type,5); return(1) ;
  case 1354:
    L_test_A_MIS_T5_CSt4() ; select_VC_category(a_type,5); return(1) ;
  case 1355:
    L_test_A_MIS_T5_CSt5() ; select_VC_category(a_type,5); return(1) ;
  case 1356:
    L_test_A_MIS_T5_CSt6() ; select_VC_category(a_type,5); return(1) ;

  case 1361:
    L_test_A_MIS_T5_CSt1() ; select_VC_category(a_type,6); return(1) ;
  case 1362:
    L_test_A_MIS_T5_CSt2() ; select_VC_category(a_type,6); return(1) ;
  case 1363:
    L_test_A_MIS_T5_CSt3() ; select_VC_category(a_type,6); return(1) ;
  case 1364:
    L_test_A_MIS_T5_CSt4() ; select_VC_category(a_type,6); return(1) ;
  case 1365:
    L_test_A_MIS_T5_CSt5() ; select_VC_category(a_type,6); return(1) ;
  case 1366:
    L_test_A_MIS_T5_CSt6() ; select_VC_category(a_type,6); return(1) ;
 
  case 900:
    latency_test_nonadaptive_L1_D32() ; return(1) ;
  case 901:
    latency_test_nonadaptive_L1_D2()  ; return(1) ;
  case 902:
    latency_test_nonadaptive_L2_D16() ; return(1) ;
  case 903:
    latency_test_nonadaptive_L4_D8()  ; return(1) ;
  case 904:
    latency_test_nonadaptive_L8_D4()  ; return(1) ;
  case 905:
    latency_test_nonadaptive_L16_D2() ; return(1) ;

  default:
    printf("Unknown experiment\n") ;
    return(0) ;
  }
}
/*---------------------------------------------------------------------------*/
