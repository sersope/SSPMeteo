#!/usr/bin/env python3

from gi.repository import Gtk, GLib
import socket
import os
from datetime import datetime, date
from stripchart import StripChart, Color

APP_DIR=os.path.dirname(os.path.realpath(__file__))
UI_FILE = os.path.join(APP_DIR,'sspmeteo_gui.glade')


labels = ['a_temp_out','a_hum_out','a_dew_out','d_rain','h_rain','d_rain','a_wind_vel','d_wind_vel',
          'a_wind_dir','a_rel_pressure','a_temp_in', 'd_temp_out_max', 'd_temp_out_min',
          'd_hum_out_max','d_hum_out_min', 'd_temp_in_max', 'd_temp_in_min',
          'd_rel_pressure_max', 'd_rel_pressure_min', 'd_wind_racha']


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
        self.grid_graf=self.builder.get_object('grid_graf')
        #~ self.grid1 = self.builder.get_object('grid1')
        #~ self.calendario = Gtk.Calendar()
        #~ self.grid1.attach(self.calendario,6,2,1,1)
        self.status = ''
        self.statusbar = self.builder.get_object('statusbar')
        self.statusbar.push(0,self.status)

        self.ui_label = {}
        for label in labels:
            self.ui_label[label] = self.builder.get_object(label)

        ancho = 400
        alto = 150
        self.grafico1 = StripChart(ancho, alto, font_face='Noto Sans', font_size=11)
        self.grafico2 = StripChart(ancho, alto, font_face='Noto Sans', font_size=11)
        self.grafico3 = StripChart(ancho, alto, font_face='Noto Sans', font_size=11)
        self.grafico4 = StripChart(ancho, alto, tipo='puntos', font_face='Noto Sans', font_size=11)
        self.grafico5 = StripChart(ancho, alto, font_face='Noto Sans', font_size=11)
        self.grafico6 = StripChart(ancho, alto, font_face='Noto Sans', font_size=11)
        self.grid_graf.attach(self.grafico1,0,0,1,1)
        self.grid_graf.attach(self.grafico2,1,0,1,1)
        self.grid_graf.attach(self.grafico3,0,1,1,1)
        self.grid_graf.attach(self.grafico4,1,1,1,1)
        self.grid_graf.attach(self.grafico5,0,2,1,1)
        self.grid_graf.attach(self.grafico6,1,2,1,1)
        self.grafico1.set_ejes(duracion, intervalo_muestreo, None, None, 5)
        self.grafico1.add_serie('Temperatura exterior (ºC)', Color.asulito)
        self.grafico1.add_serie('Temperatura interior (ºC)', Color.morado)
        self.grafico2.set_ejes(duracion, intervalo_muestreo, None, None,10)
        self.grafico2.add_serie('Humedad exterior (%)', Color.asulito)
        self.grafico3.set_ejes(duracion, intervalo_muestreo, 0, 20, 5)
        self.grafico3.add_serie('Velocidad del viento (km/h)', Color.naranja)
        self.grafico3.add_serie('Rachas (km/h)', Color.morado)
        self.grafico4.set_ejes(duracion, intervalo_muestreo, 0, 360, 90)
        self.grafico4.add_serie('Dirección del viento (º)', Color.rojo)
        self.grafico5.set_ejes(duracion, intervalo_muestreo, None, None, 1)
        self.grafico5.add_serie('Presión (mbar)', Color.negro)
        self.grafico6.set_ejes(duracion, intervalo_muestreo, 0, 20, 5)
        self.grafico6.add_serie('Lluvia (mm)', Color.asulito)
        self.grafico6.add_serie('Lluvia últ. hora (mm)', Color.naranja)

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
        lineas_fichero = fichero[:-2].split('\n')   # Quita '\n\v' del final del fichero
        for linea in lineas_fichero:
            linea_sin_hora = linea.split(',')[2:]   # Elimina fecha y hora al principio
            datos=[float(x) for x in linea_sin_hora]
            self._update_graficos(datos)

    def _update_labels(self, datos):
        for l,f,v in zip(labels, formatos, datos):
            self.ui_label[l].set_label(f.format(v))
        self.status = 'Último acceso: ' + datetime.now().strftime('%x %X') +\
                      '  Recepción: {:5.1f}%'.format(100.0 * datos[11] / 54.0)
        # Datos calculados
        self.ui_label['d_temp_out_max'].set_label(str(self.grafico1.datos[0].maximo))
        self.ui_label['d_temp_out_min'].set_label(str(self.grafico1.datos[0].minimo))
        self.ui_label['d_temp_in_max'].set_label(str(self.grafico1.datos[1].maximo))
        self.ui_label['d_temp_in_min'].set_label(str(self.grafico1.datos[1].minimo))
        self.ui_label['d_hum_out_max'].set_label(str(self.grafico2.datos[0].maximo))
        self.ui_label['d_hum_out_min'].set_label(str(self.grafico2.datos[0].minimo))
        self.ui_label['d_wind_vel'].set_label(str(round(self.grafico3.datos[0].maximo,1)))
        self.ui_label['d_wind_racha'].set_label(str(round(self.grafico3.datos[1].maximo,1)))
        self.ui_label['d_rel_pressure_max'].set_label(str(round(self.grafico5.datos[0].maximo,1)))
        self.ui_label['d_rel_pressure_min'].set_label(str(round(self.grafico5.datos[0].minimo,1)))

    def _update_graficos(self,datos):
        self.grafico1.add_valor(0, datos[0])
        self.grafico1.add_valor(1, datos[10])
        self.grafico2.add_valor(0, datos[1])
        self.grafico3.add_valor(0, datos[6])
        self.grafico3.add_valor(1, datos[7])
        self.grafico4.add_valor(0, datos[8])
        self.grafico5.add_valor(0, datos[9])
        self.grafico6.add_valor(0, datos[5])
        self.grafico6.add_valor(1, datos[4])
        self.grafico1.update()
        self.grafico2.update()
        self.grafico3.update()
        self.grafico4.update()
        self.grafico5.update()
        self.grafico6.update()

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
print('Conectado.')
MainWindow().run()
conexion.close()
