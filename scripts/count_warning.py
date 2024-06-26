import click

@click.command()
@click.argument("logfile", type=click.Path("r"))
def count_warning(logfile):
    warnings = {}
    with open(logfile, "r") as f:
        for line in f.readlines():
            if all(keyword in line for keyword in ['ERROR', '---']):
                warn_name = line.split('---')[1]
                if warn_name not in warnings:
                    warnings[warn_name] = 0
                warnings[warn_name] += 1

    print(warnings)
    print(sum(warnings.values()))


if __name__ == "__main__":
    count_warning()
