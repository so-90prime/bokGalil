#!/usr/bin/env python3


# +
# import(s)
# -
import argparse
import contextlib
import mmap
import os
import struct
import sys
import time


# +
# function: msleep()
# -
def msleep(_seconds: int = 1):
    time.sleep(_seconds/1000.0)


# +
# constant(s)
# -
BOK_DELAY = 1000
BOK_STR_64 = 64
BOK_TIME_STAMP = 26
BOK_UDP_MMAP_FILE = "/dev/shm/udp_shm"


# +
# function: read_mmap_to_dict()
# -
def read_mmap_to_dict(_m: mmap.mmap = None, _verbose: bool = False) -> dict:

    # declare variables and initialize them
    _udp_d, _hardware, _software, _timestamp = {}, '', '', ''

    # decode memory map (caveat lector: this is very fickle and sensitive to changes!)
    try:
        _m.seek(0)
        _udp_d['jd'] = float(f"{struct.unpack('d', _m.read(8))[0]}")
        _udp_d['aaxis_enabled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_enabled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_enabled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_enabled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_enabled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_enabled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_enabled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_enabled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['saxis_enabled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['taxis_enabled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_enabled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['header_length'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # iaxis
        _udp_d['iaxis_sample_number'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_input_0'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_input_1'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_input_2'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_input_3'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_input_4'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_input_5'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_input_6'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_input_7'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_input_8'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_input_9'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_output_0'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_output_1'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_output_2'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_output_3'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_output_4'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_output_5'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_output_6'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_output_7'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_output_8'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_general_output_9'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_error_code'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_echo_on'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_trace_on'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_awaiting_input'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['iaxis_program_running'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # saxis
        _udp_d['saxis_segment_count'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['saxis_decelerating'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['saxis_stopping'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['saxis_slewing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['saxis_moving'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['saxis_distance_traveled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # taxis
        _udp_d['taxis_segment_count'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['taxis_decelerating'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['stopping'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['taxis_slewing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['taxis_moving'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['taxis_distance_traveled'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # aaxis
        _udp_d['aaxis_motor_off'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_error_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_latch_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_decelerating'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_stopping'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_slewing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_contour_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_negative_direction'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_motion_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_home_1'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_home_2'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_homing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_finding_edge'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_pa_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_pr_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_moving'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_sm_jumper'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_state_home'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_state_reverse'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_state_forward'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_state_latch'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_latch_occurred'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_stop_code'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_reference_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_motor_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_position_error'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_auxiliary_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_velocity'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_torque'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['aaxis_analog_in'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # baxis
        _udp_d['baxis_motor_off'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_error_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_latch_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_decelerating'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_stopping'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_slewing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_contour_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_negative_direction'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_motion_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_home_1'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_home_2'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_homing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_finding_edge'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_pa_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_pr_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_moving'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_sm_jumper'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_state_home'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_state_reverse'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_state_forward'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_state_latch'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_latch_occurred'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_stop_code'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_reference_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_motor_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_position_error'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_auxiliary_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_velocity'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_torque'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['baxis_analog_in'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # caxis
        _udp_d['caxis_motor_off'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_error_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_latch_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_decelerating'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_stopping'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_slewing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_contour_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_negative_direction'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_motion_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_home_1'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_home_2'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_homing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_finding_edge'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_pa_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_pr_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_moving'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_sm_jumper'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_state_home'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_state_reverse'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_state_forward'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_state_latch'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_latch_occurred'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_stop_code'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_reference_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_motor_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_position_error'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_auxiliary_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_velocity'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_torque'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['caxis_analog_in'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # daxis
        _udp_d['daxis_motor_off'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_error_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_latch_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_decelerating'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_stopping'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_slewing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_contour_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_negative_direction'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_motion_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_home_1'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_home_2'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_homing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_finding_edge'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_pa_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_pr_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_moving'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_sm_jumper'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_state_home'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_state_reverse'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_state_forward'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_state_latch'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_latch_occurred'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_stop_code'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_reference_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_motor_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_position_error'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_auxiliary_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_velocity'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_torque'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['daxis_analog_in'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # eaxis
        _udp_d['eaxis_motor_off'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_error_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_latch_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_decelerating'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_stopping'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_slewing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_contour_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_negative_direction'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_motion_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_home_1'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_home_2'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_homing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_finding_edge'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_pa_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_pr_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_moving'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_sm_jumper'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_state_home'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_state_reverse'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_state_forward'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_state_latch'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_latch_occurred'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_stop_code'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_reference_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_motor_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_position_error'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_auxiliary_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_velocity'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_torque'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['eaxis_analog_in'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # faxis
        _udp_d['faxis_motor_off'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_error_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_latch_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_decelerating'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_stopping'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_slewing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_contour_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_negative_direction'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_motion_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_home_1'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_home_2'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_homing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_finding_edge'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_pa_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_pr_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_moving'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_sm_jumper'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_state_home'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_state_reverse'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_state_forward'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_state_latch'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_latch_occurred'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_stop_code'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_reference_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_motor_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_position_error'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_auxiliary_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_velocity'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_torque'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['faxis_analog_in'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # gaxis
        _udp_d['gaxis_motor_off'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_error_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_latch_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_decelerating'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_stopping'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_slewing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_contour_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_negative_direction'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_motion_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_home_1'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_home_2'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_homing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_finding_edge'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_pa_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_pr_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_moving'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_sm_jumper'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_state_home'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_state_reverse'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_state_forward'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_state_latch'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_latch_occurred'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_stop_code'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_reference_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_motor_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_position_error'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_auxiliary_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_velocity'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_torque'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['gaxis_analog_in'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # haxis
        _udp_d['haxis_motor_off'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_error_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_latch_armed'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_decelerating'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_stopping'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_slewing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_contour_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_negative_direction'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_motion_mode'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_home_1'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_home_2'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_homing'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_finding_edge'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_pa_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_pr_motion'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_moving'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_sm_jumper'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_state_home'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_state_reverse'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_state_forward'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_state_latch'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_latch_occurred'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_stop_code'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_reference_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_motor_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_position_error'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_auxiliary_position'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_velocity'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_torque'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['haxis_analog_in'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        # miscellaneous
        _udp_d['a_encoder'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['b_encoder'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['c_encoder'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['a_position'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _udp_d['b_position'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _udp_d['c_position'] = float(f"{struct.unpack('f', _m.read(4))[0]}")
        _udp_d['simulate'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['shutdown'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _udp_d['counter'] = int(f"{struct.unpack('i', _m.read(4))[0]}")
        _timestamp = _m.read(BOK_TIME_STAMP)
        _udp_d['timestamp'] = _timestamp.decode().rstrip('\x00')
        _udp_d['file_read_error'] = ''
    except Exception as _x:
        return {'file_read_error': f'{_x}'}
    else:
        if _verbose:
            print(f"_udp_d={_udp_d}")

    # return
    return _udp_d


# +
# function: read_udp_memory_as_dict()
# -
def read_udp_memory_as_dict(_file: str = BOK_UDP_MMAP_FILE, _verbose: bool = False) -> dict:
    with open(os.path.abspath(os.path.expanduser(_file)), 'r') as _f:
        with contextlib.closing(mmap.mmap(_f.fileno(), 0, access=mmap.ACCESS_READ)) as _m:
            return read_mmap_to_dict(_m=_m, _verbose=_verbose)


# +
# function: read_udp_memory_in_loop()
# -
def read_udp_memory_in_loop(_file: str = BOK_UDP_MMAP_FILE, _delay: int = BOK_DELAY, _verbose: bool = False) -> None:
    _running = 1
    with open(os.path.abspath(os.path.expanduser(_file)), 'r') as _f:
        with contextlib.closing(mmap.mmap(_f.fileno(), 0, access=mmap.ACCESS_READ)) as _m:
            while _running > 0:
                _d = read_mmap_to_dict(_m=_m, _verbose=_verbose)
                if 'shutdown' in _d and _d['shutdown'] > 0:
                    _running = 0
                msleep(_delay)


# +
# main()
# -
if __name__ == '__main__':

    # get command line argument(s)
    _p = argparse.ArgumentParser(description='Galil_DMC_22x0_UDP_Read', formatter_class=argparse.RawTextHelpFormatter)
    _p.add_argument('--file', default=f"{BOK_UDP_MMAP_FILE}", help="""Input file [%(default)s]""")
    _p.add_argument('--delay', default=f"{BOK_DELAY}", help="""Delay (ms) [%(default)s]""")
    _p.add_argument('--loop', default=False, action='store_true', help='if present, read memory in loop')
    _p.add_argument('--verbose', default=False, action='store_true', help='if present, print output')
    _args = _p.parse_args()

    # noinspection PyBroadException
    try:
        if bool(_args.loop):
            read_udp_memory_in_loop(_file=_args.file, _delay=int(_args.delay), _verbose=bool(_args.verbose))
        else:
            read_udp_memory_as_dict(_file=_args.file, _verbose=bool(_args.verbose))
    except Exception as _e:
        print(f'{_e}\nUse: python3 {sys.argv[0]} --help')
