#!/usr/bin/env python3

from gi.repository import Gtk, GLib
import socket
import os
from datetime import datetime, date
from stripchart import StripChart, Color

APP_DIR=os.path.dirname(os.path.realpath(__file__))
UI_FILE = os.path.join(APP_DIR,'sspmeteo_gui.glade')


labels = ['a_temp_out','a_hum_out','a_dew_out','d_rain','m_rain','y_rain','a_wind_gust','d_wind_gust',
          'a_wind_dir','a_rel_pressure','a_temp_in']
       #~ 'a_hum_in','d_temp_out_max','d_temp_out_min','d_hum_out_max','d_hum_out_min','d_rel_pressure_max',
       #~ 'd_rel_pressure_min','d_temp_in_max','d_temp_in_min','d_hum_in_max',
       #~ 'd_hum_in_min','m_temp_out_max_hi','m_temp_out_min_lo','m_hum_out_max','m_hum_out_min',
       #~ 'm_rel_pressure_max','m_rel_pressure_min','m_wind_gust','m_temp_in_max_hi',
       #~ 'm_temp_in_min_lo','m_hum_in_max','m_hum_in_min','y_temp_out_max_hi','y_temp_out_min_lo',
       #~ 'y_hum_out_max','y_hum_out_min','y_rel_pressure_max','y_rel_pressure_min','y_wind_gust',
       #~ 'y_temp_in_max_hi','y_temp_in_min_lo','y_hum_in_max','y_hum_in_min'
       #~ ]


formatos = ['{:5.1f} ºC','{:5.1f} %','{:5.1f} ºC','{:5.1f} mm','{:5.1f} mm','{:5.1f} mm','{:5.1f} km/h',
            '{:5.1f} km/h','{:5.0f} º','{:5.1f} mbar','{:5.1f} ºC']


class MainWindow(object):
    """Clase manejadora de la ventana principal.

    """
    def __init__(self):

        self.primera_vez = True

        self.builder = Gtk.Builder()
        self.builder.add_from_file(UI_FILE)
        self.builder.connect_signals(self)
        self.win=self.builder.get_object('window1')
        self.graf_box=self.builder.get_object('graf_box')
        #~ self.grid1 = self.builder.get_object('grid1')
        #~ self.calendario = Gtk.Calendar()
        #~ self.grid1.attach(self.calendario,6,2,1,1)
        self.status = ''
        self.statusbar = self.builder.get_object('statusbar')
        self.statusbar.push(0,self.status)

        self.ui_label = {}
        for label in labels:
            self.ui_label[label] = self.builder.get_object(label)

        self.grafico1 = StripChart(ancho=800, alto=150, font_face='Noto Sans', font_size=11)
        self.grafico2 = StripChart(alto=150, font_face='Noto Sans', font_size=11)
        self.grafico3 = StripChart(alto=150, font_face='Noto Sans', font_size=11)
        self.grafico4 = StripChart(alto=150, font_face='Noto Sans', font_size=11)
        self.graf_box.pack_start(self.grafico1,True,True,0)
        self.graf_box.pack_start(self.grafico2,True,True,0)
        self.graf_box.pack_start(self.grafico3,True,True,0)
        self.graf_box.pack_start(self.grafico4,True,True,0)
        self.grafico1.set_ejes(duracion, intervalo_muestreo, None, None,5)
        self.grafico2.set_ejes(duracion, intervalo_muestreo, None, None,10)
        self.grafico3.set_ejes(duracion, intervalo_muestreo, None, None,5)
        self.grafico4.set_ejes(duracion, intervalo_muestreo, 0, None,5)
        self.grafico1.add_serie('Temperatura exterior (ºC)', Color.asulito)
        self.grafico1.add_serie('Temperatura interior (ºC)', Color.morado)
        self.grafico2.add_serie('Humedad exterior (%)', Color.asulito)
        self.grafico3.add_serie('Presión (mbar)', Color.negro)
        self.grafico4.add_serie('Velocidad del viento (km/h)', Color.naranja)
        self.grafico4.add_serie('Rachas (km/h)', Color.rojo)

        self.win.show_all()
        self._update_inicial_graficos()
        self._update_ui()
        GLib.timeout_add(intervalo_muestreo *1000, self._update_ui)

    def _update_inicial_graficos(self):
        global err_conexion
        peticion = 'getfile ' + date.today().isoformat() + '.dat\r\n'   # Fichero del dia
        conexion.send(peticion.encode())
        fichero = ''
        while True:
            chunk = conexion.recv(4096).decode()
            if len(chunk):
                fichero += chunk
                if  '\v' == fichero[-1]:
                    break;
            else:
                err_conexion = True
                self.status += ' ERROR DE CONEXION.'
                return
        lineas_fichero = fichero[:-2].split('\n')   # Quita '\n\v' del final
        for linea in lineas_fichero:
            linea_sin_hora = linea.split(',')[2:]   # Elimina fecha y hora al principio
            datos=[float(x) for x in linea_sin_hora]
            self._update_graficos(datos)

    def _update_labels(self, datos):
        for l,f,v in zip(labels, formatos, datos):
            self.ui_label[l].set_label(f.format(v))
        self.status = 'Último acceso: ' + datetime.now().strftime('%x %X') +\
                      '  Recepción: {:5.1f}%'.format(100.0 * datos[11] / 54.0)

    def _update_graficos(self,datos):
        self.grafico1.add_valor(0, datos[0])
        self.grafico1.add_valor(1, datos[10])
        self.grafico2.add_valor(0, datos[1])
        self.grafico3.add_valor(0, datos[9])
        self.grafico4.add_valor(0, datos[6])
        self.grafico4.add_valor(1, datos[7])
        self.grafico1.update()
        self.grafico2.update()
        self.grafico3.update()
        self.grafico4.update()

    def _update_ui(self):
        global err_conexion
        self.statusbar.pop(0)
        if not err_conexion:
            conexion.send("getcurrent\r\n".encode())
            respuesta = conexion.recv(128).decode()
            if len(respuesta)>0:
                datos=[float(x) for x in respuesta.split(',')]
                self._update_labels(datos)
                if not self.primera_vez:
                    self._update_graficos(datos)
            else:
                err_conexion = True
                self.status += ' ERROR DE CONEXION.'
        self.statusbar.push(0, self.status)
        self.primera_vez = False
        return True

    def on_window_destroy(self,win):
        Gtk.main_quit()

    def run(self):
        Gtk.main()


intervalo_muestreo = 5 * 60
duracion = 24 * 60 * 60
err_conexion = False
conexion = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print('Conectando...')
conexion.connect(('192.168.1.20', 5556))
MainWindow().run()
conexion.close()
