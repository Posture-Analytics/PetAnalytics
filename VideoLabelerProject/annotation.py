import tkinter as tk
from tkinter import ttk
from tkinter import filedialog, simpledialog
import cv2
from PIL import Image, ImageTk
import math
import csv
import os

class VideoLabeler(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Video Labeler")
        self.geometry("800x600")

        # Variáveis de controle
        self.video_path = 'teste.mp4'  # Atualize para o caminho do seu vídeo
        self.cap = cv2.VideoCapture(self.video_path)
        if not self.cap.isOpened():
            print("Erro ao abrir o vídeo.")
            return
        self.total_frames = int(self.cap.get(cv2.CAP_PROP_FRAME_COUNT))
        self.fps = self.cap.get(cv2.CAP_PROP_FPS)
        self.duration = self.total_frames / self.fps  # Duração total em segundos
        self.current_frame_index = 0
        self.playback_speed = 1.0  # Velocidade de reprodução
        self.labels = []  # Lista de regiões e labels
        self.zoom_scale = 1.0  # Nível de zoom da barra de progresso

        # Estados de seleção de região
        self.selecting_region = False
        self.region_start_frame = None

        # Flags de controle
        self.is_playing = True
        self.is_frame_by_frame = False

        # Variáveis para exibição do tempo na barra
        self.time_display = None  # Referência para o texto na timeline

        # Variáveis para avanço contínuo de frames
        self.holding_next = False
        self.holding_prev = False

        # Marcadores de início de região
        self.region_start_markers = []

        # Opções de labels predefinidas
        self.label_options = ['Caminhando', 'Comendo', 'Dormindo', 'Brincando']

        # Criação dos widgets
        self.create_widgets()

        # Bind para fechar o aplicativo corretamente
        self.protocol("WM_DELETE_WINDOW", self.on_closing)

        # Iniciar a atualização dos frames
        self.update_frame()

    def create_widgets(self):
        # Frame principal
        main_frame = tk.Frame(self)
        main_frame.grid(row=0, column=0, sticky='nsew')

        # Configurar pesos para o redimensionamento adequado
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)
        main_frame.grid_rowconfigure(0, weight=1)
        main_frame.grid_columnconfigure(0, weight=1)

        # Área de exibição de vídeo
        self.video_label = tk.Label(main_frame)
        self.video_label.grid(row=0, column=0, sticky='nsew')

        # Controles de reprodução
        controls_frame = tk.Frame(main_frame)
        controls_frame.grid(row=1, column=0, pady=5)

        tk.Button(controls_frame, text="Play/Pause", command=self.toggle_playback).pack(side='left')
        tk.Button(controls_frame, text="Velocidade 0.25x", command=lambda: self.set_speed(0.25)).pack(side='left')
        tk.Button(controls_frame, text="Velocidade 1x", command=lambda: self.set_speed(1)).pack(side='left')
        tk.Button(controls_frame, text="Velocidade 5x", command=lambda: self.set_speed(5)).pack(side='left')  # Substituído 2x por 5x
        tk.Button(controls_frame, text="Modo Frame a Frame", command=self.toggle_frame_by_frame_mode).pack(side='left')

        # Botões de frame-by-frame
        self.prev_frame_button = tk.Button(controls_frame, text="<< Frame Anterior")
        self.prev_frame_button.pack(side='left')
        self.next_frame_button = tk.Button(controls_frame, text="Próximo Frame >>")
        self.next_frame_button.pack(side='left')

        # Eventos para avanço contínuo
        self.prev_frame_button.bind("<ButtonPress>", self.start_holding_prev)
        self.prev_frame_button.bind("<ButtonRelease>", self.stop_holding_prev)
        self.next_frame_button.bind("<ButtonPress>", self.start_holding_next)
        self.next_frame_button.bind("<ButtonRelease>", self.stop_holding_next)

        # Inicialmente desabilitados
        self.prev_frame_button.config(state='disabled')
        self.next_frame_button.config(state='disabled')

        # Barra de progresso personalizada
        self.timeline_canvas = tk.Canvas(main_frame, height=50, bg='lightgray')
        self.timeline_canvas.grid(row=2, column=0, sticky='ew')
        self.timeline_canvas.bind("<Button-1>", self.on_timeline_click)
        self.timeline_canvas.bind("<Control-Button-1>", self.start_region)
        self.timeline_canvas.bind("<Control-Button-3>", self.end_region)

        # Adicionar barra de rolagem horizontal
        self.timeline_scrollbar = tk.Scrollbar(main_frame, orient='horizontal', command=self.timeline_canvas.xview)
        self.timeline_scrollbar.grid(row=3, column=0, sticky='ew')
        self.timeline_canvas.config(xscrollcommand=self.timeline_scrollbar.set)

        # Bindings para eventos de scroll do mouse
        self.bind_mousewheel_events()

        # Evento para mostrar o tempo na timeline
        self.timeline_canvas.bind("<Motion>", self.show_time_on_timeline)

    def bind_mousewheel_events(self):
        # Eventos de scroll para Windows
        self.timeline_canvas.bind("<MouseWheel>", self.zoom_timeline)

        # Eventos de scroll para Linux (X11)
        self.timeline_canvas.bind("<Button-4>", self.zoom_timeline)
        self.timeline_canvas.bind("<Button-5>", self.zoom_timeline)

    def update_frame(self):
        if self.is_playing and not self.is_frame_by_frame:
            if self.cap.isOpened():
                ret, frame = self.cap.read()
                if ret:
                    self.current_frame_index = int(self.cap.get(cv2.CAP_PROP_POS_FRAMES))
                    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

                    # Calcular o tempo atual em segundos
                    current_time_sec = self.current_frame_index / self.fps

                    # Sobrepor o tempo no frame
                    text = f"Tempo: {current_time_sec:.2f} s"
                    cv2.putText(frame, text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX,
                                1, (255, 255, 255), 2, cv2.LINE_AA)

                    # Redimensionar o frame para caber no video_label
                    frame = self.resize_frame(frame)

                    img = Image.fromarray(frame)
                    imgtk = ImageTk.PhotoImage(image=img)
                    self.video_label.imgtk = imgtk
                    self.video_label.configure(image=imgtk)

                    # Chamar draw_timeline antes de scroll_timeline_to_current_frame
                    self.draw_timeline()

                    # Rolar a timeline para manter o indicador visível
                    self.scroll_timeline_to_current_frame()
                else:
                    # Vídeo terminou
                    print("Vídeo terminou.")
                    self.on_closing()
                    return
            else:
                # Tentar reabrir o vídeo se estiver fechado
                self.cap.open(self.video_path)
                self.cap.set(cv2.CAP_PROP_POS_FRAMES, self.current_frame_index)

            # Ajustar o intervalo de atualização com base na velocidade
            interval = int(1000 / (self.fps * self.playback_speed))
            self.after(interval, self.update_frame)
        else:
            # Se pausado ou em modo frame-by-frame, atualizar o frame atual
            self.display_current_frame()
            self.draw_timeline()
            self.after(50, self.update_frame)  # Intervalo reduzido para melhorar a resposta

    def resize_frame(self, frame):
        # Obter o tamanho do video_label
        label_width = self.video_label.winfo_width()
        label_height = self.video_label.winfo_height()

        if label_width > 1 and label_height > 1:
            # Redimensionar o frame para caber no video_label
            frame_height, frame_width, _ = frame.shape

            # Calcular a proporção para manter a proporção original
            ratio = min(label_width / frame_width, label_height / frame_height)
            new_width = int(frame_width * ratio)
            new_height = int(frame_height * ratio)

            # Redimensionar o frame
            frame = cv2.resize(frame, (new_width, new_height), interpolation=cv2.INTER_AREA)
        return frame

    def toggle_playback(self):
        if self.is_frame_by_frame:
            return  # Não alterar o estado de reprodução no modo frame-by-frame
        self.is_playing = not self.is_playing
        print("Playback:", "Playing" if self.is_playing else "Paused")

    def set_speed(self, speed):
        self.playback_speed = speed
        print(f"Velocidade de reprodução ajustada para {speed}x")

    def toggle_frame_by_frame_mode(self):
        self.is_frame_by_frame = not self.is_frame_by_frame
        if self.is_frame_by_frame:
            self.is_playing = False  # Pausar a reprodução
            self.prev_frame_button.config(state='normal')
            self.next_frame_button.config(state='normal')
            print("Modo Frame a Frame ativado")
        else:
            self.prev_frame_button.config(state='disabled')
            self.next_frame_button.config(state='disabled')
            print("Modo Frame a Frame desativado")

    def display_current_frame(self):
        self.cap.set(cv2.CAP_PROP_POS_FRAMES, self.current_frame_index)
        ret, frame = self.cap.read()
        if ret:
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

            # Calcular o tempo atual em segundos
            current_time_sec = self.current_frame_index / self.fps

            # Sobrepor o tempo no frame
            text = f"Tempo: {current_time_sec:.2f} s"
            cv2.putText(frame, text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX,
                        1, (255, 255, 255), 2, cv2.LINE_AA)

            # Redimensionar o frame
            frame = self.resize_frame(frame)

            img = Image.fromarray(frame)
            imgtk = ImageTk.PhotoImage(image=img)
            self.video_label.imgtk = imgtk
            self.video_label.configure(image=imgtk)

    def next_frame(self, event=None):
        if self.current_frame_index < self.total_frames - 1:
            self.current_frame_index += 1
            self.display_current_frame()
            self.draw_timeline()
            self.scroll_timeline_to_current_frame()

    def prev_frame(self, event=None):
        if self.current_frame_index > 0:
            self.current_frame_index -= 1
            self.display_current_frame()
            self.draw_timeline()
            self.scroll_timeline_to_current_frame()

    def start_holding_next(self, event):
        self.holding_next = True
        self.hold_next_frame()

    def stop_holding_next(self, event):
        self.holding_next = False

    def hold_next_frame(self):
        if self.holding_next:
            # Avançar mais frames para aumentar a velocidade
            for _ in range(3):  # Ajuste o número de frames para controlar a velocidade
                self.next_frame()
            self.update_idletasks()  # Forçar atualização da interface
            self.after(20, self.hold_next_frame)  # Intervalo reduzido para aumentar a velocidade

    def start_holding_prev(self, event):
        self.holding_prev = True
        self.hold_prev_frame()

    def stop_holding_prev(self, event):
        self.holding_prev = False

    def hold_prev_frame(self):
        if self.holding_prev:
            # Retroceder mais frames para aumentar a velocidade
            for _ in range(3):  # Ajuste o número de frames para controlar a velocidade
                self.prev_frame()
            self.update_idletasks()  # Forçar atualização da interface
            self.after(20, self.hold_prev_frame)  # Intervalo reduzido para aumentar a velocidade

    def on_timeline_click(self, event):
        # Calcular o frame correspondente ao local clicado
        x = self.timeline_canvas.canvasx(event.x)
        total_timeline_width = float(self.timeline_canvas.cget('scrollregion').split()[2])
        frame = int((x / total_timeline_width) * self.total_frames)
        frame = max(0, min(frame, self.total_frames - 1))
        self.current_frame_index = frame
        self.cap.set(cv2.CAP_PROP_POS_FRAMES, frame)
        print(f"Pulado para o frame {frame}")

    def start_region(self, event):
        # Se já existe uma região em seleção, remover o marcador anterior
        if self.selecting_region:
            if self.region_start_frame in self.region_start_markers:
                self.region_start_markers.remove(self.region_start_frame)
            self.selecting_region = False
            print("Região anterior não finalizada foi descartada.")

        self.selecting_region = True
        x = self.timeline_canvas.canvasx(event.x)
        total_timeline_width = float(self.timeline_canvas.cget('scrollregion').split()[2])
        self.region_start_frame = int((x / total_timeline_width) * self.total_frames)
        print(f"Região iniciada no frame {self.region_start_frame}")

        # Adicionar marcador na barra de progresso
        self.region_start_markers.append(self.region_start_frame)
        self.draw_timeline()

    def end_region(self, event):
        if self.selecting_region:
            x = self.timeline_canvas.canvasx(event.x)
            total_timeline_width = float(self.timeline_canvas.cget('scrollregion').split()[2])
            region_end_frame = int((x / total_timeline_width) * self.total_frames)
            label = self.ask_label()
            if label:
                region = {
                    'start': min(self.region_start_frame, region_end_frame),
                    'end': max(self.region_start_frame, region_end_frame),
                    'label': label
                }
                self.labels.append(region)
                print(f"Região de {region['start']} a {region['end']} com label '{label}'")
            self.selecting_region = False
            self.draw_timeline()

            # Remover marcador de início após definir a região
            if self.region_start_frame in self.region_start_markers:
                self.region_start_markers.remove(self.region_start_frame)
            self.region_start_frame = None

    def ask_label(self):
        # Criar uma nova janela Toplevel
        label_window = tk.Toplevel(self)
        label_window.title("Selecione a Label")

        # **Aguardar até que a janela seja visível**
        label_window.wait_visibility()

        # Tornar a janela modal
        label_window.grab_set()

        label_var = tk.StringVar(value=self.label_options[0])  # Valor padrão

        # Criar botões de opção para cada label
        for option in self.label_options:
            tk.Radiobutton(label_window, text=option, variable=label_var, value=option).pack(anchor='w')

        def on_ok():
            label_window.destroy()

        tk.Button(label_window, text="OK", command=on_ok).pack()

        # Aguardar o fechamento da janela
        self.wait_window(label_window)

        # Retornar a label selecionada
        return label_var.get()

    def draw_timeline(self):
        self.timeline_canvas.delete("all")
        timeline_width = self.timeline_canvas.winfo_width()
        scale = self.zoom_scale

        # Duração da janela visível (10s no zoom 1x)
        window_duration = 10 / scale  # Ajustar a duração da janela conforme o zoom
        total_timeline_width = (self.duration / window_duration) * timeline_width

        # Configurar a região de scroll
        self.timeline_canvas.config(scrollregion=(0, 0, total_timeline_width, 50))

        # Obter a posição de scroll atual
        x0 = self.timeline_canvas.canvasx(0)
        x1 = self.timeline_canvas.canvasx(timeline_width)

        # Calcular o intervalo das marcações (0.5s no zoom 1x)
        base_tick_interval = 0.5  # 0.5 segundos
        tick_interval = base_tick_interval / scale

        # Calcular o tempo inicial e final visíveis
        start_time = (x0 / total_timeline_width) * self.duration
        end_time = (x1 / total_timeline_width) * self.duration

        # Ajustar para evitar problemas de precisão
        start_time = max(0, start_time)
        end_time = min(self.duration, end_time)

        # Desenhar as marcações
        current_time = start_time - (start_time % tick_interval)
        while current_time <= end_time:
            x = (current_time / self.duration) * total_timeline_width
            self.timeline_canvas.create_line(x, 0, x, 10, fill='black')
            time_text = f"{current_time:.1f}s"
            self.timeline_canvas.create_text(x + 5, 20, text=time_text, anchor='w', fill='black')
            current_time += tick_interval

        # Desenhar os marcadores de início de região
        for start_frame in self.region_start_markers:
            x = (start_frame / self.total_frames) * total_timeline_width
            self.timeline_canvas.create_line(x, 0, x, 50, fill='green', dash=(4, 2))

        # Desenhar as regiões
        for region in self.labels:
            x1 = (region['start'] / self.total_frames) * total_timeline_width
            x2 = (region['end'] / self.total_frames) * total_timeline_width
            self.timeline_canvas.create_rectangle(x1, 30, x2, 50, fill='blue', stipple='gray25')
            self.timeline_canvas.create_text((x1 + x2) / 2, 40, text=region['label'], fill='white')

        # Desenhar o indicador de posição atual
        current_x = (self.current_frame_index / self.total_frames) * total_timeline_width
        self.timeline_canvas.create_line(current_x, 0, current_x, 50, fill='red')

    def scroll_timeline_to_current_frame(self):
        # Obter a largura total da timeline
        scrollregion = self.timeline_canvas.cget('scrollregion')
        if not scrollregion:
            return  # Scrollregion não está configurado ainda
        else:
            total_timeline_width = float(scrollregion.split()[2])
            # Calcular o x do frame atual
            current_x = (self.current_frame_index / self.total_frames) * total_timeline_width
            # Obter a largura da área visível
            visible_width = self.timeline_canvas.winfo_width()
            # Obter a posição de scroll atual (frações)
            x0, x1 = self.timeline_canvas.xview()
            # Converter frações para coordenadas de tela
            visible_start = x0 * total_timeline_width
            visible_end = x1 * total_timeline_width
            # Verificar se o current_x está fora da área visível
            if current_x < visible_start or current_x > visible_end:
                # Calcular a nova posição de scroll para centralizar o indicador
                new_x0 = (current_x - visible_width / 2) / total_timeline_width
                # Garantir que new_x0 esteja entre 0 e 1 - (visible_width / total_timeline_width)
                max_x0 = 1 - (visible_width / total_timeline_width)
                new_x0 = max(0, min(new_x0, max_x0))
                self.timeline_canvas.xview_moveto(new_x0)

    def show_time_on_timeline(self, event):
        # Remover o tempo anterior
        if self.time_display:
            self.timeline_canvas.delete(self.time_display)
            self.time_display = None

        # Calcular o tempo correspondente à posição do mouse
        x = self.timeline_canvas.canvasx(event.x)
        total_timeline_width = float(self.timeline_canvas.cget('scrollregion').split()[2])
        time_sec = (x / total_timeline_width) * self.duration
        time_sec = max(0, min(time_sec, self.duration))

        # Exibir o tempo na posição do mouse
        text = f"{time_sec:.2f} s"
        self.time_display = self.timeline_canvas.create_text(x, 10, text=text, fill='black', anchor='s')

    def zoom_timeline(self, event):
        if event.num == 4 or event.delta > 0:
            # Scroll up (Zoom in)
            self.zoom_scale *= 2  # Ajustar para dobrar o zoom
        elif event.num == 5 or event.delta < 0:
            # Scroll down (Zoom out)
            self.zoom_scale /= 2  # Ajustar para reduzir o zoom pela metade

        # Limitar o zoom entre 1x e 8x
        self.zoom_scale = max(1.0, min(self.zoom_scale, 8.0))
        print(f"Zoom da timeline: {self.zoom_scale}x")
        self.draw_timeline()

    def on_closing(self):
        # Salvar as labels antes de fechar
        self.save_labels_to_csv()
        self.destroy()

    def save_labels_to_csv(self):
        video_filename = os.path.splitext(os.path.basename(self.video_path))[0]
        csv_filename = f"{video_filename}_labels.csv"
        with open(csv_filename, 'w', newline='') as csvfile:
            fieldnames = ['start_frame', 'end_frame', 'start_time', 'end_time', 'label']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            for label in self.labels:
                start_frame = label['start']
                end_frame = label['end']
                start_time = start_frame / self.fps
                end_time = end_frame / self.fps
                writer.writerow({
                    'start_frame': start_frame,
                    'end_frame': end_frame,
                    'start_time': f"{start_time:.2f}",
                    'end_time': f"{end_time:.2f}",
                    'label': label['label']
                })
        print(f"Labels salvas em {csv_filename}")

if __name__ == "__main__":
    app = VideoLabeler()
    app.mainloop()
