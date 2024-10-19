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

relative_df = df.copy()
for i in range(len(df)):

    origin_real_time = df[(df['name'] == 'frxml') & (df['count'] == df.at[i, 'count'])]['real_time'].values[0]
    relative_df.at[i, 'real_time'] /= origin_real_time

    origin_max_bytes_used = df[(df['name'] == 'frxml') & (df['count'] == df.at[i, 'count'])]['max_bytes_used'].values[0]
    relative_df.at[i, 'max_bytes_used'] /= origin_max_bytes_used

df = relative_df

df['count'] = df['count'].astype(float) / 1024


perf_pivot = df.pivot_table(index='count', columns='name', values='real_time')
mem_pivot = df.pivot_table(index='count', columns='name', values='max_bytes_used')
perf_pivot.reset_index(inplace=True)
mem_pivot.reset_index(inplace=True)




fig, axes = plt.subplots(nrows=1, ncols=2)

plt.sca(axes[0])

plt.xlabel("XML Size (KiB)")
plt.ylabel("Elapsed (relative to frxml)")
plt.xlim([perf_pivot['count'].min(), perf_pivot['count'].max()])

for lib in ['pugixml', 'tinyxml2', 'rapidxml', 'libxml2', 'frxml']:
    plt.plot(perf_pivot['count'], perf_pivot[lib], label=lib)

plt.xticks(rotation=45, ha='right')
plt.semilogy()
plt.yticks([ 0.5, 1, 2, 3, 4, 5 ], [ '0.5', '1.0', '2.0', '3.0', '4.0', '5.0' ])
plt.title("Parsing Time (log scale)")
plt.legend(title="Library")


plt.sca(axes[1])

axes[1].set_aspect('auto')
plt.xlabel("XML Size (KiB)")
plt.ylabel("Usage (relative to frxml)")
plt.xlim([mem_pivot['count'].min(), mem_pivot['count'].max()])
#plt.ylim([df['max_bytes_used'].min(), 3])

for lib in ['pugixml', 'tinyxml2', 'rapidxml', 'libxml2', 'frxml']:
    plt.plot(mem_pivot['count'], mem_pivot[lib], label=lib)

plt.xticks(rotation=45, ha='right')
plt.yticks([ 0.5, 1, 2, 3, 4, 5 ], [ '0.5', '1.0', '2.0', '3.0', '4.0', '5.0' ])
plt.title("Allocated Memory (linear scale)")
plt.legend(title="Library")

fig.tight_layout()
plt.show()
