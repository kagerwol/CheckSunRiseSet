# -*- coding: utf-8 -*-

import ephem
from datetime import datetime, timedelta
import pandas as pd
import pytz
import sys
import time

# Beobachterposition einstellen (z. B. Berlin)
observer = ephem.Observer()
#observer.lat, observer.lon = '52.5200', '13.4050'  # Breiten- und Längengrad von Berlin
#observer.lat, observer.lon = '47.582752519429604', '16.004253309523712' # Breiten und Laengengrad von Sankt Corona am Wechsel
observer.lat, observer.lon = '49.59749459760013',  '11.030420778017055' # Breiten und Laengengrad von Erlangen LHS7

# Zeitzone definieren
local_tz = pytz.timezone('Europe/Berlin')


def calculate_sun_times(start_date, num_days):
    data_print = []
    data_unix = []

    for i in range(num_days): 
        current_date = start_date + timedelta(days=i)
        observer.date = current_date

        sunrise_utc = observer.next_rising(ephem.Sun()).datetime()
        sunset_utc = observer.next_setting(ephem.Sun()).datetime()
        sunrise_local = pytz.utc.localize(sunrise_utc).astimezone(local_tz)
        sunset_local = pytz.utc.localize(sunset_utc).astimezone(local_tz)
        day_length = sunset_utc - sunrise_utc

        # Konvertiere in Unix-Timestamps
        sunrise_unix_ms = round(sunrise_utc.timestamp() * 1000)
        sunset_unix_ms = round(sunset_utc.timestamp() * 1000)
        day_length_ms = round(day_length.total_seconds() * 1000)
        data_print.append({
            "Date": current_date,
            "Sunrise (local)": sunrise_local.time(),
            "Sunset (local)": sunset_local.time(),
            "Day Length": str(day_length)
        })
        data_unix.append({
            "Date (unix ms)": current_date,
            "Sunrise (Unix ms)": sunrise_unix_ms,
            "Sunset (Unix ms)": sunset_unix_ms,
            "Day Length (ms)": day_length_ms
        })

    return pd.DataFrame(data_print), pd.DataFrame(data_unix)

def main():
    # Eingabedatum verarbeiten
    if len(sys.argv) > 1:
        try:
            input_date = datetime.strptime(sys.argv[1], "%Y-%m-%d").date()
        except ValueError:
            print("Ungültiges Datum. Bitte im Format YYYY-MM-DD eingeben.")
            return
    else:
        input_date = datetime.now().date()

    # Zeitraum berechnen
    start_date = input_date - timedelta(days=366)
    end_date = input_date + timedelta(days=366)

    # Sonnenzeiten berechnen
    dprint, dunix = calculate_sun_times(start_date, (end_date - start_date).days)

    # Ergebnisse anzeigen
    print(dprint)
    
    # Daten als CSV exportieren
    dunix.to_json("sun_times.json", orient="records")
    dunix.to_csv("sun_times.csv", index=False)
    print("Daten in 'sun_times.csv' gespeichert.")

if __name__ == "__main__":
    main()
