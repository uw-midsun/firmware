import subprocess
import socket
import _thread
import time

# return the sun_path of the most recently started program
def get_sun_path(prog, port):

    latest_path = ""
    res = subprocess.run(["pgrep", "-l", prog], stdout = subprocess.PIPE)
    # decode pgrep output from bytes and get the pids and program names
    res_arr = res.stdout.decode("utf-8").split("\n")

    for proc in res_arr:
        if proc:
            try:
                tmp_arr = proc.split(" ")
                if tmp_arr[1] == prog:
                    latest_path = "%s/%s/%s" % (tmp_arr[0],prog,port)
            except:
                continue

    return latest_path

class Socket:

    def __init__(self, prog, port):
        self.m_prog = prog
        self.m_port = port
        self.m_connected = False
        self.m_sock = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
        self.m_sun_path = ""
        self.m_partner = None

    # add the connected socket
    def add_partner(self, sock):
        self.m_partner = sock

    # connect to the c server socket
    def connect(self):
        if not self.m_connected:
            sun_path = get_sun_path(self.m_prog, self.m_port)
            try:
                self.m_sock.connect("\0"+sun_path)
                self.m_connected = True
                self.m_sun_path = sun_path
                self.setup_sock()
                return True
            except:
                return False
        return True

    def setup_sock(self):
        try:
            _thread.start_new_thread(self.read, ())
        except:
            print("Error: unable to start read thread for %s." % (self.m_prog))

    # read data from the c server socket and send to partner
    def read(self):
        while (self.m_connected):
            data = self.m_sock.recv(25)
            if len(data) > 0:
                self.m_partner.write(data)

    # write tx_data to the server socket
    def write(self, tx_data):
        if self.m_connected:
            self.m_sock.sendall(tx_data)
        else:
            print("Socket not connected to %s, could not send data." % (self.m_prog))

    # getters
    def get_prog(self):
        return self.m_prog

    def get_port(self):
        return self.m_port

    def get_path(self):
        return self.m_sun_path

    def is_connected(self):
        return self.m_connected

class Connection:

    def __init__(self, prog1, port1, prog2, port2):
        # create socket objects and add the prefix "uart"
        port1 = "uart" + port1
        port2 = "uart" + port2
        self.m_sock1 = Socket(prog1, port1)
        self.m_sock2 = Socket(prog2, port2)

        # connect the sockets
        self.m_sock1.add_partner(self.m_sock2)
        self.m_sock2.add_partner(self.m_sock1)

        self.m_first_attempt = False
        self.m_num_connected = 0

    # attempt to connect to the unix domain server sockets
    def connect(self):
        if not self.m_sock1.is_connected():
            if self.m_sock1.connect():
                self.m_num_connected += 1
                print("%s connected to %s!" % (self.m_sock1.get_prog(), self.m_sock1.get_path()))
            else:
                if not self.m_first_attempt:
                    print("%s did not connect." % (self.m_sock1.get_prog()))

        if not self.m_sock2.is_connected():
            if self.m_sock2.connect():
                self.m_num_connected += 1
                print("%s connected to %s!" % (self.m_sock2.get_prog(), self.m_sock2.get_path()))
            else:
                if not self.m_first_attempt:
                    print("%s did not connect." % (self.m_sock2.get_prog()))

        # don't spam did not connect messages
        self.m_first_attempt = True

        return self.m_num_connected


#####DECLARE CONNECTIONS BELOW#####

conn1 = Connection("uart", "1", "uart2", "1")

conns = [conn1]
#####DECLARE CONNECTIONS ABOVE#####

for conn in conns:
    conn.connect()

done = False    # fix this later

while not done:
    # keep this program alive so that it can handle communication
    time.sleep(1)
