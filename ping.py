from scapy.all import *
import time
from scapy.layers.inet import IP, ICMP

# Set the IP address or hostname to ping
host = "192.168.64.6"

with open("ping_result_py.txt", "w") as f:
    count = 1; 
    # Loop indefinitely
    while True:
        # Send the ICMP echo request packet and capture the reply
        start_time = time.time()
        reply = sr1(IP(dst=host)/ICMP(), timeout=2, verbose=False)
        end_time = time.time()

        # Calculate the RTT (in milliseconds)
        rtt = (end_time - start_time) * 1000

        # Write the RTT to a file
        f.write(f"Round-Trip Time: {rtt:.2f} ms\n")
        f.flush() 

        # Wait for 5 seconds before sending the next ping
        time.sleep(5)
        count+=1 
    
    