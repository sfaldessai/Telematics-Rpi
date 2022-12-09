import can_decoder
import pandas as pd
from datetime import datetime, timedelta, timezone
import random as rd
from jproperties import Properties
import warnings
import models

def generate_can_data():
    warnings.simplefilter(action='ignore', category=FutureWarning)
    configs=Properties()
    with open('config.properties', 'rb') as config_file:
        configs.load(config_file)
    drive_duration = int(configs.get("DriveDurationInMinutes").data)
    data_interval = int(configs.get("DataIntervalInSeconds").data)
    num_of_records = int((drive_duration * 60)/data_interval)
    timestamp = datetime.now(timezone.utc)
    dataset = pd.DataFrame(columns=['TimeStamp', 'VIN', 'MFG_Date' , 'EngineRPM', 'VehicleSpeed', 'FuelSystemStatus',
                                'EngineCoolantTemp', 'ShortTermFuelTrim', 'LongTermFuelTrim', 'IntakeAirTemperature',
                                    'ThrottlePosition', 'OxygenSensor', 'RelativeThrottlePosition',
                                    'AmbientAirTemperature', 'AbsoluteThrottlePosition'])
    vin = 'MA3NFG81SNA103948'
    mfg_date = str(models.Month[vin[10]].value)+'-'+str(models.Year[vin[9]].value)

    # Signals
    signal_engine = can_decoder.Signal(
        signal_name="EngineRPM",
        signal_start_bit=24,
        signal_size=16,
        signal_factor=1,
        signal_offset=0.25,
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_speed = can_decoder.Signal(
        signal_name="VehicleSpeed",
        signal_start_bit=24,
        signal_size=8,
        signal_factor=1,
        signal_offset=0,
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_fuel_system_status = can_decoder.Signal(
        signal_name="FuelSystemStatus",
        signal_start_bit=24,
        signal_size=16,
        signal_factor=1,
        signal_offset=0,
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_engine_load = can_decoder.Signal(
        signal_name="EngineLoad",
        signal_start_bit=24,
        signal_size=8,
        signal_factor=(1 / 2.55),
        signal_offset=0,
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_engine_coolant_temp = can_decoder.Signal(
        signal_name="EngineCoolantTemp",
        signal_start_bit=24,
        signal_size=8,
        signal_factor=1,
        signal_offset=(-40),
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_short_term_fuel_trim = can_decoder.Signal(
        signal_name="ShotTermFuelTrim",
        signal_start_bit=24,
        signal_size=8,
        signal_factor=(1/1.28),
        signal_offset=(-100),
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_long_term_fuel_trim = can_decoder.Signal(
        signal_name="LongTermFuelTrim",
        signal_start_bit=24,
        signal_size=8,
        signal_factor=(1 / 1.28),
        signal_offset=(-100),
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_intake_air_temp = can_decoder.Signal(
        signal_name="IntakeAirTemperature",
        signal_start_bit=24,
        signal_size=8,
        signal_factor=1,
        signal_offset=(-40),
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_throttle_position = can_decoder.Signal(
        signal_name="ThrottlePosition",
        signal_start_bit=24,
        signal_size=8,
        signal_factor=(1/2.55),
        signal_offset=0,
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_oxygen_sensor = can_decoder.Signal(
        signal_name="OxygenSensor",
        signal_start_bit=24,
        signal_size=8,
        signal_factor=0.005,
        signal_offset=0,
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_relative_throttle_pos = can_decoder.Signal(
        signal_name="RelativeThrottlePosition",
        signal_start_bit=24,
        signal_size=8,
        signal_factor=(1/2.55),
        signal_offset=0,
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_ambient_air_temp = can_decoder.Signal(
        signal_name="AmbientAirTemperature",
        signal_start_bit=24,
        signal_size=8,
        signal_factor=1,
        signal_offset=(-40),
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    signal_absolute_throttle_pos = can_decoder.Signal(
        signal_name="AbsoluteThrottlePosition",
        signal_start_bit=24,
        signal_size=8,
        signal_factor=(1 / 2.55),
        signal_offset=0,
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )
    for i in range(1, num_of_records):

        #frames
        rpm_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x0C, rd.randint(0, 40), rd.randint(0, 255), 0xAA, 0xAA, 0xAA
            ]
        }]
        speed_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x0D, rd.randint(0, 255), 0xAA, 0xAA, 0xAA, 0xAA
            ]
        }]
        fuel_system_status_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x03, rd.randint(0, 255), rd.randint(0, 255), 0xAA, 0xAA, 0xAA
            ]
        }]
        engine_load_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x04, rd.randint(0, 100), 0xAA, 0xAA, 0xAA, 0xAA
            ]
        }]
        engine_coolant_temp_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x05, rd.randint(-40, 215), 0xAA, 0xAA, 0xAA, 0xAA
            ]
        }]
        short_term_fuel_trim_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x06, rd.randint(-100, 99), 0xAA, 0xAA, 0xAA, 0xAA
            ]
        }]
        long_term_fuel_trim_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x07, rd.randint(-100, 99), 0xAA, 0xAA, 0xAA, 0xAA
            ]
        }]
        intake_air_temp_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x0F, rd.randint(-40, 215), 0xAA, 0xAA, 0xAA, 0xAA
            ]
        }]
        throttle_pos_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x11, rd.randint(0, 100), 0xAA, 0xAA, 0xAA, 0xAA
            ]
        }]
        oxygen_sensor_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x15, rd.randint(0, 255), 0xAA, 0xAA, 0xAA, 0xAA
            ]
        }]
        relative_throttle_pos_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x45, rd.randint(0, 100), 0xAA, 0xAA, 0xAA, 0xAA
            ]
        }]
        ambient_air_temp_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x46, rd.randint(-40, 215), 0xAA, 0xAA, 0xAA, 0xAA
            ]
        }]
        absolute_throttle_pos_frame = [{
            "TimeStamp": timestamp,
            "ID": 0x07E8,
            "IDE": False,
            "DataBytes": [
                0x04, 0x41, 0x47, rd.randint(0, 100), 0xAA, 0xAA, 0xAA, 0xAA
            ]
        }]

        #responses
        rpm_res = process_can_request(0x0C, signal_engine, rpm_frame)
        #corelated_speed = (rpm*60*pi*TireDiameter)/(GearRatio*WheelAxleRatio*1000000)
        corelated_speed = (rpm_res['Physical Value'].values[0] * 60 * 3.14 * int(configs.get("TireDiameter").data) * 25.4)/(float(configs.get("GearRatio").data)*float(configs.get("WheelAxleRatio").data)*1000000)
        speed_res = process_can_request(0x0D, signal_speed, speed_frame)
        fuel_sys_status_res = process_can_request(0x03, signal_fuel_system_status, fuel_system_status_frame)
        engine_load_res = process_can_request(0x04, signal_engine_load, engine_load_frame)
        engine_coolant_temp_res = process_can_request(0x05, signal_engine_coolant_temp, engine_coolant_temp_frame)
        short_term_fuel_trim_res = process_can_request(0x06, signal_short_term_fuel_trim, short_term_fuel_trim_frame)
        long_term_fuel_trim_res = process_can_request(0x07, signal_long_term_fuel_trim, long_term_fuel_trim_frame)
        intake_air_temp_res = process_can_request(0x0F, signal_intake_air_temp, intake_air_temp_frame)
        throttle_pos_res = process_can_request(0x11, signal_throttle_position, throttle_pos_frame)
        oxygen_sensor_res = process_can_request(0x15, signal_oxygen_sensor, oxygen_sensor_frame)
        rel_throttle_pos_res = process_can_request(0x45, signal_relative_throttle_pos, relative_throttle_pos_frame)
        ambient_air_temp_res = process_can_request(0x46, signal_ambient_air_temp, ambient_air_temp_frame)
        abs_throttle_pos_res = process_can_request(0x47, signal_absolute_throttle_pos, absolute_throttle_pos_frame)
        current_data = {'TimeStamp': timestamp,
                        'VIN': vin,
                        'MFG_Date': mfg_date,
                        'EngineRPM': rpm_res['Physical Value'].values[0],
                        'VehicleSpeed': corelated_speed,
                        'FuelSystemStatus': fuel_sys_status_res['Physical Value'].values[0],
                        'EngineLoad': engine_load_res['Physical Value'].values[0],
                        'EngineCoolantTemp': engine_coolant_temp_res['Physical Value'].values[0],
                        'ShortTermFuelTrim': short_term_fuel_trim_res['Physical Value'].values[0],
                        'LongTermFuelTrim': long_term_fuel_trim_res['Physical Value'].values[0],
                        'IntakeAirTemperature': intake_air_temp_res['Physical Value'].values[0],
                        'ThrottlePosition': throttle_pos_res['Physical Value'].values[0],
                        'OxygenSensor': oxygen_sensor_res['Physical Value'].values[0],
                        'RelativeThrottlePosition': rel_throttle_pos_res['Physical Value'].values[0],
                        'AmbientAirTemperature': ambient_air_temp_res['Physical Value'].values[0],
                        'AbsoluteThrottlePosition': abs_throttle_pos_res['Physical Value'].values[0],
                        }
        dataset = dataset.append(current_data, ignore_index=True)
        timestamp = timestamp + timedelta(seconds=1)
    dataset.to_csv('dataset.csv')
    print("Finished")


def process_can_request(pid, pid_signal, frames):
    """Helper function to setup example decoding rules for OBD2.

    :return: Database with example OBD2 decoding rules.
    """
    # While not required for correct decoding, OBD2 is specified as the protocol type.
    db = can_decoder.SignalDB(protocol="OBD2")

    frame = can_decoder.Frame(
        frame_id=0x000007E8,
        frame_size=8
    )

    signal_main_mux = can_decoder.Signal(
        signal_name="ServiceMux",
        signal_start_bit=8,
        signal_size=8,
        signal_factor=1,
        signal_offset=0,
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )

    signal_minor_mux = can_decoder.Signal(
        signal_name="PIDMux",
        signal_start_bit=16,
        signal_size=8,
        signal_factor=1,
        signal_offset=0,
        signal_is_little_endian=False,
        signal_is_float=False,
        signal_is_signed=False,
    )

    # Link the signals and muxes.
    signal_minor_mux.add_multiplexed_signal(pid, pid_signal)
    signal_main_mux.add_multiplexed_signal(0x41, signal_minor_mux)
    frame.add_signal(signal_main_mux)

    db.add_frame(frame)

    test_data = pd.DataFrame(frames).set_index("TimeStamp")
    decoder = can_decoder.DataFrameDecoder(db)
    result = decoder.decode_frame(test_data)

    return result


if __name__ == '__main__':
    generate_can_data()
    pass
