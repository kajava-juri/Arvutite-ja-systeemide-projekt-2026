""" WRITTEN BY CLAUDE 2026-11-05

CSV data logger for rtu_server register updates.

Writes one row every time a watched register (by default ``setpoint`` and
``pressure``) arrives. Each row carries the latest known value of every
watched register, so the file can be loaded straight into pandas/Excel
without pivoting.

Example output::

    timestamp,elapsed_s,source,setpoint,pressure
    2026-11-05T12:40:01.123456,0.000,setpoint,250,
    2026-11-05T12:40:01.224001,0.101,pressure,250,231
"""

from __future__ import annotations

import csv
import threading
import time
from datetime import datetime
from pathlib import Path
from typing import Iterable

from rtu_server.server import RegisterUpdate

DEFAULT_FIELDS = ("setpoint", "pressure")


def default_log_path(log_dir: str | Path = "logs") -> Path:
    """Return a timestamped CSV path inside ``log_dir``."""
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    return Path(log_dir) / f"rtu_log_{stamp}.csv"


class CsvDataLogger:
    """Subscribe an instance to ``ModbusRtuUIServer`` to log updates to CSV.

    The instance is callable, so it can be passed directly to
    ``rtu_server.subscribe(logger)``. It is thread-safe, since updates
    usually arrive from the server's worker thread.
    """

    def __init__(self, path: str | Path, fields: Iterable[str] = DEFAULT_FIELDS):
        self.path = Path(path)
        self.fields = tuple(fields)
        self._latest: dict[str, object] = {name: "" for name in self.fields}
        self._lock = threading.Lock()
        self._t0: float | None = None

        self.path.parent.mkdir(parents=True, exist_ok=True)
        # newline='' is required by the csv module to avoid blank lines on Windows
        self._file = open(self.path, "w", newline="", encoding="utf-8")
        self._writer = csv.writer(self._file)
        self._writer.writerow(["timestamp", "elapsed_s", "source", *self.fields])
        self._file.flush()

    def __call__(self, update: RegisterUpdate) -> None:
        self.on_register_update(update)

    def on_register_update(self, update: RegisterUpdate) -> None:
        if update.name not in self.fields:
            return

        now = time.monotonic()
        with self._lock:
            if self._file.closed:
                return
            if self._t0 is None:
                self._t0 = now

            self._latest[update.name] = update.value
            self._writer.writerow([
                datetime.now().isoformat(timespec="microseconds"),
                f"{now - self._t0:.3f}",
                update.name,
                *(self._latest[name] for name in self.fields),
            ])
            # Flush every row so data survives a crash or a pulled USB cable
            self._file.flush()

    def close(self) -> None:
        with self._lock:
            if not self._file.closed:
                self._file.close()

    def __enter__(self) -> "CsvDataLogger":
        return self

    def __exit__(self, *exc) -> None:
        self.close()