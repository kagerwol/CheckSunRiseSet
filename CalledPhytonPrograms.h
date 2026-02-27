#pragma once


const char GetSunRiseSetPyProgram[] = R"GetSunRiseSetPy(
# -*- coding: utf-8 -*-

import ephem
from datetime import datetime, timedelta, timezone
import pandas as pd
import pytz
import sys
import time
import re

# Set default observer position (e.g., Erlangen)
# These defaults are used if no latitude/longitude are passed as arguments.
observer = ephem.Observer()
#observer.lat, observer.lon = '52.5200', '13.4050'  # Latitude and longitude of Berlin
#observer.lat, observer.lon = '47.582752519429604', '16.004253309523712' # Latitude and longitude of Sankt Corona am Wechsel
observer.lat, observer.lon = '49.59749459760013',  '11.030420778017055' # Latitude and longitude of Erlangen LHS7
#observer.lat, observer.lon = '49.6', '11.02' # Another Erlangen
#observer.lat, observer.lon = '60.99245734329251', '24.447981540937395' # Hämmeenlinna
#observer.lat, observer.lon = '41.14699766775208', '123.02513400392203' #Anshan China

# Define timezone
local_tz = datetime.now().astimezone().tzinfo  # lokale PC-Zeitzone (Regeln inkl. DST)
unitc_tz = pytz.timezone('UTC')

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
        local_midnight = datetime(
          current_date.year,
          current_date.month,
          current_date.day,
          0, 0, 0,
          tzinfo=local_tz
        )
        utc_midnight = local_midnight.astimezone(timezone.utc)
        # Use current_date as the calendar day we want results for.
        # ephem expects a date/time; using the date is acceptable (treated as midnight UTC).
        observer.date = utc_midnight

        # Get next rising/setting from observer.date.
        # Wrap in try/except to handle polar day/night cases robustly.
        try:
            sunrise_dt = observer.next_rising(ephem.Sun()).datetime()
            sunset_dt = observer.next_setting(ephem.Sun()).datetime()
        except ephem.AlwaysUpError:
            # Polar day: sun never sets -> full 24h daylight
            sunrise_dt = datetime(current_date.year, current_date.month, current_date.day, 0, 0, 0)
            sunset_dt = sunrise_dt + timedelta(days=1)   # treat as 24:00 (next day's 00:00)
        except ephem.NeverUpError:
            # Polar night: sun never rises -> 0h daylight
            sunrise_dt = datetime(current_date.year, current_date.month, current_date.day, 0, 0, 0)
            sunset_dt = sunrise_dt                        # same moment -> zero length

        # Treat ephem returned datetimes as naive UTC instants; localize to UTC for conversions
        sunrise_utc = pytz.utc.localize(sunrise_dt).astimezone(timezone.utc)
        sunset_utc = pytz.utc.localize(sunset_dt).astimezone(timezone.utc)

        # For readable local times convert from UTC to unitc_tz
        sunrise_local = sunrise_utc.astimezone(unitc_tz)
        sunset_local = sunset_utc.astimezone(unitc_tz)

        # Mark if sunrise is from previous day or sunset is from next day (relative to calendar day)
        sunrise_marker = " "
        sunset_marker = " "
        if sunrise_utc.date() < current_date:
            sunrise_marker = "P"
        if sunset_utc.date() > current_date:
            sunset_marker = "N"

        # Compute day length:
        # - If sunset_utc >= sunrise_utc, normal interval
        # - If sunset_utc < sunrise_utc, interpret as: daylight = midnight->sunset + sunrise->midnight_next_day
        midnight = datetime(current_date.year, current_date.month, current_date.day, 0, 0, 0, tzinfo=timezone.utc)
        next_midnight = midnight + timedelta(days=1)

        if sunset_utc >= sunrise_utc:
            day_length = sunset_utc - sunrise_utc
        else:
            # Do not shift sunset_utc; compute split interval across midnight
            # midnight and next_midnight are timezone-aware UTC datetimes; ensure sunrise_utc/sunset_utc are UTC-aware too
            # (they are timezone-aware above)
            day_part1 = max(timedelta(0), sunset_utc - midnight)         # midnight -> sunset
            day_part2 = max(timedelta(0), next_midnight - sunrise_utc)   # sunrise -> next midnight
            day_length = day_part1 + day_part2

        sunrise_unix_ms = round(sunrise_utc.timestamp() * 1000)
        sunset_unix_ms = round(sunset_utc.timestamp() * 1000)
        day_length_ms = round(day_length.total_seconds() * 1000)

        data_unix.append({
            "Date (unix)": current_date,
            "Sunrise (Unix ms)": f"{sunrise_unix_ms}{sunrise_marker}",
            "Sunset (Unix ms)": f"{sunset_unix_ms}{sunset_marker}",
            "Day Length (ms)": day_length_ms,
            "Sunrise (local)": sunrise_local.time(),
            "Sunset (local)": sunset_local.time(),
            "Day Length": str(day_length)
        })

    return pd.DataFrame(data_unix)

def _is_date_string(s: str) -> bool:
    """Return True if s matches YYYY-MM-DD."""
    return re.match(r'^\d{4}-\d{2}-\d{2}$', s) is not None

def _is_float_like(s: str) -> bool:
    """Return True if s can be interpreted as a float (latitude/longitude)."""
    try:
        float(s)
        return True
    except ValueError:
        return False

def main() -> None:
    """
    Main function of the script.
    Usage:
      python script.py [YYYY-MM-DD] [latitude longitude]
    - If a date is provided it must be YYYY-MM-DD.
    - If latitude and longitude are provided they override the defaults.
    Examples:
      python script.py 2024-02-20
      python script.py 2024-02-20 49.5974946 11.0304208
      python script.py 49.5974946 11.0304208   # date omitted -> uses today
    """
    # Parse command-line arguments permissively:
    # print("Arguments received:", sys.argv)
    args = sys.argv[1:]

    input_date = None
    lat_arg = None
    lon_arg = None

    # Extract date and numeric arguments from args in any reasonable order
    for a in args:
        # print(f"Processing argument: {a}")
        if _is_date_string(a) and input_date is None:
            try:
                input_date = datetime.strptime(a, "%Y-%m-%d").date()
                #print(f"Parsed date: {input_date}")
            except ValueError:
                # malformed date; ignore and continue
                #print(f"Argument '{a}' looks like a date but is not valid. Ignoring.")
                input_date = None
        elif _is_float_like(a):
            if lat_arg is None:
                lat_arg = a
            elif lon_arg is None:
                lon_arg = a
            else:
                # ignore extra numeric args
                pass
        elif a.startswith("--lat=") and lat_arg is None:
            lat_arg = a.split("=", 1)[1]
        elif a.startswith("--lon=") and lon_arg is None:
            lon_arg = a.split("=", 1)[1]
        # other flags can be added here
        #print(f"Current parsed values: date={input_date}, lat={lat_arg}, lon={lon_arg}")

    if input_date is None:
        input_date = datetime.now().date()

    # If both latitude and longitude provided, override the defaults
    if (lat_arg is not None) and (lon_arg is not None):
        # ephem expects strings for lat/lon; preserve sign/format
        observer.lat = str(lat_arg)
        observer.lon = str(lon_arg)
    elif (lat_arg is not None) or (lon_arg is not None):
        # partially provided coordinates are ambiguous; report and exit
        print("If you supply coordinates, supply both latitude and longitude.")
        return

    # Calculate date range: one year back/forward
    start_date = input_date - timedelta(days=366)
    end_date = input_date + timedelta(days=366)

    # Calculate sun times
    dunix = calculate_sun_times(start_date, (end_date - start_date).days)

    # Output input date (first line)
    print(input_date)

    # Output results directly to stdout (one CSV-like line per day)
    for _, row in dunix.iterrows():
         print(f"{row['Date (unix)']}, {row['Sunrise (Unix ms)']}, {row['Sunset (Unix ms)']}, {row['Day Length (ms)']}")

if __name__ == "__main__":
    main()
)GetSunRiseSetPy";