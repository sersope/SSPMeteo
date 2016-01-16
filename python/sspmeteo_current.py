#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#
# Sergio Soriano Peiro - sersope@gmail.com
"""Lectura continua de sensores de sspmeteo.

"""
from __future__ import division,absolute_import,print_function,unicode_literals
import curses
import time
import socket

cs = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
cs.connect(('192.168.1.20', 5556))

tecla = None
pantalla = curses.initscr()
curses.noecho()
pantalla.nodelay(1)
pantalla.addstr(0,1,'sspmeteo_current')
cs_error = False

while tecla != ord('q') and tecla != ord('Q') and not cs_error:
    cs.send("getcurrent\r\n")
    d = cs.recv(128)
    if len(d)>0:
        l=[float(x) for x in d.split(',')]
        pantalla.addstr( 4, 1, 'Temperatura     = {:5.1f} *C'.format(l[0]))
        pantalla.addstr( 5, 1, 'Humedad         = {:5.1f} %'.format(l[1]))
        pantalla.addstr( 6, 1, 'Punto de rocio  = {:5.1f} *C'.format(l[2]))
        pantalla.addstr( 7, 1, 'Lluvia total    = {:5.1f} mm (ticks {:5.0f})'.format(l[3], l[3] / 0.138)) # 0.138 mm/tick
        pantalla.addstr( 8, 1, 'Lluvia ult.hora = {:5.1f} mm'.format(l[4]))
        pantalla.addstr( 9, 1, 'Lluvia diaria   = {:5.1f} mm'.format(l[5]))
        pantalla.addstr(10, 1, 'Vel. viento     = {:5.1f} km/h'.format(l[6]))
        pantalla.addstr(11, 1, 'Vel. racha      = {:5.1f} km/h'.format(l[7]))
        pantalla.addstr(12, 1, 'Dir. viento     = {:5.0f} grados'.format(l[8]))
        pantalla.addstr(13, 1, 'Presion rel.    = {:5.1f} mbar'.format(l[9]))
        pantalla.addstr(14, 1, 'Temp. interior  = {:5.1f} *C'.format(l[10]))
        pantalla.addstr(15, 1, 'Cal. recepc.    = {:5.1f} %'.format(100.0 * l[11] / 54.0))
        pantalla.addstr(16, 1, '                  {:2.0f} {:2.0f} {:2.0f} {:2.0f} {:2.0f} {:2.0f} -> {:2.0f}'.format(l[12],l[13],l[14],l[15],l[16],l[17],l[11]))
        pantalla.addstr(18,1,"('q' para terminar.)")
    else:
        cs_error = True
    time.sleep(0.25)
    tecla = pantalla.getch()

cs.close()
curses.nocbreak(); pantalla.keypad(0); curses.echo()
curses.endwin()
if cs_error:
    print("Se perdió la conexion con el servidor.")

