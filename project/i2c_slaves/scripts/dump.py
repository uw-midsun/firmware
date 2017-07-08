#!/usr/bin/env python3
import serial
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui, QtCore

class XYTrace():
  def __init__(self):
    self.plt = pg.plot()
    self.plt.setRange(xRange=(-2048, 2048), yRange=(-2048, 2048))
    self.curve = self.plt.plot([0], [0], pen=None, symbol='o')

    self.timer = QtCore.QTimer()
    self.timer.timeout.connect(self.update)
    self.timer.start(0)

    self.data = self.get_data()

  def update(self):
    try:
      x, y, z = next(self.data)
      self.curve.setData([x], [y])
    except:
      pass

  def get_data(self):
    with serial.Serial('/dev/ttyCMSIS', 115200) as ser:
      while True:
        try:
          source, *data = ser.readline().decode('ascii').rstrip().split(' ')
          if source == 'MAG':
            yield map(int, data)
        except:
          continue

app = QtGui.QApplication([])
trace = XYTrace()
app.exec_()
