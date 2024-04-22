from datetime import datetime
import pandas as pd

# Custom Base64 decoding based on the C++ implementation provided
BASE64_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
BASE64_DECODE_MAP = {v: i for i, v in enumerate(BASE64_ALPHABET)}

GMT_OFFSET = -3 * 60 * 60 * 1000

def decode_base64_to_bytes(input_str):
        """
        Decodes an 8-character Base64 string into 6 bytes.
        
        :param input_str: An 8-character Base64 encoded string.
        :return: A list of 6 bytes.
        """
        bytes_list = []
        buffer = 0
        bit_position = 0
        for char in input_str:
            value = BASE64_DECODE_MAP.get(char)
            if value is None:
                raise ValueError(f"Invalid Base64 character: {char}")

            buffer = (buffer << 6) | value
            bit_position += 6
            if bit_position >= 8:
                bit_position -= 8
                bytes_list.append((buffer >> bit_position) & 0xFF)
        
        return bytes_list

def decode_sensor_data(input_str):
        """
        Decodes an 8-character Base64 string into three sensor data values.
        
        :param input_str: An 8-character Base64 encoded string.
        :return: A list containing three 16-bit integers.
        """
        bytes_list = decode_base64_to_bytes(input_str)
        
        # Function to convert 2 bytes into a signed 16-bit integer
        def bytes_to_int16(msb, lsb):
            value = (msb << 8) | lsb
            # Check if the sign bit is set (negative number)
            if value & 0x8000:
                value -= 0x10000  # Adjust for two's complement
            return value

        sensor_data = [
            bytes_to_int16(bytes_list[0], bytes_list[1]),
            bytes_to_int16(bytes_list[2], bytes_list[3]),
            bytes_to_int16(bytes_list[4], bytes_list[5]),
        ]

        return sensor_data

def decode_timestamp(input_str):
    """
    Decodes a 5-character Base64 string into a timestamp.
    
    :param input_str: A 5-character Base64 encoded string.
    :return: The decoded timestamp as an integer.
    """
    timestamp = 0

    # Decode each Base64 character to bits and add to the timestamp
    for char in input_str:
        value = BASE64_DECODE_MAP.get(char)
        if value is None:
            raise ValueError(f"Invalid Base64 character: {char}")
        timestamp = (timestamp << 6) | value

    return timestamp

def decode_id_and_delta_time(input_char):
    """
    Decodes a single Base64 character into an ID and deltaTime.
    
    :param input_char: A single Base64 encoded character.
    :return: A tuple containing the ID and deltaTime.
    """
    value = BASE64_DECODE_MAP.get(input_char)
    if value is None:
        raise ValueError(f"Invalid Base64 character: {input_char}")

    # Extract the ID (upper 2 bits) and deltaTime (lower 4 bits) from the value
    id_ = (value >> 4) & 0x03
    delta_time = value & 0x0F

    return id_, delta_time

def decode_data_block(block_key, block_value):
    
    # Assert if the block_key has a length of 5
    assert len(block_key) == 5, "Invalid block key length"

    # Assert if the block_value has a length multiple of 25
    assert len(block_value) % 25 == 0, "Invalid block value length"

    # Decode the block_key into the timestamp_offset
    timestamp_offset = decode_timestamp(block_key)

    # Split the block_value into 25-character data blocks
    data_blocks = [block_value[i:i+25] for i in range(0, len(block_value), 25)]

    # Iterate through each data block
    decoded_data_block = []
    for data_block in data_blocks:
        # Split the data block into the sensor_id and delta_time char, and the sensor_data blocks
        encoded_id_and_delta_time = data_block[0]
        encoded_accel_data = data_block[1:9]
        encoded_gyro_data = data_block[9:17]
        encoded_mag_data = data_block[17:]

        # Decode the sensor_id and delta_time char
        sensor_id, delta_time = decode_id_and_delta_time(encoded_id_and_delta_time)

        # Decode the sensor data blocks
        decoded_accel_data = decode_sensor_data(encoded_accel_data)
        decoded_gyro_data = decode_sensor_data(encoded_gyro_data)
        decoded_mag_data = decode_sensor_data(encoded_mag_data)

        # Create a dictionary to store the decoded sample
        decoded_sample = dict()

        # Update the timestamp_offset with the delta_time
        timestamp_offset += delta_time

        # Fill the dictionary with the decoded data
        decoded_sample['timestamp'] = timestamp_offset
        decoded_sample['sensor_id'] = sensor_id
        decoded_sample['accel_data'] = decoded_accel_data
        decoded_sample['gyro_data'] = decoded_gyro_data
        decoded_sample['mag_data'] = decoded_mag_data

        # Append the decoded sample to the decoded_data_block
        decoded_data_block.append(decoded_sample)

    return decoded_data_block

def decode_node(data_dict):
    decoded_data = []
    for key, value in data_dict.items():
        # Decode and concatenate the decoded data blocks
        decoded_data += decode_data_block(key, value)
    return decoded_data

def decode_data(data_dict):
    decode_data = []
    for key, value in data_dict.items():
        
        # Convert the day into a millis timestamp
        date = datetime.strptime(key, "%Y-%d-%m")
        timestamp = date.timestamp() * 1000 + GMT_OFFSET

        # Decode the node data
        decoded_node = decode_node(value)

        # Add the timestamp to each decoded sample
        for sample in decoded_node:
            sample['timestamp'] = int(timestamp + sample['timestamp'])

        # Append the decoded node data to the decoded_data
        decode_data += decoded_node

    # Convert the decoded data into a DataFrame
    df = pd.DataFrame(decode_data)

    # Sort the DataFrame by timestamp
    df = df.sort_values(by='timestamp').reset_index(drop=True)

    # Split the sensor data into separate columns (X, Y, Z)
    df[['accel_x', 'accel_y', 'accel_z']] = pd.DataFrame(df['accel_data'].tolist(), index=df.index)
    df[['gyro_x', 'gyro_y', 'gyro_z']] = pd.DataFrame(df['gyro_data'].tolist(), index=df.index)
    df[['mag_x', 'mag_y', 'mag_z']] = pd.DataFrame(df['mag_data'].tolist(), index=df.index)

    # Remove the original sensor data columns
    df = df.drop(columns=['accel_data', 'gyro_data', 'mag_data'])

    # Clone the timestamp column to create a new column for the date
    df['date'] = pd.to_datetime(df['timestamp'], unit='ms')

    # Reorder the columns
    df = df[['date', 'timestamp', 'sensor_id', 'accel_x', 'accel_y', 'accel_z', 'gyro_x', 'gyro_y', 'gyro_z', 'mag_x', 'mag_y', 'mag_z']]

    return df
