# rtu_server

Modbus RTU server/slave that stores a fixed amount of registers and maps them to contextualised values. This project is tailored to work with the microcontroller firmware of this project.

## Installation

### Prerequisites

- Python 3.10 or newer (see `requires-python` in `pyproject.toml`)
- `pip` (install with `apt install python3-pip`)
- Access to a serial port / RS-485 adapter if you run the server against real hardware. (Can also be used with `socat` to simulate virtual ports)

### 1. Get the source

```bash
git clone <repository-url>
cd <repository-folder>
```

### 2. Create and activate a virtual environment (recommended)
On some systems it is required by default. A virtual environment keeps this project's dependencies separate from your system Python.

**Linux / macOS / WSL**

```bash
python3 -m venv .venv
source .venv/bin/activate
```

**Windows (PowerShell)**



```powershell
py -m venv .venv
.venv\Scripts\Activate.ps1
```

> [!NOTE] 
> If PowerShell refuses to run the activation script, run
> 
> `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned` once.
> 
> Source: https://stackoverflow.com/questions/4037939/powershell-says-execution-of-scripts-is-disabled-on-this-system

Your prompt should now start with `(.venv)`. Run `deactivate` to leave the environment.
Make sure `.venv/` is listed in `.gitignore`.

### 3. Upgrade pip

Older pip versions may not understand `pyproject.toml`-only projects.

```bash
python -m pip install --upgrade pip
```

### 4. Install the project

**Regular install** (copies the package into the venv):

```bash
pip install .
```

**Editable install** (recommended while developing — code changes take effect without reinstalling):

```bash
pip install -e .
```

The quotes stop shells like zsh from interpreting the square brackets.

### 5. Verify

```bash
python -m rtu_server --help
usage: python -m rtu_server [-h] [-p PORT] [-b BAUDRATE] [--parity {none,even,odd}] [--stopbits {1,2}] [--slave-id SLAVE_ID]

options:
  -h, --help            show this help message and exit
  -p, --port PORT       Serial port to use (default: /dev/ttyUSB0)
  -b, --baudrate BAUDRATE
                        Baud rate (default: 9600)
  --parity {none,even,odd}
                        Parity (default: none)
  --stopbits {1,2}      Stop bits (default: 2)
  --slave-id SLAVE_ID   Slave ID (default: 10)
```

#### Opening virtual port for testing

`rtu_server` requires to connect to a port to work, when developing you dont want to always want or have the possibility to have serial port connected.

In a new terminal run `socat` and replace with your username and ports you want and is not used (in this example `S10` and `S11`):
```bash
sudo socat -d -d PTY,rawer,link=/dev/ttyS10,user=<user> PTY,rawer,link=/dev/ttyS11,user=<user>
```
> [!NOTE]
> You may also replace one of the addresses with `-` if you only need to test one side.

### Serial port access

**Linux:** your user needs to be in the `dialout` group (log out and back in afterwards):

```bash
sudo usermod -aG dialout $USER
```

**WSL 2:** USB serial adapters are not visible by default. Attach them from Windows with
[usbipd-win](https://github.com/dorssel/usbipd-win); the device then appears as `/dev/ttyUSB*`.

**Windows:** use the COM port name shown in Device Manager (for example `COM3`).

### Updating and uninstalling

```bash
git pull
pip install .
```

**uninstalling**
``` bash
pip uninstall rtu_server
```

To start completely fresh, delete the `.venv` folder and repeat from step 2.

---

## Usage

```bash
python -m rtu_server [options]
```

<!-- Document the main options / example commands here. -->

---

## Building

"Building" turns the source into **distribution files** that others can install without
cloning the repo. There are two kinds:

| File | Name pattern | What it is |
|------|--------------|------------|
| **Wheel** | `rtu_server-0.1.0-py3-none-any.whl` | Ready-to-install package. This is what `pip` prefers. |
| **sdist** (source distribution) | `rtu_server-0.1.0.tar.gz` | Source archive; pip builds a wheel from it during install. |

This project uses the `flit_core.buildapi` backend, declared in `pyproject.toml`.

### 2. Build

```bash
pip install build
python -m build
```

This creates both files in `dist/`. `build` runs in an isolated temporary environment, so it
installs `flit_core` itself — you don't need it in your venv. Add `dist/` to `.gitignore`.

### 3. Test the wheel

Install it into a **fresh** virtual environment to make sure nothing depends on files that
only exist in your working copy:

```bash
python -m venv /tmp/test-env
source /tmp/test-env/bin/activate        # Windows: \tmp\test-env\Scripts\Activate.ps1
pip install dist/rtu_server-0.1.0-py3-none-any.whl
python -m rtu_server --help
deactivate
```
