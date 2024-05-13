import click
import numpy as np

def to_int(word: str):
    try:
        return int(word)
    except:
        return 0

@click.command()
@click.argument("logfile", type=click.Path("r"))
def count_time(logfile):
    times = []
    with open(logfile, "r") as f:
        for line in f.readlines():
            words = line.split(' ')
            if len(words) == 0:
                continue
            time = to_int(words[-1])
            if time > 0:
                times.append(time)
    time_np = np.array(times)
    fifty_percentile = np.percentile(time_np, 50)
    ninety_percentile = np.percentile(time_np, 90)
    ninetynine_percentile = np.percentile(time_np, 99)

    print(f"50%ile: {fifty_percentile} ms")
    print(f"90%ile: {ninety_percentile} ms")
    print(f"99%ile: {ninetynine_percentile} ms")

if __name__ == "__main__":
    count_time()
