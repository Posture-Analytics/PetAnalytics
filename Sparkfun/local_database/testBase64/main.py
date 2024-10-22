import base64_decoder
import pandas as pd

# Example data dictionary, with a 5-char timestamp and concatenated data blocks
data_dict = {
    "2024-23-03": {
        "AJlv_": 'AMDnPx3_-MDnPx3_-MDnPx3_-BMDnPx3_-MDnPx3_-MDnPx3_-CMDnPx3_-MDnPx3_-MDnPx3_-DMDnPx3_-MDnPx3_-MDnPx3_-',
        "BJlv_": 'AMDnPx3_-MDnPx3_-MDnPx3_-BMDnPx3_-MDnPx3_-MDnPx3_-CMDnPx3_-MDnPx3_-MDnPx3_-DMDnPx3_-MDnPx3_-MDnPx3_-'
    },
    "2024-24-03": {
        "CJlv_": 'AMDnPx3_-MDnPx3_-MDnPx3_-BMDnPx3_-MDnPx3_-MDnPx3_-CMDnPx3_-MDnPx3_-MDnPx3_-DMDnPx3_-MDnPx3_-MDnPx3_-',
        "DJlv_": 'AMDnPx3_-MDnPx3_-MDnPx3_-BMDnPx3_-MDnPx3_-MDnPx3_-CMDnPx3_-MDnPx3_-MDnPx3_-DMDnPx3_-MDnPx3_-MDnPx3_-'
    }
}

# Decoding the data dictionary
decoded_data = base64_decoder.decode_data(data_dict)
print(decoded_data)