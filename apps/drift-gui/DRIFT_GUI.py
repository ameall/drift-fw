from re import M
import sys
import os
import csv
from telnetlib import SE
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QLabel, QPushButton, QStackedWidget, QRubberBand, QHBoxLayout
from PyQt5.QtWebEngineWidgets import QWebEngineView
from PyQt5.QtGui import QPixmap, QIcon, QImage
from PyQt5.QtCore import Qt, QRect, QTimer, QSize
from PIL import Image, ImageDraw
# from pyqtgraph.dockarea import *
import folium 
import requests 

REMOTE_DIR = "C:\\Users\\chamo\\OneDrive\\Desktop\\Pic" # Update with actual directory
LOG_DIR = "C:\\Users\\chamo\\OneDrive\\Desktop\\Logs" # Update with actual directory
COORDINATE_URL = "http:\\raspberrypi.local\\coordinates"  # Update with actual coordinates
TEMP = "C:\\Users\\chamo\\OneDrive\\Desktop\\Pic\\tmp" # Intermediate directory for image processing

# UDP or TCP connection to the drone


class MainApp(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()
    
    def initUI(self):
        self.layout = QVBoxLayout()
        
        self.stack = QStackedWidget(self)
        self.image_viewer = ImageViewer(self)
        self.map_viewer = MapWidget(self)
        self.entry_viewer = EntryView(self)
        
        self.stack.addWidget(self.image_viewer)
        self.stack.addWidget(self.map_viewer)
        self.stack.addWidget(self.entry_viewer)
        
        self.layout.addWidget(self.stack)
        self.setLayout(self.layout)
        
        self.setStyleSheet("background-color: #1E1E2E; color: #D8DEE9; font-size: 18px; font-family: 'Arial';")
        self.setWindowTitle("Drift Camera and Tracker")
        self.setGeometry(100, 100, 800, 600)

        
        self.show_entry_view()

        #use this for the opening screen
    def show_entry_view(self):
        self.stack.setCurrentWidget(self.entry_viewer)
    
    def show_image_view(self):
        self.stack.setCurrentWidget(self.image_viewer)
    
    def show_map_view(self):
        self.map_viewer.initiate()
        self.stack.setCurrentWidget(self.map_viewer)
    
    def close_application(self):
        self.close()

class EntryView(QWidget):
    def __init__(self, main_app):
        super().__init__()
        self.main_app = main_app
        self.initUI()
    
    def initUI(self):
        layout = QVBoxLayout()
        self.setLayout(layout)

        # add a graphic here as well

        # Enter program button 
        self.start_button = QPushButton("Enter DRIFT", self)
        self.start_button.clicked.connect(self.main_app.show_image_view)
        self.start_button.setStyleSheet("background-color: #5E81AC; color: white; padding: 12px; border-radius: 5px; font: bold 20px;")
        self.start_button.setIcon(QIcon("confirm.png"))
        layout.addWidget(self.start_button)


        # Close program button
        self.close_button = QPushButton("❌ Exit", self)
        self.close_button.setStyleSheet("background-color: #D31A38; color: white; padding: 10px; border-radius: 5px;")
        self.close_button.setGeometry(100, 100, 100, 100)
        self.close_button.clicked.connect(self.main_app.close_application)
        layout.addWidget(self.close_button)


class ImageViewer(QWidget):
    def __init__(self, main_app):
        super().__init__()
        self.main_app = main_app
        self.image = QPixmap()
        self.initUI()
    
    def initUI(self):
        layout = QVBoxLayout()
        
        # upperlayout
        self.image_label = QLabel(self)
        self.image_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.image_label)
        self.rubber_band = QRubberBand(QRubberBand.Rectangle, self)
        self.origin = None

        # lower layout
        lowerLayout = QHBoxLayout()

        lowerLeft = QVBoxLayout()

        # Switch to map button 
        self.map_button = QPushButton("Drop Payload", self)
        self.map_button.clicked.connect(self.main_app.show_map_view)
        self.map_button.setStyleSheet("background-color: #5E81AC; color: white; padding: 10px; border-radius: 5px;")
        self.map_button.setEnabled(False)
        # self.close_button.clicked.connect(self.open_map)
        lowerLeft.addWidget(self.map_button)


        # Fetch and display first image button
        self.fetch_button = QPushButton("Fetch Images", self)
        self.fetch_button.clicked.connect(self.Begin)
        self.fetch_button.setStyleSheet("background-color: #5E81AC; color: white; padding: 10px; border-radius: 5px;")
        lowerLeft.addWidget(self.fetch_button)

        # Manually cycle thru images button
        self.next_button = QPushButton("Next Image", self)
        self.next_button.setEnabled(False)
        self.next_button.setStyleSheet("background-color: #5E81AC; color: white; padding: 10px; border-radius: 5px;")
        self.next_button.clicked.connect(self.load_images)
        lowerLeft.addWidget(self.next_button)

        # Close button
        self.close_button = QPushButton("❌ Close Program", self)
        self.close_button.setStyleSheet("background-color: #D31A38; color: white; padding: 10px; border-radius: 5px;")
        self.close_button.clicked.connect(self.main_app.close_application)
        lowerLeft.addWidget(self.close_button)

        # Image Selection Preview
        lowerRight = QVBoxLayout()
        self.selection_label = QLabel("Selected Area Preview", self)
        self.selection_label.setFixedSize(200, 200)
        self.selection_label.setStyleSheet("border: 2px solid #5E81AC; background-color: #3B4252;")
        self.selection_label.setAlignment(Qt.AlignCenter)
        lowerRight.addWidget(self.selection_label, alignment=Qt.AlignCenter)

        # Send selection to drone button
        self.sendto = QPushButton("Send Selection to Drone", self)
        self.sendto.setStyleSheet("background-color: #5E81AC; color: white; padding: 10px; border-radius: 5px;")
        self.sendto.clicked.connect(self.return_to_sender)
        lowerRight.addWidget(self.sendto)


        lowerLayout.addLayout(lowerLeft)
        lowerLayout.addLayout(lowerRight)

        layout.addLayout(lowerLayout)
        
        self.setLayout(layout)
        
        
    def Begin(self):
        # Load logfile inputs
        self.log_list = self.get_logs()
        self.log_list = self.fixlogs(self.log_list)

        # Load first available image
        self.image_list = self.get_images()
        print(self.image_list)
        self.image_index = 0
        self.display_image()

        # enable buttons
        self.next_button.setEnabled(True)
        self.map_button.setEnabled(True)

        # auto cycle through images
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.next_image)
        self.timer.start(100)

        #  left x y, width height
    def fixlogs(self, log_list):
        for key, value_list in log_list.items():
            for i, values in enumerate(value_list):
                if isinstance(values, list):
                    values[2] = values[0] + values[2]
                    values[3] = values[1] + values[3] 
        return log_list

    def load_images(self):
        if self.image_list:
            self.image_index = (self.image_index + 1) % len(self.image_list)
            self.display_image()

    def get_logs(self):
        # Get log coordinates
        if os.path.exists(LOG_DIR):
            for file in os.listdir(LOG_DIR):
                if file.endswith(".csv"):
                    csv_path = os.path.join(LOG_DIR, file)
                    data = self.logdict(csv_path)
                    return data
        return []

    def logdict(self, path):
        coord_dict = {}
        with open(path, newline='') as csvfile:
            reader = csv.reader(csvfile)
            for row in reader:
                if row:  
                    try:
                        key = float(row[0])  
                        values = list(map(float, row[1:]))
                        if key in coord_dict:
                            coord_dict[key].append(values)
                        else:
                            coord_dict[key] = [values]
                    except ValueError as e:
                        print(f"Skipping invalid row {row}: {e}")
        return coord_dict         
            
    def get_images(self):
        # Get jpg files
        if os.path.exists(REMOTE_DIR):
            return [os.path.join(REMOTE_DIR, file) for file in os.listdir(REMOTE_DIR) if file.endswith(".jpg")]
        return []

  # def sort_all(self, image_path, log_path)


    def draw_bound(self, img):
        draw = ImageDraw.Draw(img)
        try:
            for values in self.log_list[self.image_index]:
                x = values[0]
                y = values[1]
                z = values[2]
                w = values[3]
                draw.rectangle((x, y, z, w), width = 3, outline=(57, 255, 20))
            return img
        except KeyError:
            return img

    def resize_Image(self, image_path):
        img = Image.open(image_path)
        img = img.convert("RGB")
        img.thumbnail((416,416))
        img = self.draw_bound(img)
        img.save(TEMP + "\\" + os.path.basename(image_path))
        return TEMP + "\\" + os.path.basename(image_path)


    def display_image(self):
        if self.image_list:
            img_path = self.resize_Image(self.image_list[self.image_index])
            pixmap = QPixmap(img_path)
            self.image_label.setPixmap(pixmap)

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.origin = event.pos()
            self.rubber_band.setGeometry(QRect(self.origin, QSize())) 
            self.rubber_band.show()
    
    def mouseMoveEvent(self, event):
        if self.rubber_band.isVisible():
            self.rubber_band.setGeometry(QRect(self.origin, event.pos()).normalized())
    
    def mouseReleaseEvent(self, event):
        if event.button() == Qt.LeftButton:
            rect = self.rubber_band.geometry()
            self.capture_selected_area(rect)
            self.rubber_band.hide()

    def next_image(self):
        if self.image_list:
            if self.image_index < len(self.image_list) - 1:
                self.image_index += 1
                self.display_image()
            else:
                self.timer.stop()
    
    def capture_selected_area(self, rect):
        selected_area = self.image_label.grab(rect)
        self.selection_label.setPixmap(selected_area.scaled(200, 200, Qt.KeepAspectRatio, Qt.SmoothTransformation))

    def return_to_sender(self):
        # Send selected area back to camera app CV
        # figure out

        print("Message Sent to Drone")



class MapWidget(QWidget):   
    def __init__(self, main_app):
        super(MapWidget, self).__init__(main_app)
        self.main_app = main_app

        self.setWindowTitle("Drift Camera and Tracker")
        self.web_view = QWebEngineView()
        self.web_view.setFixedSize(800, 500)
        layout = QVBoxLayout(self)
        layout.addWidget(self.web_view)

        # Go back to Image Viewer
        self.image_button = QPushButton("Return to Images", self)
        self.image_button.clicked.connect(self.main_app.show_image_view)
        self.image_button.setStyleSheet("background-color: #5E81AC; color: white; padding: 10px; border-radius: 5px;")
        layout.addWidget(self.image_button)

        # Refress button
        self.refresh_button = QPushButton("🔄 Refresh Map", self)
        self.refresh_button.setStyleSheet("background-color: #5E81AC; color: white; padding: 10px; border-radius: 5px;")
        self.refresh_button.clicked.connect(self.refresh_map)
        layout.addWidget(self.refresh_button)
        
        # Close button
        self.close_button = QPushButton("❌ Close Program", self)
        self.close_button.setStyleSheet("background-color: #D31A38; color: white; padding: 10px; border-radius: 5px;")
        self.close_button.clicked.connect(self.main_app.close_application)
        layout.addWidget(self.close_button)

        #self.pause_refresh = QPushButton("Pause Refresh", self)
        #self.pause_refresh.clicked.connect()

        # auto refresh every 10 seconds
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.refresh_map)
        self.timer.start(10000)

    def initiate(self):
        try:
            #response = requests.get(COORDINATE_URL)
            #if response.status_code == 200:
                #coordinates = response.json()
                #lati, longi = coordinates['latitude'], coordinates['longitude']
                longi = -71.05977000
                lati = 42.35843000
               
                m = folium.Map(location=[lati, longi], zoom_start=18)
                folium.Marker([lati, longi], popup="TARGET").add_to(m)
                self.web_view.setHtml(m._repr_html_())
                
            #else:
                #self.web_view.setHtml("<h3>Failed to fetch coordinates</h3>")
        except Exception as e:
            self.web_view.setHtml(f"<h3>Error: {str(e)}</h3>")

       
    
    def refresh_map(self):
        try:
            #response = requests.get(COORDINATE_URL)
            #if response.status_code == 200:
                #coordinates = response.json()
                #lati, longi = coordinates['latitude'], coordinates['longitude']
                longi = -71.05977000
                lati = 42.35843000

                m = folium.Map(location=[lati, longi], zoom_start=18)
                folium.Marker([lati, longi], popup="TARGET").add_to(m)
                self.web_view.setHtml(m._repr_html_())
                
            #else:
                #self.web_view.setHtml("<h3>Failed to fetch coordinates</h3>")
        except Exception as e:
            self.web_view.setHtml(f"<h3>Error: {str(e)}</h3>")



if __name__ == '__main__':

    ''' # to delete files in temp folder
    folder = TEMP
    for filename in os.listdir(folder):
        file_path = os.path.join(folder, filename)
        try:
            if os.path.isfile(file_path) or os.path.islink(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)
        except Exception as e:
            print('Failed to delete %s. Reason: %s' % (file_path, e))
    '''
    app = QApplication(sys.argv)
    main_app = MainApp()
    main_app.show()
    sys.exit(app.exec_())
