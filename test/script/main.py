import json
import sys
import matplotlib.pyplot as plt
import pandas as pd

#js = json.loads(sys.stdin.read())

with open("dump.json", "r") as fd:
    js = json.load(fd)
df = pd.DataFrame(js["benchmarks"])

for i in range(len(df["name"])):
    terms = df.at[i, "name"].split("/")
    df.at[i, "name"] = terms[0].split("_")[1]
    df.at[i, "count"] = terms[1]

df.sort_values(by="count", inplace=True)

df = df[['name', 'count', 'real_time', 'max_bytes_used']]

df['count'] = df['count'].astype(float) / 1024
df['real_time'] = df['real_time'].astype(float) / 1000000
df['max_bytes_used'] = df['max_bytes_used'].astype(float) / 1024

perf_pivot = df.pivot_table(index='count', columns='name', values='real_time')
mem_pivot = df.pivot_table(index='count', columns='name', values='max_bytes_used')
perf_pivot.reset_index(inplace=True)
mem_pivot.reset_index(inplace=True)




fig, axes = plt.subplots(nrows=1, ncols=2)

plt.sca(axes[0])

plt.xlabel("XML Size (KiB)")
plt.ylabel("Elapsed (ms)")

for lib in ['pugixml', 'tinyxml2', 'rapidxml', 'frxml']:
    plt.plot(perf_pivot['count'], perf_pivot[lib], label=lib)

plt.xticks(rotation=45, ha='right')
plt.title("Parsing Time")
plt.legend(title="Library")


plt.sca(axes[1])

plt.xlabel("XML Size (KiB)")
plt.ylabel("Usage (KiB)")

for lib in ['pugixml', 'tinyxml2', 'rapidxml', 'frxml']:
    plt.plot(mem_pivot['count'], mem_pivot[lib], label=lib)

plt.xticks(rotation=45, ha='right')
plt.title("Allocated Memory")
plt.legend(title="Library")

fig.tight_layout()
plt.show()
