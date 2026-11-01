from PyQt6.QtWidgets import (
    QApplication, QHBoxLayout, QMainWindow, QPushButton, QVBoxLayout, QWidget,
    QLabel,
)
from PyQt6.QtCore import QSize, Qt
import sys


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

        header_bar = QWidget()
        root_layout.addWidget(header_bar)

        header_layout = QHBoxLayout()
        header_bar.setLayout(header_layout)

        header_bar.setAttribute(Qt.WidgetAttribute.WA_StyledBackground, True)
        header_bar.setFixedHeight(128)

        exit_button = QPushButton("Exit")
        exit_button.clicked.connect(self.close)

        header_layout.addWidget(exit_button)


        # content area layout

        content_area = QWidget()
        root_layout.addWidget(content_area)

        container_layout = QHBoxLayout()
        content_area.setLayout(container_layout)

        # === pressure label ===
        pressure_label = QLabel("Pressure:")
        font = pressure_label.font()
        font.setPointSize(48)
        pressure_label.setFont(font)
        pressure_label.setAlignment(Qt.AlignmentFlag.AlignCenter |
                                    Qt.AlignmentFlag.AlignLeft)
        container_layout.addWidget(pressure_label)


        # === setpoint label ===

        setpoint_label = QLabel("Setpoint:")
        font = setpoint_label.font()
        font.setPointSize(48)
        setpoint_label.setFont(font)
        setpoint_label.setAlignment(Qt.AlignmentFlag.AlignCenter |
                                    Qt.AlignmentFlag.AlignLeft)
        container_layout.addWidget(setpoint_label)


        # === pwm label ===
        pwm_label = QLabel("PWM:")
        font = pwm_label.font()
        font.setPointSize(48)
        pwm_label.setFont(font)
        pwm_label.setAlignment(Qt.AlignmentFlag.AlignCenter |
                                    Qt.AlignmentFlag.AlignLeft)
        container_layout.addWidget(pwm_label)


        


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())