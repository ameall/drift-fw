import serial


class GPS:
    def __init__(self, port):
        self.port = port
        self.baudrate = 115200
        self.timeout = 1
        self.serial = serial.Serial(self.port, self.baudrate, timeout=self.timeout)

    def get_gps_data(self):
        self.serial.write("?".encode('utf-8'))
        line = self.serial.readline().decode('utf-8', errors='ignore').strip()
        print(line)
        try: 
            if line.startswith('$GPGGA'):
                data = line.split(',')
                if len(data) >= 6:
                    self.time = data[1]
                    self.latitude = data[2]
                    degrees = int(self.latitude[:len(self.latitude)-8])  # Extract degrees
                    minutes = float(self.latitude[len(self.latitude)-8:])
                    print(degrees, minutes)
                    self.latitude = degrees + (minutes / 60)
                    self.nw_direction = data[3]
                    if self.nw_direction == 'S':
                        self.latitude = -self.latitude
                    self.longitude = data[4]
                    degrees = int(self.longitude[:len(self.longitude)-8])  # Extract degrees
                    minutes = float(self.longitude[len(self.longitude)-8:])
                    self.longitude = degrees + (minutes / 60)
                    self.ew_direction = data[5]
                    if self.ew_direction == 'W':
                        self.longitude = -self.longitude
                    self.fix_quality = data[6]
                    self.num_satellites = data[7]
                    self.hdop = data[8]
                    self.altitude = data[9] if len(data) > 9 else None
                    self.rssi = data[13]
                    # print(len(data))
                    # self,
        except:
            print("Error parsing GPS data")
        return self.latitude, self.longitude, self.altitude
                

if __name__ == "__main__":
    gps = GPS('COM4')  # Replace with your GPS port
    try:
        # while True:
        gps.get_gps_data()
        print(f"Latitude: {gps.latitude}, Longitude: {gps.longitude}, Altitude: {gps.altitude}")
    except KeyboardInterrupt:
        gps.serial.close()
        print("GPS data fetching stopped.")