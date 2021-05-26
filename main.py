from Fabric_Class import *
import pprint
import matplotlib.pyplot as plt

pp = pprint.PrettyPrinter(indent=4)

NUMBER_OF_ACTIVE_QUEUES = 20
NUMBER_OF_PACKETS = 50000
SCHEDULER_TYPE = 1  # 0 -> RR , 1-> Islip

NUMBER_OF_ACTIVE_QUEUES = NUMBER_OF_ACTIVE_QUEUES + 1


def Main(data):
    if SCHEDULER_TYPE == 0:
        S_1 = RoundRobinScheduler(NUMBER_OF_ACTIVE_QUEUES, data)
        while S_1.generate_sequences():
            pass
        return S_1.Get_Output()
    elif SCHEDULER_TYPE == 1:
        S_2 = iSlipScheduler(NUMBER_OF_ACTIVE_QUEUES, data)
        while S_2.generate_sequences():
            pass
        return S_2.Get_Output()


def generate_uniform_traffic(number_of_cards, packets=1):
    data_list = []
    for m in range(packets):
        for i in range(0, number_of_cards):
            for j in range(0, number_of_cards):
                obj = {'timestamp': time.time(), 'outport': i, 'data': "this is my data", 'source': j}
                data_list.append(obj)
    return data_list


def generate_bursty_traffic(number_of_cards, number_of_active_outputs, packets=1):
    data_list = []
    for m in range(packets):
        for j in range(0, number_of_cards):
            for i in range(0, number_of_active_outputs):
                obj = {'timestamp': time.time(), 'outport': i, 'data': "this is my data", 'source': j}
                data_list.append(obj)
    return data_list


def get_average_time(data_list):
    number = 0.0
    for obj in data_list:
        number = number + obj['time_out'] - obj['timestamp']
    return number, len(data_list)


def run_analysis(multiplier, type_in=1):
    if type_in == 0:
        input_data = generate_uniform_traffic(5, multiplier)
    else:
        input_data = generate_bursty_traffic(5, 2, multiplier)  # sources are bursty
    data_list = Main(input_data)
    sum_of_diff, number = get_average_time(data_list)
    return (sum_of_diff / number), number


def run_analysis_robin():
    SCHEDULER_TYPE = 0
    pairs = []
    plt1 = plt
    for multiplier in range(1, 75):
        avg, number = run_analysis(multiplier, 1)
        print(multiplier, avg, number)
        pairs.append((number, avg))

    plt1.plot([p[0] for p in pairs], [p[1] for p in pairs], 'b--', label="Round Robin")

    pairs = []
    for multiplier in range(1, 75):
        avg, number = run_analysis(multiplier, 0)
        print(multiplier, avg, number)
        pairs.append((number, avg))

    plt.plot([p[0] for p in pairs], [p[1] for p in pairs], 'g--', label="Round Robin")
    return plt, plt1


def run_analysis_islip():
    SCHEDULER_TYPE = 1
    pairs = []
    plt1 = plt
    for multiplier in range(1, 75):
        avg, number = run_analysis(multiplier, 1)
        print(multiplier, avg, number)
        pairs.append((number, avg))

    plt1.plot([p[0] for p in pairs], [p[1] for p in pairs], 'r--', label="iSLIP")

    pairs = []
    for multiplier in range(1, 75):
        avg, number = run_analysis(multiplier, 0)
        print(multiplier, avg, number)
        pairs.append((number, avg))

    plt.plot([p[0] for p in pairs], [p[1] for p in pairs], 'b--', label="iSLIP")
    return plt, plt1


def make_plots():
    plt1, plt2 = run_analysis_robin()
    plt3, plt4 = run_analysis_islip()
    plt1.legend(loc='best')
    plt2.legend(loc='best')
    plt3.legend(loc='best')
    plt4.legend(loc='best')
    plt1.show()
    plt2.show()
    plt3.show()
    plt4.show()


def Main_Test(data):
    S = RoundRobinScheduler(NUMBER_OF_ACTIVE_QUEUES, data)
    while S.generate_sequences():
        print(S.Status(), "\n")
    print(S.Status())


def evaluate_basic_functioning():
    data_list = [{'timestamp': time.time(), 'outport': 0, 'data': "this is my data", 'source': 0},
                 {'timestamp': time.time(), 'outport': 0, 'data': "this is my data", 'source': 1},
                 {'timestamp': time.time(), 'outport': 0, 'data': "this is my data", 'source': 2},
                 {'timestamp': time.time(), 'outport': 0, 'data': "this is my data", 'source': 3},
                 {'timestamp': time.time(), 'outport': 1, 'data': "this is my data", 'source': 0},
                 {'timestamp': time.time(), 'outport': 1, 'data': "this is my data", 'source': 1},
                 {'timestamp': time.time(), 'outport': 1, 'data': "this is my data", 'source': 2},
                 {'timestamp': time.time(), 'outport': 4, 'data': "this is my data", 'source': 4},
                 {'timestamp': time.time(), 'outport': 2, 'data': "this is my data", 'source': 0},
                 {'timestamp': time.time(), 'outport': 2, 'data': "this is my data", 'source': 1},
                 {'timestamp': time.time(), 'outport': 2, 'data': "this is my data", 'source': 2},
                 {'timestamp': time.time(), 'outport': 2, 'data': "this is my data", 'source': 3},
                 {'timestamp': time.time(), 'outport': 3, 'data': "this is my data", 'source': 0},
                 {'timestamp': time.time(), 'outport': 3, 'data': "this is my data", 'source': 1},
                 {'timestamp': time.time(), 'outport': 3, 'data': "this is my data", 'source': 2},
                 {'timestamp': time.time(), 'outport': 3, 'data': "this is my data", 'source': 3},
                 {'timestamp': time.time(), 'outport': 1, 'data': "this is my data", 'source': 1},
                 {'timestamp': time.time(), 'outport': 1, 'data': "this is my data", 'source': 0},
                 {'timestamp': time.time(), 'outport': 0, 'data': "this is my data", 'source': 0},
                 {'timestamp': time.time(), 'outport': 0, 'data': "this is my data", 'source': 1}]

    Main_Test(data_list)


evaluate_basic_functioning()
make_plots()
