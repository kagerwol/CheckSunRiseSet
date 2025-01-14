#pragma once


const char GetSunRiseSetPyProgram[] = R"GetSunRiseSetPy(
# -*- coding: utf-8 -*-

import ephem
from datetime import datetime, timedelta, timezone
import pandas as pd
import pytz
import sys
import time

# Set observer position (e.g., Erlangen)
observer = ephem.Observer()
#observer.lat, observer.lon = '52.5200', '13.4050'  # Latitude and longitude of Berlin
#observer.lat, observer.lon = '47.582752519429604', '16.004253309523712' # Latitude and longitude of Sankt Corona am Wechsel
observer.lat, observer.lon = '49.59749459760013',  '11.030420778017055' # Latitude and longitude of Erlangen LHS7
#observer.lat, observer.lon = '49.6', '11.02' # Another Erlangen
#observer.lat, observer.lon = '60.99245734329251', '24.447981540937395' # Hämmeenlinna

# Define timezone
local_tz = pytz.timezone('UTC')

def calculate_sun_times(start_date: datetime, num_days: int) -> pd.DataFrame:
    """
    Calculates sunrise, sunset, and day length for a given range of dates.

    Parameters:
        start_date (datetime): Start date of the calculation.
        num_days (int): Number of days to calculate.

    Returns:
        pd.DataFrame: A DataFrame containing Unix timestamps and day lengths in milliseconds.
    """
    data_unix = []   # Data for Unix timestamp output

    for i in range(num_days): 
        current_date = start_date + timedelta(days=i)
        observer.date = current_date

        # Calculate sunrise and sunset in UTC and local time
        sunrise = observer.next_rising(ephem.Sun()).datetime()
        sunset = observer.next_setting(ephem.Sun()).datetime()
        sunrise_local = pytz.utc.localize(sunrise).astimezone(local_tz)
        sunset_local = pytz.utc.localize(sunset).astimezone(local_tz)
        sunrise_utc = pytz.utc.localize(sunrise).astimezone(timezone.utc)
        sunset_utc = pytz.utc.localize(sunset).astimezone(timezone.utc)
        day_length = sunset_utc - sunrise_utc

        # Convert to Unix timestamps
        sunrise_unix_ms = round(sunrise_utc.timestamp() * 1000)
        sunset_unix_ms = round(sunset_utc.timestamp() * 1000)
        day_length_ms = round(day_length.total_seconds() * 1000)

        # Append readable and Unix data
        data_unix.append({
            "Date (unix)": current_date,
            "Sunrise (Unix ms)": sunrise_unix_ms,
            "Sunset (Unix ms)": sunset_unix_ms,
            "Day Length (ms)": day_length_ms,
            "Sunrise (local)": sunrise_local.time(),
            "Sunset (local)": sunset_local.time(),
            "Day Length": str(day_length)
        })

    return pd.DataFrame(data_unix)

def main() -> None:
    """
    Main function of the script.
    Processes input data, calculates sun times, and outputs the results.
    """
    # Process input date
    if len(sys.argv) > 1:
        try:
            input_date = datetime.strptime(sys.argv[1], "%Y-%m-%d").date()
        except ValueError:
            print("Invalid date. Please use the format YYYY-MM-DD.")
            return
    else:
        input_date = datetime.now().date()

    # Calculate date range
    start_date = input_date - timedelta(days=366)
    end_date = input_date + timedelta(days=366)

    # Calculate sun times
    dunix = calculate_sun_times(start_date, (end_date - start_date).days)

    # Output input date
    print(input_date)
    
    # Output results directly to stdout
    for _, row in dunix.iterrows():
         print(f"{row['Date (unix)']}, {row['Sunrise (Unix ms)']}, {row['Sunset (Unix ms)']}, {row['Day Length (ms)']}")

if __name__ == "__main__":
    main()
)GetSunRiseSetPy";