# SwitchFabric

 The simulated switch works in a time slot manner. The whole system progresses one slot time (i.e. ATM cell) at a time. For offered simulation load in every time slot system checks for new arrivals of cells to the inputs and decides which cells will be forwarded from inputs to outputs trough switching fabric. Simulator should have a modular architecture. Every module has different functionality. The functionality modules are: traffic model, queuing policy, switch fabric, and scheduling algorithm. It provide overall statistics (i.e. average latency per cell, throughput) after simulation.
 
 By default, the input and output queues are first-come-first-served (FCFS). Note virtual-output-queues (VOQs) reside at input ports, meaning there’re N VOQs in each input for NxN switch. The simulator should be able to support queues of arbitrary size; however the user can set the maximum queue capacity via configuration file, or command line argument if needed.
 
 The switching fabric is used for the input and output ports interconnection. The crossbar is a switching fabric which is modeled in this project because it is one of the most used fabrics for high performance switches. By default the crossbar fabric may setup the input and output interconnection once per time slot (speedup = 1).
 
