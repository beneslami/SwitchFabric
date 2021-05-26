import time
import pprint

# get a list of input obejcts and allocate them to different subques.
# if a object want to route to a 
# packet = {
# 	'timestamp' : xxxxx,
# 	'outport' : xxxx,
# 	'data' : xxxxxxxxxxx,
# 	'source' : xxxxx
# }

pp = pprint.PrettyPrinter(indent=4)


class Queue:

    def __init__(self, lst=[]):
        self.queue = []
        self.out = 0

    def get_len(self):
        return len(self.queue)

    def insert(self, obj):
        self.queue.append(obj)

    def process(self):
        try:
            data = self.queue[0]
            del self.queue[0]
            return data
        except:
            return None

    def debug(self):
        return self.queue


class SuperQueue:

    def __init__(self, default_number_queue=4):
        self.input_streams = {}
        self.number_of_queues = 0
        for i in range(0, default_number_queue):
            self.input_streams[i] = {
                'outport': i,
                'queue': Queue(default_number_queue)
            }
            self.number_of_queues = default_number_queue

    def pop_from_queue(self, source, dest):
        return self.input_streams[int(source)].process()

    def insert_data_in_queues(self, data_list):
        for packet in data_list:
            Stream = None
            Stream = self.input_streams[packet['outport']]
            Stream['queue'].insert(packet)
            self.input_streams[packet['outport']] = Stream

    def debug(self):
        for i in self.input_streams:
            print(self.input_streams[i]['queue'].debug())


class SuperMultiQueue:

    def __init__(self, default_number_queue=4):
        self.input_ports = {}
        self.number_of_queues = default_number_queue

        for i in range(0, default_number_queue):
            self.input_ports[i] = {
                'inport': i,
                'queue': SuperQueue(default_number_queue)
            }

    def debug(self):
        for i in self.input_ports:
            print("\t\t", i)
            self.input_ports[i]['queue'].debug()

    def insert_data_in_queues(self, data_list):
        packets_source = {}

        for packet in data_list:
            src = int(packet['source'])
            try:
                packets_source[src].append(packet)
            except Exception as e:
                packets_source[src] = []
                packets_source[src].append(packet)

        for keys in packets_source:
            queue1 = None
            queue1 = self.input_ports[keys]['queue']
            queue1.insert_data_in_queues(packets_source[keys])
            self.input_ports[keys]['queue'] = queue1

    def pop_from_queue(self, card):
        temp_super = self.input_ports[card[0]]
        temp_queue = temp_super['queue'].input_streams[card[1]]['queue']
        data = temp_queue.process()
        temp_super['queue'].input_streams[card[1]]['queue'] = temp_queue
        self.input_ports[card[0]] = temp_super
        return data

    def get_data_stream(self):
        data_list = []
        for key in self.input_ports:
            queue = self.input_ports[key]['queue']
            for key_next in queue.input_streams:
                stream = queue.input_streams[key_next]['queue']
                while True:
                    pop = stream.process()
                    if pop is None:
                        break
                    data_list.append(pop)
        return data_list

    def generate_input_status(self):
        data_list = {}
        for key in self.input_ports:
            data_list[key] = {}
            queue = self.input_ports[key]['queue']

            for key_next in queue.input_streams:
                data_list[key][key_next] = 0
                stream = queue.input_streams[key_next]['queue']
                data_list[key][key_next] = stream.get_len()
        return data_list
