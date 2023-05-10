import re
import matplotlib.pyplot as plt
import numpy as np


# read the data from the file
with open("syn_results_c.txt", "r") as f:
    data = f.readlines()

# parse the data and calculate the time needed to send each packet
times = []

for i in range(0, len(data), 2):
    match_num = re.search(r'^\d+$', data[i].strip())
    match_time = re.search(r'^\d+$', data[i+1].strip())
    if match_num and match_time:
        time_str = match_time.group()
        
        time_sec = float(time_str) / 1000000  # convert from microseconds to seconds
        times.append(time_sec)

    

# for line in data:
#     match_time = re.search(r'^\d+\.\d+$', line.strip())
#     if match_time:
#         time_str = match_time.group()
#         time_sec = float(time_str) / 1000 # convert from microseconds to seconds
#         times.append(time_sec)



# plot the graph
x = np.array(times)
y = np.array(range(1, len(times)+1))
mask = x <= 3
plt.plot(x[mask], y[mask])
plt.xlabel("Time needed to send a packet (seconds)", fontsize=12)
plt.ylabel("Number of packets sent", fontsize=12)
plt.title("Number of packets sent vs Time needed to send a packet")


plt.savefig("Syn_pkts_c.png")

print("Plot saved to Syn_pkts_c.png")