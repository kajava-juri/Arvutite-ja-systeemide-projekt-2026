from PyQt6.QtWidgets import (
    QApplication, QHBoxLayout, QMainWindow, QPushButton, QVBoxLayout, QWidget,
    QLabel,
)
from PyQt6.QtGui import QShortcut
from PyQt6.QtCore import QSize, Qt
import sys

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
        root_layout.addWidget(content_area)


        QShortcut(Qt.Key.Key_Escape, self, member=lambda: self.close())



        


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())