import json
from datetime import datetime, timedelta
from collections import defaultdict
import numpy as np
import pandas as pd

def process_imu_data(json_file, cutoff_time_str, cutoff_frame):
    """Processa os dados IMU, sincroniza com frames e lida com lacunas."""
    try:
        with open(json_file, 'r') as f:
            data = json.load(f)
    except (FileNotFoundError, json.JSONDecodeError) as e:
        raise Exception(f"Erro ao carregar JSON: {e}")

    imu_data = data.get("IMUData", {})
    cutoff_time = datetime.strptime(cutoff_time_str, "%H:%M:%S")
    sub_lists = defaultdict(list)

    for key, value in imu_data.items():
        for time_key, records in value.items():
            time = datetime.strptime(time_key, "%H:%M:%S")
            if time >= cutoff_time:
                if isinstance(records, (list, dict)):
                    sub_lists[time_key].extend(records.values() if isinstance(records, dict) else filter(None, records))

    missing_times = find_missing_times(sub_lists)
    if missing_times:
        print("Horários com lacunas encontrados:", missing_times)
    else:
        print("Nenhuma lacuna nos horários encontrada.")

    frame_data = create_frame_data(sub_lists, cutoff_time, cutoff_frame)
    return pd.DataFrame(frame_data)

def find_missing_times(sub_lists):
    """Encontra lacunas nos horários."""
    sorted_times = sorted(sub_lists.keys(), key=lambda x: datetime.strptime(x, '%H:%M:%S'))
    missing_times = []
    for i in range(len(sorted_times) - 1):
        current_time = datetime.strptime(sorted_times[i], '%H:%M:%S')
        next_time = datetime.strptime(sorted_times[i + 1], '%H:%M:%S')
        if next_time != current_time + timedelta(seconds=1):
            missing_range = [(current_time + timedelta(seconds=j)).strftime('%H:%M:%S') for j in range(1, (next_time - current_time).seconds)]
            missing_times.extend(missing_range)
    return missing_times

def create_frame_data(sub_lists, cutoff_time, cutoff_frame):
    """Cria dados formatados por frame."""
    current_frame = cutoff_frame
    frame_data = []
    last_time = None
    sorted_keys = sorted(sub_lists.keys(), key=lambda x: datetime.strptime(x, "%H:%M:%S"))

    for time_key in sorted_keys:
        current_time = datetime.strptime(time_key, "%H:%M:%S")
        if last_time is not None:
            time_diff = (current_time - last_time).total_seconds()
            if time_diff > 1:
                current_frame += int(time_diff - 1) * 30
        sub_lists_split = np.array_split(sub_lists[time_key], 30)
        for data_chunk in sub_lists_split:
            frame_data.append({'frame': current_frame, 'dados': data_chunk})
            current_frame += 1
        last_time = current_time
    return frame_data
