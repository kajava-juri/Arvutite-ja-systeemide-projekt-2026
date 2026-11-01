import argparse

from PyQt6.QtWidgets import (
    QApplication, QHBoxLayout, QMainWindow, QPushButton, QVBoxLayout, QWidget,
    QLabel,
)
from PyQt6.QtGui import QShortcut
from PyQt6.QtCore import Qt, pyqtSignal
import sys

import serial

from rtu_server.server import ModbusRtuUIServer, RegisterUpdate, UpdateCallback
from rtu_server.registers import REGISTER_MAP, Register

PORT = '/dev/ttyUSB0' # 'COMXXX' on Windows
# PORT = None
BAUDRATE = 9600
PARITY = serial.PARITY_NONE
STOPBITS = 2

SLAVE_ID = 10
BLOCK = 'ui'
START_ADDR = 0
NUM_REGS = 20

def parity_from_string(parity_str):
    if parity_str == 'none':
        return serial.PARITY_NONE
    elif parity_str == 'even':
        return serial.PARITY_EVEN
    elif parity_str == 'odd':
        return serial.PARITY_ODD
    else:
        raise ValueError(f"Invalid parity value: {parity_str}")

class Header(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        # root_layout.addWidget(header_bar)
        self.setObjectName("header_bar")
        header_layout = QHBoxLayout(self)

        self.setAttribute(Qt.WidgetAttribute.WA_StyledBackground, True)
        self.setFixedHeight(128)

        exit_button = QPushButton("Exit")
        exit_button.clicked.connect(QApplication.quit)

        header_layout.addWidget(exit_button)

class ContentArea(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("content_area")

        container_layout = QHBoxLayout(self)

        self.labels = {}    # register name -> QLabel
        self.titles = {}    # register name -> text before the value

        # for name, title in [("pressure", "Pressure"), ("setpoint", "Setpoint"), ("pwm", "PWM")]:
        for name, register in REGISTER_MAP.items():
            title = name.capitalize()
            label = QLabel(f"{title}: -")
            font = label.font()
            font.setPointSize(48)
            label.setFont(font)
            label.setAlignment(Qt.AlignmentFlag.AlignCenter | Qt.AlignmentFlag.AlignLeft)
            container_layout.addWidget(label)
            self.labels[name] = label
            self.titles[name] = title

        # # === pressure label ===
        # pressure_label = QLabel("Pressure:")
        # font = pressure_label.font()
        # font.setPointSize(48)
        # pressure_label.setFont(font)
        # pressure_label.setAlignment(Qt.AlignmentFlag.AlignCenter |
        #                             Qt.AlignmentFlag.AlignLeft)
        # container_layout.addWidget(pressure_label)


        # # === setpoint label ===

        # setpoint_label = QLabel("Setpoint:")
        # font = setpoint_label.font()
        # font.setPointSize(48)
        # setpoint_label.setFont(font)
        # setpoint_label.setAlignment(Qt.AlignmentFlag.AlignCenter |
        #                             Qt.AlignmentFlag.AlignLeft)
        # container_layout.addWidget(setpoint_label)


        # # === pwm label ===
        # pwm_label = QLabel("PWM:")
        # font = pwm_label.font()
        # font.setPointSize(48)
        # pwm_label.setFont(font)
        # pwm_label.setAlignment(Qt.AlignmentFlag.AlignCenter |
        #                             Qt.AlignmentFlag.AlignLeft)
        # container_layout.addWidget(pwm_label)
        

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        # root window setup
        self.setWindowTitle("PyQt6 Demo")
        # self.showMaximized()
        self.showFullScreen()

        root_widget = QWidget()
        root_layout = QVBoxLayout()
        root_widget.setLayout(root_layout)

        self.setCentralWidget(root_widget)

        # header bar layout

        header_bar = Header(root_widget)
        root_layout.addWidget(header_bar)


        # content area layout

        content_area = ContentArea(root_widget)
        self.content_area = content_area
        root_layout.addWidget(content_area)


        QShortcut(Qt.Key.Key_Escape, self, member=lambda: self.close())

    def on_register_update(self, update: RegisterUpdate):
        label = self.content_area.labels.get(update.name)
        if label:
            title = self.content_area.titles.get(update.name, update.name)
            label.setText(f"{title}: {update.value} {update.unit}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("-p", "--port", help="Serial port to use (default: /dev/ttyUSB0)", default=PORT)
    parser.add_argument("-b", "--baudrate", help="Baud rate (default: 9600)", type=int, default=BAUDRATE)
    parser.add_argument("--parity", help="Parity (default: none)", choices=['none', 'even', 'odd'], default='none')
    parser.add_argument("--stopbits", help="Stop bits (default: 2)", type=int, choices=[1, 2], default=STOPBITS)
    parser.add_argument("--slave-id", help="Slave ID (default: 10)", type=int, default=SLAVE_ID)

    args = parser.parse_args()

    rtu_server = ModbusRtuUIServer(
        port=args.port,
        slave_id=args.slave_id,
        block=BLOCK,
        start_addr=START_ADDR,
        num_regs=NUM_REGS,
        baudrate=args.baudrate,
        parity=parity_from_string(args.parity),
        stopbits=args.stopbits
    )

    rtu_server.start()

    app = QApplication(sys.argv)
    window = MainWindow()

    rtu_server.subscribe(window.on_register_update)

    app.aboutToQuit.connect(rtu_server.stop)

    window.show()
    sys.exit(app.exec())