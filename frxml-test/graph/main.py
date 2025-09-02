import json
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

def parse_benchmark_results(filepath='results.json'):
    with open(filepath, 'r') as f:
        data = json.load(f)

    parsed_data = []
    for run in data['benchmarks']:
        name_parts = run['name'].split('/')
        if len(name_parts) < 2:
            continue

        library_name = name_parts[0]
        try:
            file_size = int(run['label'])
        except (KeyError, ValueError):
            print(f"Warning: Could not parse file size from label for {run['name']}")
            continue

        parsed_data.append({
            'library': library_name,
            'file_size_bytes': file_size,
            'cpu_time_ns': run['cpu_time'],
            'bytes_per_second': run.get('bytes_per_second', 0)
        })

    return pd.DataFrame(parsed_data)

def plot_graphs(df):
    sns.set_theme(style="whitegrid")

    # 1. File size vs CPU time
    plt.figure(figsize=(12, 6))
    sns.lineplot(data=df, x='file_size_bytes', y='cpu_time_ns', hue='library', marker='o')

    plt.title('Execution Time vs. File Size', fontsize=16)
    plt.xlabel('File Size (Bytes)', fontsize=12)
    plt.ylabel('CPU Time (nanoseconds)', fontsize=12)
    plt.xscale('log')
    plt.yscale('log')
    plt.legend(title='Library')
    plt.grid(True, which="both", ls="--")

    plt.savefig('time_vs_filesize.png', dpi=300)
    print("Saved 'time_vs_filesize.png'")

    # 2. File size vs Throughput
    # Convert to GiB/s
    df['throughput_gib_s'] = df['bytes_per_second'] / (1024**3)

    plt.figure(figsize=(12, 6))
    # Excluding `strlen`
    plot_df = df[df['library'] != 'strlen']
    sns.lineplot(data=plot_df, x='file_size_bytes', y='throughput_gib_s', hue='library', marker='o')

    plt.title('Throughput vs. File Size', fontsize=16)
    plt.xlabel('File Size (Bytes)', fontsize=12)
    plt.ylabel('Throughput (GiB/s)', fontsize=12)
    plt.xscale('log')
    plt.legend(title='Library')
    plt.grid(True, which="both", ls="--")

    plt.savefig('throughput_vs_filesize.png', dpi=300)
    print("Saved 'throughput_vs_filesize.png'")

    plt.show()


if __name__ == '__main__':
    try:
        benchmark_df = parse_benchmark_results()
        plot_graphs(benchmark_df)
    except FileNotFoundError:
        print("Error: 'results.json' not found. Please run the benchmark first with:")
        print("your_executable --benchmark_format=json > results.json")
