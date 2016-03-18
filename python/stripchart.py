#!/usr/bin/env python3

from gi.repository import Gtk, Gdk
from collections import deque
from datetime import datetime, timedelta


class Color:
    negro = (0,0,0)
    gris = (0.3, 0.3, 0.3)
    rojo = (1.0, 0, 0)
    asulito = (30/255.0, 144/255.0, 255/255.0)
    morado  = (128/255.0, 0/255.0, 128/255.0)
    naranja = (255/255.0, 79/255.0, 0/255.0)
    fondo = (255/255.0, 244/255.0, 238/255.0)


class Serie:
    titulo = ''
    color = (0, 0, 0)   # tupla (r, g ,b)
    puntos = None       # deque de valores y
    maximo = float('-inf')
    minimo = float('inf')

class StripChart(Gtk.DrawingArea):

    def __init__(self, ancho=500, alto=250, tipo='linea', font_face= '', font_size=11):
        Gtk.DrawingArea.__init__(self)
        self.set_size_request(ancho, alto)
        self.connect("draw", self._on_draw)
        self.add_events(Gdk.EventMask.POINTER_MOTION_MASK)
        self.connect("motion_notify_event", self._on_mouse_move)

        self.intervalo = 1      # Intervalo de muestreo (seg.)
        self.duracion = 60      # Duracion total del grafico (seg.)
        self.val_min = None        # Valor maximo
        self.val_max = None      # Valor minimo
        self.ahora = datetime.now()
        self.datos = []         # Contiene las series
        self.inter_val = 10
        self.tipo = tipo
        self.font_size = font_size
        self.font_face = font_face
        self.b_izq = 0          # Bordes
        self.b_der = 5          # Bordes
        self.b_sup = 0          # Bordes
        self.b_inf = 0          # Bordes
        self.wrect = 0          # Dimensiones de la ventana del widget
        self.g_ancho = 1        # Anchura del grafico
        self.g_alto = 1         # Altura del grafico
        self.cursor_tiempo = 0  # Posicion en tiempo del cursor (seg.)
        self.cursor_on = False
        self.cursor_x = 0
        self.cursor_y = 0
        self.cursor_index = 0

    def _on_mouse_move(self, widget, cursor):
        self.cursor_x = cursor.x
        self.cursos_y = cursor.y
        x_en_grafica = cursor.x >= self.b_izq and cursor.x <= self.b_izq + self.g_ancho
        y_en_grafica = cursor.y >= self.b_sup and cursor.y <= self.b_sup + self.g_alto
        self.cursor_on = x_en_grafica and y_en_grafica
        self.cursor_tiempo = 0
        self.cursor_index = 0
        if self.cursor_on:
            self.cursor_tiempo = (self.b_izq + self.g_ancho - cursor.x) * self.duracion / self.g_ancho
            self.cursor_index = round(self.cursor_tiempo / self.intervalo)
            n_puntos = len(self.datos[0].puntos)
            if self.cursor_index < 0 or self.cursor_index >= n_puntos:
                self.cursor_tiempo = 0
                self.cursor_index = 0
                self.cursor_on = False
        self.queue_draw()
        return False

    def _on_draw(self, widget, ct):
        ct.select_font_face(self.font_face)
        ct.set_font_size(self.font_size)
        (foo, font_desc, font_alto, font_ancho, foo) = ct.font_extents()
        # Anchura bordes
        t_vmax = str(self.val_max)
        t_vmin = str(self.val_min)
        self.b_izq = font_ancho + max(ct.text_extents(t_vmax)[2], ct.text_extents(t_vmin)[2])
        self.b_sup = self.b_inf = font_alto
        # Area del grafico
        self.wrect = self.get_allocation()
        self.g_ancho = self.wrect.width - self.b_izq - self.b_der
        self.g_alto = self.wrect.height - self.b_sup - self.b_inf
        ct.set_source_rgb(*Color.fondo)
        ct.rectangle(self.b_izq, self.b_sup, self.g_ancho, self.g_alto)
        ct.fill()
        # Etiquetas eje y lineas secundarias
        ct.set_source_rgb(*Color.gris)
        ct.set_line_width(0.2)
        inter_y = self.inter_val * self.g_alto / (self.val_max - self.val_min)
        n_lineas = round((self.val_max - self.val_min) / self.inter_val)
        for n in range(0,n_lineas):
            ct.move_to(font_ancho/2, self.b_sup + self.g_alto - n*inter_y + font_alto/4)
            ct.set_source_rgb(*Color.negro)
            ct.show_text(str(self.val_min + n*self.inter_val))
            ct.move_to(self.b_izq, self.b_sup + self.g_alto - n*inter_y)
            ct.set_source_rgb(*Color.gris)
            ct.rel_line_to(self.g_ancho, 0)
        ct.move_to(font_ancho/2, self.b_sup + font_alto/4)
        ct.set_source_rgb(*Color.negro)
        ct.show_text(t_vmax)
        ct.move_to(self.b_izq, self.b_sup)
        ct.set_source_rgb(*Color.gris)
        ct.rel_line_to(self.g_ancho, 0)
        ct.stroke()
        # Cursor y eiquetas del cursor
        ct.set_line_width(0.8)
        #~ x = self.b_izq  + self.g_ancho
        if self.cursor_on:
            x = self.cursor_x
            ct.set_source_rgb(*Color.negro)
            ct.move_to(x, self.b_sup)
            ct.rel_line_to(0, self.g_alto)
            ct.stroke()
            cursor_hora = datetime.strftime(self.ahora - timedelta(seconds = self.cursor_tiempo), "%x %H:%M")
            texto = str(cursor_hora)
            for serie in self.datos:
                punto = serie.puntos[self.cursor_index]
                texto += '  ' + str(round(float(punto),1))
            texto_ext = ct.text_extents(texto)[4]
            y = self.wrect.height - font_desc
            if self.cursor_x > self.wrect.width/2 or not self.cursor_on:
                ct.move_to(x - texto_ext, y)
            else:
                ct.move_to(x, y)
            ct.show_text(str(cursor_hora))
            for serie in self.datos:
                punto = serie.puntos[self.cursor_index]
                ct.set_source_rgb(*serie.color)
                ct.show_text( '  ' + str(round(float(punto),1)))
        # Leyenda
        ct.move_to(self.b_izq, self.b_sup - font_desc)
        for serie in self.datos:
            ct.set_source_rgb(*serie.color)
            ct.show_text(serie.titulo + '   ')
        # Dibuja curvas de datos
        self._draw_datos(ct)
        return False

    def _draw_datos(self, ct):
        x0 = self.b_izq + self.g_ancho
        y0 = self.b_sup + self.g_alto
        ct.set_line_width(1.0)
        for serie in self.datos:
            ct.set_source_rgb(*serie.color)
            x = 0
            for y in serie.puntos:
                dx = x * self.g_ancho / self.duracion
                dy = (y - self.val_min) * self.g_alto / (self.val_max - self.val_min)
                if self.tipo == 'linea':
                    if x==0:
                        ct.move_to(x0, y0 - dy -1)
                    ct.line_to(x0 - dx, y0 - dy)
                else:
                    ct.new_sub_path()
                    ct.arc(x0 - dx, y0 - dy, 1, 0, 2*3.1416)
                x += self.intervalo
            ct.stroke()

    def set_ejes(self, duracion, intervalo, val_min, val_max, intervalo_lineas_secundarias= 10):
        self.intervalo = intervalo
        self.duracion = duracion
        self.val_min = val_min
        self.val_max = val_max
        self.inter_val = intervalo_lineas_secundarias

    def add_serie(self, titulo, color):
        s = Serie()
        s.titulo = titulo
        s.color = color
        s.puntos = deque()
        self.datos.append(s)

    def add_valor(self, nserie, valor):
        if nserie >= 0 and nserie < len(self.datos):
            serie = self.datos[nserie]
            serie.puntos.appendleft(valor)
            serie.maximo = max(serie.maximo, valor)
            serie.minimo = min(serie.minimo, valor)
            # Determina valores extremos del eje y
            if self.val_max == None or self.val_min == None:
                if self.val_max == None:
                    self.val_max = valor - (valor % self.inter_val) + self.inter_val
                if self.val_min == None:
                    self.val_min = valor - (valor % self.inter_val)# - self.inter_val
            else:
                if valor > self.val_max:
                    self.val_max = valor - (valor % self.inter_val) + self.inter_val
                elif valor < self.val_min:
                    self.val_min = valor - (valor % self.inter_val)# - self.inter_val
            # Elimina de la cola el punto mas antiguo cuando la grafica está completa
            t = len(serie.puntos)
            if t > (self.duracion / self.intervalo + 1):
                serie.puntos.pop()

    # Solo llamar cuando se anyaden nuevos valores
    def update(self):
        self.ahora = datetime.now()
        self.queue_draw()


def test():
    from gi.repository import GLib
    import random

    def timer():
        grafico.add_valor(0, random.randint(0,100))
        #~ grafico.add_valor(1, random.randint(20,30))
        #~ grafico.add_valor(2, random.randint(70,90))
        grafico.update()
        return True

    intervalo_muestreo = 1
    duracion = 5 * 60
    grafico = StripChart(900,150, tipo = 'puntos', font_face = 'Noto Sans', font_size=10)
    grafico.set_ejes(duracion, intervalo_muestreo, None, None, 2)
    grafico.add_serie(titulo='T. exterior (ºC)', color=Color.asulito)
    #~ grafico.add_serie('T. interior (ºC)', Color.morado)
    #~ grafico.add_serie('H. exterior (ºC)', Color.naranja)

    window = Gtk.Window()
    window.add(grafico)
    window.connect("destroy", Gtk.main_quit)
    window.show_all()
    GLib.timeout_add(intervalo_muestreo*1000,timer)
    timer()
    Gtk.main()


if __name__ == "__main__":
    test()
