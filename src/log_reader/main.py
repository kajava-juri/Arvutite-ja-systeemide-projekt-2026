import argparse
import csv
import matplotlib.pyplot as plt


def parse_float(value: str):
    """Return float, or None for empty cells."""
    return float(value) if value.strip() else None


def read_csv(path: str):
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            yield {
                "timestamp": row["timestamp"],
                "elapsed_s": parse_float(row["elapsed_s"]),
                "source": row["source"],
                "setpoint": parse_float(row["setpoint"]),
                "pressure": parse_float(row["pressure"]),
            }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("path", help="CSV file to read")
    args = parser.parse_args()

    rows = read_csv(args.path)  # create once

    # Advance until the first row with a pressure value
    first = None
    for row in rows:
        if row["pressure"] is not None:
            first = row
            break

    if first is None:
        print("No pressure values in file")
        return

    print(f"First row with pressure: {first}")

    # Continues from the row after `first`
    pressures = []
    elapsed_s = []
    setpoints = []
    for row in rows:
        pressures.append(row["pressure"])
        elapsed_s.append(row["elapsed_s"])
        setpoints.append(row["setpoint"])

    plt.plot(elapsed_s, pressures, label="Pressure", color="blue")
    plt.plot(elapsed_s, setpoints, label="Setpoint", color="orange", linestyle="--")
    plt.xlabel("Elapsed Time (s)")
    plt.ylabel("Pressure")
    plt.title("Pressure vs Elapsed Time")
    plt.grid()
    plt.show()


if __name__ == "__main__":
    main()