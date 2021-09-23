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
# function: read_udp_mmap_file()
# -
def read_udp_mmap_file(_file: str = BOK_UDP_MMAP_FILE, _delay: int = BOK_DELAY) -> None:
    _file = os.path.abspath(os.path.expanduser(_file))
    running = 1
    with open(_file, 'r') as _f:
        with contextlib.closing(mmap.mmap(_f.fileno(), 0, access=mmap.ACCESS_READ)) as _m:
            while running > 0:
                # reset to start of file
                _m.seek(0)
                # read data
                jd = struct.unpack('d', _m.read(8))
                aaxis_enabled = struct.unpack('i', _m.read(4))
                baxis_enabled = struct.unpack('i', _m.read(4))
                caxis_enabled = struct.unpack('i', _m.read(4))
                daxis_enabled = struct.unpack('i', _m.read(4))
                eaxis_enabled = struct.unpack('i', _m.read(4))
                faxis_enabled = struct.unpack('i', _m.read(4))
                gaxis_enabled = struct.unpack('i', _m.read(4))
                haxis_enabled = struct.unpack('i', _m.read(4))
                saxis_enabled = struct.unpack('i', _m.read(4))
                taxis_enabled = struct.unpack('i', _m.read(4))
                iaxis_enabled = struct.unpack('i', _m.read(4))
                header_length = struct.unpack('i', _m.read(4))
                # iaxis
                iaxis_sample_number = struct.unpack('i', _m.read(4))
                iaxis_general_input_0 = struct.unpack('i', _m.read(4))
                iaxis_general_input_1 = struct.unpack('i', _m.read(4))
                iaxis_general_input_2 = struct.unpack('i', _m.read(4))
                iaxis_general_input_3 = struct.unpack('i', _m.read(4))
                iaxis_general_input_4 = struct.unpack('i', _m.read(4))
                iaxis_general_input_5 = struct.unpack('i', _m.read(4))
                iaxis_general_input_6 = struct.unpack('i', _m.read(4))
                iaxis_general_input_7 = struct.unpack('i', _m.read(4))
                iaxis_general_input_8 = struct.unpack('i', _m.read(4))
                iaxis_general_input_9 = struct.unpack('i', _m.read(4))
                iaxis_general_output_0 = struct.unpack('i', _m.read(4))
                iaxis_general_output_1 = struct.unpack('i', _m.read(4))
                iaxis_general_output_2 = struct.unpack('i', _m.read(4))
                iaxis_general_output_3 = struct.unpack('i', _m.read(4))
                iaxis_general_output_4 = struct.unpack('i', _m.read(4))
                iaxis_general_output_5 = struct.unpack('i', _m.read(4))
                iaxis_general_output_6 = struct.unpack('i', _m.read(4))
                iaxis_general_output_7 = struct.unpack('i', _m.read(4))
                iaxis_general_output_8 = struct.unpack('i', _m.read(4))
                iaxis_general_output_9 = struct.unpack('i', _m.read(4))
                iaxis_error_code = struct.unpack('i', _m.read(4))
                iaxis_echo_on = struct.unpack('i', _m.read(4))
                iaxis_trace_on = struct.unpack('i', _m.read(4))
                iaxis_awaiting_input = struct.unpack('i', _m.read(4))
                iaxis_program_running = struct.unpack('i', _m.read(4))
                # saxis
                saxis_segment_count = struct.unpack('i', _m.read(4))
                saxis_decelerating = struct.unpack('i', _m.read(4))
                saxis_stopping = struct.unpack('i', _m.read(4))
                saxis_slewing = struct.unpack('i', _m.read(4))
                saxis_moving = struct.unpack('i', _m.read(4))
                saxis_distance_traveled = struct.unpack('i', _m.read(4))
                # taxis
                taxis_segment_count = struct.unpack('i', _m.read(4))
                taxis_decelerating = struct.unpack('i', _m.read(4))
                taxis_stopping = struct.unpack('i', _m.read(4))
                taxis_slewing = struct.unpack('i', _m.read(4))
                taxis_moving = struct.unpack('i', _m.read(4))
                taxis_distance_traveled = struct.unpack('i', _m.read(4))
                # aaxis
                aaxis_motor_off = struct.unpack('i', _m.read(4))
                aaxis_error_armed = struct.unpack('i', _m.read(4))
                aaxis_latch_armed = struct.unpack('i', _m.read(4))
                aaxis_decelerating = struct.unpack('i', _m.read(4))
                aaxis_stopping = struct.unpack('i', _m.read(4))
                aaxis_slewing = struct.unpack('i', _m.read(4))
                aaxis_contour_mode = struct.unpack('i', _m.read(4))
                aaxis_negative_direction = struct.unpack('i', _m.read(4))
                aaxis_motion_mode = struct.unpack('i', _m.read(4))
                aaxis_home_1 = struct.unpack('i', _m.read(4))
                aaxis_home_2 = struct.unpack('i', _m.read(4))
                aaxis_homing = struct.unpack('i', _m.read(4))
                aaxis_finding_edge = struct.unpack('i', _m.read(4))
                aaxis_pa_motion = struct.unpack('i', _m.read(4))
                aaxis_pr_motion = struct.unpack('i', _m.read(4))
                aaxis_moving = struct.unpack('i', _m.read(4))
                aaxis_sm_jumper = struct.unpack('i', _m.read(4))
                aaxis_state_home = struct.unpack('i', _m.read(4))
                aaxis_state_reverse = struct.unpack('i', _m.read(4))
                aaxis_state_forward = struct.unpack('i', _m.read(4))
                aaxis_state_latch = struct.unpack('i', _m.read(4))
                aaxis_latch_occurred = struct.unpack('i', _m.read(4))
                aaxis_stop_code = struct.unpack('i', _m.read(4))
                aaxis_reference_position = struct.unpack('i', _m.read(4))
                aaxis_motor_position = struct.unpack('i', _m.read(4))
                aaxis_position_error = struct.unpack('i', _m.read(4))
                aaxis_auxiliary_position = struct.unpack('i', _m.read(4))
                aaxis_velocity = struct.unpack('i', _m.read(4))
                aaxis_torque = struct.unpack('i', _m.read(4))
                aaxis_analog_in = struct.unpack('i', _m.read(4))
                # baxis
                baxis_motor_off = struct.unpack('i', _m.read(4))
                baxis_error_armed = struct.unpack('i', _m.read(4))
                baxis_latch_armed = struct.unpack('i', _m.read(4))
                baxis_decelerating = struct.unpack('i', _m.read(4))
                baxis_stopping = struct.unpack('i', _m.read(4))
                baxis_slewing = struct.unpack('i', _m.read(4))
                baxis_contour_mode = struct.unpack('i', _m.read(4))
                baxis_negative_direction = struct.unpack('i', _m.read(4))
                baxis_motion_mode = struct.unpack('i', _m.read(4))
                baxis_home_1 = struct.unpack('i', _m.read(4))
                baxis_home_2 = struct.unpack('i', _m.read(4))
                baxis_homing = struct.unpack('i', _m.read(4))
                baxis_finding_edge = struct.unpack('i', _m.read(4))
                baxis_pa_motion = struct.unpack('i', _m.read(4))
                baxis_pr_motion = struct.unpack('i', _m.read(4))
                baxis_moving = struct.unpack('i', _m.read(4))
                baxis_sm_jumper = struct.unpack('i', _m.read(4))
                baxis_state_home = struct.unpack('i', _m.read(4))
                baxis_state_reverse = struct.unpack('i', _m.read(4))
                baxis_state_forward = struct.unpack('i', _m.read(4))
                baxis_state_latch = struct.unpack('i', _m.read(4))
                baxis_latch_occurred = struct.unpack('i', _m.read(4))
                baxis_stop_code = struct.unpack('i', _m.read(4))
                baxis_reference_position = struct.unpack('i', _m.read(4))
                baxis_motor_position = struct.unpack('i', _m.read(4))
                baxis_position_error = struct.unpack('i', _m.read(4))
                baxis_auxiliary_position = struct.unpack('i', _m.read(4))
                baxis_velocity = struct.unpack('i', _m.read(4))
                baxis_torque = struct.unpack('i', _m.read(4))
                baxis_analog_in = struct.unpack('i', _m.read(4))
                # caxis
                caxis_motor_off = struct.unpack('i', _m.read(4))
                caxis_error_armed = struct.unpack('i', _m.read(4))
                caxis_latch_armed = struct.unpack('i', _m.read(4))
                caxis_decelerating = struct.unpack('i', _m.read(4))
                caxis_stopping = struct.unpack('i', _m.read(4))
                caxis_slewing = struct.unpack('i', _m.read(4))
                caxis_contour_mode = struct.unpack('i', _m.read(4))
                caxis_negative_direction = struct.unpack('i', _m.read(4))
                caxis_motion_mode = struct.unpack('i', _m.read(4))
                caxis_home_1 = struct.unpack('i', _m.read(4))
                caxis_home_2 = struct.unpack('i', _m.read(4))
                caxis_homing = struct.unpack('i', _m.read(4))
                caxis_finding_edge = struct.unpack('i', _m.read(4))
                caxis_pa_motion = struct.unpack('i', _m.read(4))
                caxis_pr_motion = struct.unpack('i', _m.read(4))
                caxis_moving = struct.unpack('i', _m.read(4))
                caxis_sm_jumper = struct.unpack('i', _m.read(4))
                caxis_state_home = struct.unpack('i', _m.read(4))
                caxis_state_reverse = struct.unpack('i', _m.read(4))
                caxis_state_forward = struct.unpack('i', _m.read(4))
                caxis_state_latch = struct.unpack('i', _m.read(4))
                caxis_latch_occurred = struct.unpack('i', _m.read(4))
                caxis_stop_code = struct.unpack('i', _m.read(4))
                caxis_reference_position = struct.unpack('i', _m.read(4))
                caxis_motor_position = struct.unpack('i', _m.read(4))
                caxis_position_error = struct.unpack('i', _m.read(4))
                caxis_auxiliary_position = struct.unpack('i', _m.read(4))
                caxis_velocity = struct.unpack('i', _m.read(4))
                caxis_torque = struct.unpack('i', _m.read(4))
                caxis_analog_in = struct.unpack('i', _m.read(4))
                # daxis
                daxis_motor_off = struct.unpack('i', _m.read(4))
                daxis_error_armed = struct.unpack('i', _m.read(4))
                daxis_latch_armed = struct.unpack('i', _m.read(4))
                daxis_decelerating = struct.unpack('i', _m.read(4))
                daxis_stopping = struct.unpack('i', _m.read(4))
                daxis_slewing = struct.unpack('i', _m.read(4))
                daxis_contour_mode = struct.unpack('i', _m.read(4))
                daxis_negative_direction = struct.unpack('i', _m.read(4))
                daxis_motion_mode = struct.unpack('i', _m.read(4))
                daxis_home_1 = struct.unpack('i', _m.read(4))
                daxis_home_2 = struct.unpack('i', _m.read(4))
                daxis_homing = struct.unpack('i', _m.read(4))
                daxis_finding_edge = struct.unpack('i', _m.read(4))
                daxis_pa_motion = struct.unpack('i', _m.read(4))
                daxis_pr_motion = struct.unpack('i', _m.read(4))
                daxis_moving = struct.unpack('i', _m.read(4))
                daxis_sm_jumper = struct.unpack('i', _m.read(4))
                daxis_state_home = struct.unpack('i', _m.read(4))
                daxis_state_reverse = struct.unpack('i', _m.read(4))
                daxis_state_forward = struct.unpack('i', _m.read(4))
                daxis_state_latch = struct.unpack('i', _m.read(4))
                daxis_latch_occurred = struct.unpack('i', _m.read(4))
                daxis_stop_code = struct.unpack('i', _m.read(4))
                daxis_reference_position = struct.unpack('i', _m.read(4))
                daxis_motor_position = struct.unpack('i', _m.read(4))
                daxis_position_error = struct.unpack('i', _m.read(4))
                daxis_auxiliary_position = struct.unpack('i', _m.read(4))
                daxis_velocity = struct.unpack('i', _m.read(4))
                daxis_torque = struct.unpack('i', _m.read(4))
                daxis_analog_in = struct.unpack('i', _m.read(4))
                # eaxis
                eaxis_motor_off = struct.unpack('i', _m.read(4))
                eaxis_error_armed = struct.unpack('i', _m.read(4))
                eaxis_latch_armed = struct.unpack('i', _m.read(4))
                eaxis_decelerating = struct.unpack('i', _m.read(4))
                eaxis_stopping = struct.unpack('i', _m.read(4))
                eaxis_slewing = struct.unpack('i', _m.read(4))
                eaxis_contour_mode = struct.unpack('i', _m.read(4))
                eaxis_negative_direction = struct.unpack('i', _m.read(4))
                eaxis_motion_mode = struct.unpack('i', _m.read(4))
                eaxis_home_1 = struct.unpack('i', _m.read(4))
                eaxis_home_2 = struct.unpack('i', _m.read(4))
                eaxis_homing = struct.unpack('i', _m.read(4))
                eaxis_finding_edge = struct.unpack('i', _m.read(4))
                eaxis_pa_motion = struct.unpack('i', _m.read(4))
                eaxis_pr_motion = struct.unpack('i', _m.read(4))
                eaxis_moving = struct.unpack('i', _m.read(4))
                eaxis_sm_jumper = struct.unpack('i', _m.read(4))
                eaxis_state_home = struct.unpack('i', _m.read(4))
                eaxis_state_reverse = struct.unpack('i', _m.read(4))
                eaxis_state_forward = struct.unpack('i', _m.read(4))
                eaxis_state_latch = struct.unpack('i', _m.read(4))
                eaxis_latch_occurred = struct.unpack('i', _m.read(4))
                eaxis_stop_code = struct.unpack('i', _m.read(4))
                eaxis_reference_position = struct.unpack('i', _m.read(4))
                eaxis_motor_position = struct.unpack('i', _m.read(4))
                eaxis_position_error = struct.unpack('i', _m.read(4))
                eaxis_auxiliary_position = struct.unpack('i', _m.read(4))
                eaxis_velocity = struct.unpack('i', _m.read(4))
                eaxis_torque = struct.unpack('i', _m.read(4))
                eaxis_analog_in = struct.unpack('i', _m.read(4))
                # faxis
                faxis_motor_off = struct.unpack('i', _m.read(4))
                faxis_error_armed = struct.unpack('i', _m.read(4))
                faxis_latch_armed = struct.unpack('i', _m.read(4))
                faxis_decelerating = struct.unpack('i', _m.read(4))
                faxis_stopping = struct.unpack('i', _m.read(4))
                faxis_slewing = struct.unpack('i', _m.read(4))
                faxis_contour_mode = struct.unpack('i', _m.read(4))
                faxis_negative_direction = struct.unpack('i', _m.read(4))
                faxis_motion_mode = struct.unpack('i', _m.read(4))
                faxis_home_1 = struct.unpack('i', _m.read(4))
                faxis_home_2 = struct.unpack('i', _m.read(4))
                faxis_homing = struct.unpack('i', _m.read(4))
                faxis_finding_edge = struct.unpack('i', _m.read(4))
                faxis_pa_motion = struct.unpack('i', _m.read(4))
                faxis_pr_motion = struct.unpack('i', _m.read(4))
                faxis_moving = struct.unpack('i', _m.read(4))
                faxis_sm_jumper = struct.unpack('i', _m.read(4))
                faxis_state_home = struct.unpack('i', _m.read(4))
                faxis_state_reverse = struct.unpack('i', _m.read(4))
                faxis_state_forward = struct.unpack('i', _m.read(4))
                faxis_state_latch = struct.unpack('i', _m.read(4))
                faxis_latch_occurred = struct.unpack('i', _m.read(4))
                faxis_stop_code = struct.unpack('i', _m.read(4))
                faxis_reference_position = struct.unpack('i', _m.read(4))
                faxis_motor_position = struct.unpack('i', _m.read(4))
                faxis_position_error = struct.unpack('i', _m.read(4))
                faxis_auxiliary_position = struct.unpack('i', _m.read(4))
                faxis_velocity = struct.unpack('i', _m.read(4))
                faxis_torque = struct.unpack('i', _m.read(4))
                faxis_analog_in = struct.unpack('i', _m.read(4))
                # gaxis
                gaxis_motor_off = struct.unpack('i', _m.read(4))
                gaxis_error_armed = struct.unpack('i', _m.read(4))
                gaxis_latch_armed = struct.unpack('i', _m.read(4))
                gaxis_decelerating = struct.unpack('i', _m.read(4))
                gaxis_stopping = struct.unpack('i', _m.read(4))
                gaxis_slewing = struct.unpack('i', _m.read(4))
                gaxis_contour_mode = struct.unpack('i', _m.read(4))
                gaxis_negative_direction = struct.unpack('i', _m.read(4))
                gaxis_motion_mode = struct.unpack('i', _m.read(4))
                gaxis_home_1 = struct.unpack('i', _m.read(4))
                gaxis_home_2 = struct.unpack('i', _m.read(4))
                gaxis_homing = struct.unpack('i', _m.read(4))
                gaxis_finding_edge = struct.unpack('i', _m.read(4))
                gaxis_pa_motion = struct.unpack('i', _m.read(4))
                gaxis_pr_motion = struct.unpack('i', _m.read(4))
                gaxis_moving = struct.unpack('i', _m.read(4))
                gaxis_sm_jumper = struct.unpack('i', _m.read(4))
                gaxis_state_home = struct.unpack('i', _m.read(4))
                gaxis_state_reverse = struct.unpack('i', _m.read(4))
                gaxis_state_forward = struct.unpack('i', _m.read(4))
                gaxis_state_latch = struct.unpack('i', _m.read(4))
                gaxis_latch_occurred = struct.unpack('i', _m.read(4))
                gaxis_stop_code = struct.unpack('i', _m.read(4))
                gaxis_reference_position = struct.unpack('i', _m.read(4))
                gaxis_motor_position = struct.unpack('i', _m.read(4))
                gaxis_position_error = struct.unpack('i', _m.read(4))
                gaxis_auxiliary_position = struct.unpack('i', _m.read(4))
                gaxis_velocity = struct.unpack('i', _m.read(4))
                gaxis_torque = struct.unpack('i', _m.read(4))
                gaxis_analog_in = struct.unpack('i', _m.read(4))
                # haxis
                haxis_motor_off = struct.unpack('i', _m.read(4))
                haxis_error_armed = struct.unpack('i', _m.read(4))
                haxis_latch_armed = struct.unpack('i', _m.read(4))
                haxis_decelerating = struct.unpack('i', _m.read(4))
                haxis_stopping = struct.unpack('i', _m.read(4))
                haxis_slewing = struct.unpack('i', _m.read(4))
                haxis_contour_mode = struct.unpack('i', _m.read(4))
                haxis_negative_direction = struct.unpack('i', _m.read(4))
                haxis_motion_mode = struct.unpack('i', _m.read(4))
                haxis_home_1 = struct.unpack('i', _m.read(4))
                haxis_home_2 = struct.unpack('i', _m.read(4))
                haxis_homing = struct.unpack('i', _m.read(4))
                haxis_finding_edge = struct.unpack('i', _m.read(4))
                haxis_pa_motion = struct.unpack('i', _m.read(4))
                haxis_pr_motion = struct.unpack('i', _m.read(4))
                haxis_moving = struct.unpack('i', _m.read(4))
                haxis_sm_jumper = struct.unpack('i', _m.read(4))
                haxis_state_home = struct.unpack('i', _m.read(4))
                haxis_state_reverse = struct.unpack('i', _m.read(4))
                haxis_state_forward = struct.unpack('i', _m.read(4))
                haxis_state_latch = struct.unpack('i', _m.read(4))
                haxis_latch_occurred = struct.unpack('i', _m.read(4))
                haxis_stop_code = struct.unpack('i', _m.read(4))
                haxis_reference_position = struct.unpack('i', _m.read(4))
                haxis_motor_position = struct.unpack('i', _m.read(4))
                haxis_position_error = struct.unpack('i', _m.read(4))
                haxis_auxiliary_position = struct.unpack('i', _m.read(4))
                haxis_velocity = struct.unpack('i', _m.read(4))
                haxis_torque = struct.unpack('i', _m.read(4))
                haxis_analog_in = struct.unpack('i', _m.read(4))
                # miscellaneous
                a_encoder = struct.unpack('i', _m.read(4))
                b_encoder = struct.unpack('i', _m.read(4))
                c_encoder = struct.unpack('i', _m.read(4))
                a_position = struct.unpack('f', _m.read(4))
                b_position = struct.unpack('f', _m.read(4))
                c_position = struct.unpack('f', _m.read(4))
                simulate = struct.unpack('i', _m.read(4))
                shutdown = struct.unpack('i', _m.read(4))
                counter = struct.unpack('i', _m.read(4))
                timestamp = _m.read(BOK_TIME_STAMP)
                timestamp_d = timestamp.decode().rstrip('\x00')
                # dump record
                print(f"\n{'<SIMULATE> ' if simulate[0] > 0 else '<DIRECT> '}"
                      f"aaxis_enabled={aaxis_enabled[0]}, "
                      f"baxis_enabled={baxis_enabled[0]}, "
                      f"caxis_enabled={caxis_enabled[0]}, "
                      f"daxis_enabled={daxis_enabled[0]}, "
                      f"eaxis_enabled={eaxis_enabled[0]}, "
                      f"faxis_enabled={faxis_enabled[0]}, "
                      f"gaxis_enabled={gaxis_enabled[0]}, "
                      f"haxis_enabled={haxis_enabled[0]}, "
                      f"saxis_enabled={saxis_enabled[0]}, "
                      f"taxis_enabled={taxis_enabled[0]}, "
                      f"iaxis_enabled={iaxis_enabled[0]}, "
                      f"header_length={header_length[0]}, "
                      f"iaxis_sample_number={iaxis_sample_number[0]}, "
                      f"iaxis_general_input[0]={iaxis_general_input_0[0]}, "
                      f"iaxis_general_input[1]={iaxis_general_input_1[0]}, "
                      f"iaxis_general_input[2]={iaxis_general_input_2[0]}, "
                      f"iaxis_general_input[3]={iaxis_general_input_3[0]}, "
                      f"iaxis_general_input[4]={iaxis_general_input_4[0]}, "
                      f"iaxis_general_input[5]={iaxis_general_input_5[0]}, "
                      f"iaxis_general_input[6]={iaxis_general_input_6[0]}, "
                      f"iaxis_general_input[7]={iaxis_general_input_7[0]}, "
                      f"iaxis_general_input[8]={iaxis_general_input_8[0]}, "
                      f"iaxis_general_input[9]={iaxis_general_input_9[0]}, "
                      f"iaxis_general_output[0]={iaxis_general_output_0[0]}, "
                      f"iaxis_general_output[1]={iaxis_general_output_1[0]}, "
                      f"iaxis_general_output[2]={iaxis_general_output_2[0]}, "
                      f"iaxis_general_output[3]={iaxis_general_output_3[0]}, "
                      f"iaxis_general_output[4]={iaxis_general_output_4[0]}, "
                      f"iaxis_general_output[5]={iaxis_general_output_5[0]}, "
                      f"iaxis_general_output[6]={iaxis_general_output_6[0]}, "
                      f"iaxis_general_output[7]={iaxis_general_output_7[0]}, "
                      f"iaxis_general_output[8]={iaxis_general_output_8[0]}, "
                      f"iaxis_general_output[9]={iaxis_general_output_9[0]}, "
                      f"iaxis_error_code={iaxis_error_code[0]}, "
                      f"iaxis_echo_on={iaxis_echo_on[0]}, "
                      f"iaxis_trace_on={iaxis_trace_on[0]}, "
                      f"iaxis_awaiting_input={iaxis_awaiting_input[0]}, "
                      f"iaxis_program_running={iaxis_program_running[0]}, "
                      f"saxis_segment_count={saxis_segment_count[0]}, "
                      f"saxis_decelerating={saxis_decelerating[0]}, "
                      f"saxis_stopping={saxis_stopping[0]}, "
                      f"saxis_slewing={saxis_slewing[0]}, "
                      f"saxis_moving={saxis_moving[0]}, "
                      f"saxis_distance_traveled={saxis_distance_traveled[0]}, "
                      f"taxis_segment_count={taxis_segment_count[0]}, "
                      f"taxis_decelerating={taxis_decelerating[0]}, "
                      f"taxis_stopping={taxis_stopping[0]}, "
                      f"taxis_slewing={taxis_slewing[0]}, "
                      f"taxis_moving={taxis_moving[0]}, "
                      f"taxis_distance_traveled={taxis_distance_traveled[0]}, "
                      f"aaxis_motor_off={aaxis_motor_off[0]}, "
                      f"aaxis_error_armed={aaxis_error_armed[0]}, "
                      f"aaxis_latch_armed={aaxis_latch_armed[0]}, "
                      f"aaxis_decelerating={aaxis_decelerating[0]}, "
                      f"aaxis_stopping={aaxis_stopping[0]}, "
                      f"aaxis_slewing={aaxis_slewing[0]}, "
                      f"aaxis_contour_mode={aaxis_contour_mode[0]}, "
                      f"aaxis_negative_direction={aaxis_negative_direction[0]}, "
                      f"aaxis_motion_mode={aaxis_motion_mode[0]}, "
                      f"aaxis_home_1={aaxis_home_1[0]}, "
                      f"aaxis_home_2={aaxis_home_2[0]}, "
                      f"aaxis_homing={aaxis_homing[0]}, "
                      f"aaxis_finding_edge={aaxis_finding_edge[0]}, "
                      f"aaxis_pa_motion={aaxis_pa_motion[0]}, "
                      f"aaxis_pr_motion={aaxis_pr_motion[0]}, "
                      f"aaxis_moving={aaxis_moving[0]}, "
                      f"aaxis_sm_jumper={aaxis_sm_jumper[0]}, "
                      f"aaxis_state_home={aaxis_state_home[0]}, "
                      f"aaxis_state_reverse={aaxis_state_reverse[0]}, "
                      f"aaxis_state_forward={aaxis_state_forward[0]}, "
                      f"aaxis_state_latch={aaxis_state_latch[0]}, "
                      f"aaxis_latch_occurred={aaxis_latch_occurred[0]}, "
                      f"aaxis_stop_code={aaxis_stop_code[0]}, "
                      f"aaxis_reference_position={aaxis_reference_position[0]}, "
                      f"aaxis_motor_position={aaxis_motor_position[0]}, "
                      f"aaxis_position_error={aaxis_position_error[0]}, "
                      f"aaxis_auxiliary_position={aaxis_auxiliary_position[0]}, "
                      f"aaxis_velocity={aaxis_velocity[0]}, "
                      f"aaxis_torque={aaxis_torque[0]}, "
                      f"aaxis_analog_in={aaxis_analog_in[0]}, "
                      f"baxis_motor_off={baxis_motor_off[0]}, "
                      f"baxis_error_armed={baxis_error_armed[0]}, "
                      f"baxis_latch_armed={baxis_latch_armed[0]}, "
                      f"baxis_decelerating={baxis_decelerating[0]}, "
                      f"baxis_stopping={baxis_stopping[0]}, "
                      f"baxis_slewing={baxis_slewing[0]}, "
                      f"baxis_contour_mode={baxis_contour_mode[0]}, "
                      f"baxis_negative_direction={baxis_negative_direction[0]}, "
                      f"baxis_motion_mode={baxis_motion_mode[0]}, "
                      f"baxis_home_1={baxis_home_1[0]}, "
                      f"baxis_home_2={baxis_home_2[0]}, "
                      f"baxis_homing={baxis_homing[0]}, "
                      f"baxis_finding_edge={baxis_finding_edge[0]}, "
                      f"baxis_pa_motion={baxis_pa_motion[0]}, "
                      f"baxis_pr_motion={baxis_pr_motion[0]}, "
                      f"baxis_moving={baxis_moving[0]}, "
                      f"baxis_sm_jumper={baxis_sm_jumper[0]}, "
                      f"baxis_state_home={baxis_state_home[0]}, "
                      f"baxis_state_reverse={baxis_state_reverse[0]}, "
                      f"baxis_state_forward={baxis_state_forward[0]}, "
                      f"baxis_state_latch={baxis_state_latch[0]}, "
                      f"baxis_latch_occurred={baxis_latch_occurred[0]}, "
                      f"baxis_stop_code={baxis_stop_code[0]}, "
                      f"baxis_reference_position={baxis_reference_position[0]}, "
                      f"baxis_motor_position={baxis_motor_position[0]}, "
                      f"baxis_position_error={baxis_position_error[0]}, "
                      f"baxis_auxiliary_position={baxis_auxiliary_position[0]}, "
                      f"baxis_velocity={baxis_velocity[0]}, "
                      f"baxis_torque={baxis_torque[0]}, "
                      f"baxis_analog_in={baxis_analog_in[0]}, "
                      f"caxis_motor_off={caxis_motor_off[0]}, "
                      f"caxis_error_armed={caxis_error_armed[0]}, "
                      f"caxis_latch_armed={caxis_latch_armed[0]}, "
                      f"caxis_decelerating={caxis_decelerating[0]}, "
                      f"caxis_stopping={caxis_stopping[0]}, "
                      f"caxis_slewing={caxis_slewing[0]}, "
                      f"caxis_contour_mode={caxis_contour_mode[0]}, "
                      f"caxis_negative_direction={caxis_negative_direction[0]}, "
                      f"caxis_motion_mode={caxis_motion_mode[0]}, "
                      f"caxis_home_1={caxis_home_1[0]}, "
                      f"caxis_home_2={caxis_home_2[0]}, "
                      f"caxis_homing={caxis_homing[0]}, "
                      f"caxis_finding_edge={caxis_finding_edge[0]}, "
                      f"caxis_pa_motion={caxis_pa_motion[0]}, "
                      f"caxis_pr_motion={caxis_pr_motion[0]}, "
                      f"caxis_moving={caxis_moving[0]}, "
                      f"caxis_sm_jumper={caxis_sm_jumper[0]}, "
                      f"caxis_state_home={caxis_state_home[0]}, "
                      f"caxis_state_reverse={caxis_state_reverse[0]}, "
                      f"caxis_state_forward={caxis_state_forward[0]}, "
                      f"caxis_state_latch={caxis_state_latch[0]}, "
                      f"caxis_latch_occurred={caxis_latch_occurred[0]}, "
                      f"caxis_stop_code={caxis_stop_code[0]}, "
                      f"caxis_reference_position={caxis_reference_position[0]}, "
                      f"caxis_motor_position={caxis_motor_position[0]}, "
                      f"caxis_position_error={caxis_position_error[0]}, "
                      f"caxis_auxiliary_position={caxis_auxiliary_position[0]}, "
                      f"caxis_velocity={caxis_velocity[0]}, "
                      f"caxis_torque={caxis_torque[0]}, "
                      f"caxis_analog_in={caxis_analog_in[0]}, "
                      f"daxis_motor_off={daxis_motor_off[0]}, "
                      f"daxis_error_armed={daxis_error_armed[0]}, "
                      f"daxis_latch_armed={daxis_latch_armed[0]}, "
                      f"daxis_decelerating={daxis_decelerating[0]}, "
                      f"daxis_stopping={daxis_stopping[0]}, "
                      f"daxis_slewing={daxis_slewing[0]}, "
                      f"daxis_contour_mode={daxis_contour_mode[0]}, "
                      f"daxis_negative_direction={daxis_negative_direction[0]}, "
                      f"daxis_motion_mode={daxis_motion_mode[0]}, "
                      f"daxis_home_1={daxis_home_1[0]}, "
                      f"daxis_home_2={daxis_home_2[0]}, "
                      f"daxis_homing={daxis_homing[0]}, "
                      f"daxis_finding_edge={daxis_finding_edge[0]}, "
                      f"daxis_pa_motion={daxis_pa_motion[0]}, "
                      f"daxis_pr_motion={daxis_pr_motion[0]}, "
                      f"daxis_moving={daxis_moving[0]}, "
                      f"daxis_sm_jumper={daxis_sm_jumper[0]}, "
                      f"daxis_state_home={daxis_state_home[0]}, "
                      f"daxis_state_reverse={daxis_state_reverse[0]}, "
                      f"daxis_state_forward={daxis_state_forward[0]}, "
                      f"daxis_state_latch={daxis_state_latch[0]}, "
                      f"daxis_latch_occurred={daxis_latch_occurred[0]}, "
                      f"daxis_stop_code={daxis_stop_code[0]}, "
                      f"daxis_reference_position={daxis_reference_position[0]}, "
                      f"daxis_motor_position={daxis_motor_position[0]}, "
                      f"daxis_position_error={daxis_position_error[0]}, "
                      f"daxis_auxiliary_position={daxis_auxiliary_position[0]}, "
                      f"daxis_velocity={daxis_velocity[0]}, "
                      f"daxis_torque={daxis_torque[0]}, "
                      f"daxis_analog_in={daxis_analog_in[0]}, "
                      f"eaxis_motor_off={eaxis_motor_off[0]}, "
                      f"eaxis_error_armed={eaxis_error_armed[0]}, "
                      f"eaxis_latch_armed={eaxis_latch_armed[0]}, "
                      f"eaxis_decelerating={eaxis_decelerating[0]}, "
                      f"eaxis_stopping={eaxis_stopping[0]}, "
                      f"eaxis_slewing={eaxis_slewing[0]}, "
                      f"eaxis_contour_mode={eaxis_contour_mode[0]}, "
                      f"eaxis_negative_direction={eaxis_negative_direction[0]}, "
                      f"eaxis_motion_mode={eaxis_motion_mode[0]}, "
                      f"eaxis_home_1={eaxis_home_1[0]}, "
                      f"eaxis_home_2={eaxis_home_2[0]}, "
                      f"eaxis_homing={eaxis_homing[0]}, "
                      f"eaxis_finding_edge={eaxis_finding_edge[0]}, "
                      f"eaxis_pa_motion={eaxis_pa_motion[0]}, "
                      f"eaxis_pr_motion={eaxis_pr_motion[0]}, "
                      f"eaxis_moving={eaxis_moving[0]}, "
                      f"eaxis_sm_jumper={eaxis_sm_jumper[0]}, "
                      f"eaxis_state_home={eaxis_state_home[0]}, "
                      f"eaxis_state_reverse={eaxis_state_reverse[0]}, "
                      f"eaxis_state_forward={eaxis_state_forward[0]}, "
                      f"eaxis_state_latch={eaxis_state_latch[0]}, "
                      f"eaxis_latch_occurred={eaxis_latch_occurred[0]}, "
                      f"eaxis_stop_code={eaxis_stop_code[0]}, "
                      f"eaxis_reference_position={eaxis_reference_position[0]}, "
                      f"eaxis_motor_position={eaxis_motor_position[0]}, "
                      f"eaxis_position_error={eaxis_position_error[0]}, "
                      f"eaxis_auxiliary_position={eaxis_auxiliary_position[0]}, "
                      f"eaxis_velocity={eaxis_velocity[0]}, "
                      f"eaxis_torque={eaxis_torque[0]}, "
                      f"eaxis_analog_in={eaxis_analog_in[0]}, "
                      f"faxis_motor_off={faxis_motor_off[0]}, "
                      f"faxis_error_armed={faxis_error_armed[0]}, "
                      f"faxis_latch_armed={faxis_latch_armed[0]}, "
                      f"faxis_decelerating={faxis_decelerating[0]}, "
                      f"faxis_stopping={faxis_stopping[0]}, "
                      f"faxis_slewing={faxis_slewing[0]}, "
                      f"faxis_contour_mode={faxis_contour_mode[0]}, "
                      f"faxis_negative_direction={faxis_negative_direction[0]}, "
                      f"faxis_motion_mode={faxis_motion_mode[0]}, "
                      f"faxis_home_1={faxis_home_1[0]}, "
                      f"faxis_home_2={faxis_home_2[0]}, "
                      f"faxis_homing={faxis_homing[0]}, "
                      f"faxis_finding_edge={faxis_finding_edge[0]}, "
                      f"faxis_pa_motion={faxis_pa_motion[0]}, "
                      f"faxis_pr_motion={faxis_pr_motion[0]}, "
                      f"faxis_moving={faxis_moving[0]}, "
                      f"faxis_sm_jumper={faxis_sm_jumper[0]}, "
                      f"faxis_state_home={faxis_state_home[0]}, "
                      f"faxis_state_reverse={faxis_state_reverse[0]}, "
                      f"faxis_state_forward={faxis_state_forward[0]}, "
                      f"faxis_state_latch={faxis_state_latch[0]}, "
                      f"faxis_latch_occurred={faxis_latch_occurred[0]}, "
                      f"faxis_stop_code={faxis_stop_code[0]}, "
                      f"faxis_reference_position={faxis_reference_position[0]}, "
                      f"faxis_motor_position={faxis_motor_position[0]}, "
                      f"faxis_position_error={faxis_position_error[0]}, "
                      f"faxis_auxiliary_position={faxis_auxiliary_position[0]}, "
                      f"faxis_velocity={faxis_velocity[0]}, "
                      f"faxis_torque={faxis_torque[0]}, "
                      f"faxis_analog_in={faxis_analog_in[0]}, "
                      f"gaxis_motor_off={gaxis_motor_off[0]}, "
                      f"gaxis_error_armed={gaxis_error_armed[0]}, "
                      f"gaxis_latch_armed={gaxis_latch_armed[0]}, "
                      f"gaxis_decelerating={gaxis_decelerating[0]}, "
                      f"gaxis_stopping={gaxis_stopping[0]}, "
                      f"gaxis_slewing={gaxis_slewing[0]}, "
                      f"gaxis_contour_mode={gaxis_contour_mode[0]}, "
                      f"gaxis_negative_direction={gaxis_negative_direction[0]}, "
                      f"gaxis_motion_mode={gaxis_motion_mode[0]}, "
                      f"gaxis_home_1={gaxis_home_1[0]}, "
                      f"gaxis_home_2={gaxis_home_2[0]}, "
                      f"gaxis_homing={gaxis_homing[0]}, "
                      f"gaxis_finding_edge={gaxis_finding_edge[0]}, "
                      f"gaxis_pa_motion={gaxis_pa_motion[0]}, "
                      f"gaxis_pr_motion={gaxis_pr_motion[0]}, "
                      f"gaxis_moving={gaxis_moving[0]}, "
                      f"gaxis_sm_jumper={gaxis_sm_jumper[0]}, "
                      f"gaxis_state_home={gaxis_state_home[0]}, "
                      f"gaxis_state_reverse={gaxis_state_reverse[0]}, "
                      f"gaxis_state_forward={gaxis_state_forward[0]}, "
                      f"gaxis_state_latch={gaxis_state_latch[0]}, "
                      f"gaxis_latch_occurred={gaxis_latch_occurred[0]}, "
                      f"gaxis_stop_code={gaxis_stop_code[0]}, "
                      f"gaxis_reference_position={gaxis_reference_position[0]}, "
                      f"gaxis_motor_position={gaxis_motor_position[0]}, "
                      f"gaxis_position_error={gaxis_position_error[0]}, "
                      f"gaxis_auxiliary_position={gaxis_auxiliary_position[0]}, "
                      f"gaxis_velocity={gaxis_velocity[0]}, "
                      f"gaxis_torque={gaxis_torque[0]}, "
                      f"gaxis_analog_in={gaxis_analog_in[0]}, "
                      f"haxis_motor_off={haxis_motor_off[0]}, "
                      f"haxis_error_armed={haxis_error_armed[0]}, "
                      f"haxis_latch_armed={haxis_latch_armed[0]}, "
                      f"haxis_decelerating={haxis_decelerating[0]}, "
                      f"haxis_stopping={haxis_stopping[0]}, "
                      f"haxis_slewing={haxis_slewing[0]}, "
                      f"haxis_contour_mode={haxis_contour_mode[0]}, "
                      f"haxis_negative_direction={haxis_negative_direction[0]}, "
                      f"haxis_motion_mode={haxis_motion_mode[0]}, "
                      f"haxis_home_1={haxis_home_1[0]}, "
                      f"haxis_home_2={haxis_home_2[0]}, "
                      f"haxis_homing={haxis_homing[0]}, "
                      f"haxis_finding_edge={haxis_finding_edge[0]}, "
                      f"haxis_pa_motion={haxis_pa_motion[0]}, "
                      f"haxis_pr_motion={haxis_pr_motion[0]}, "
                      f"haxis_moving={haxis_moving[0]}, "
                      f"haxis_sm_jumper={haxis_sm_jumper[0]}, "
                      f"haxis_state_home={haxis_state_home[0]}, "
                      f"haxis_state_reverse={haxis_state_reverse[0]}, "
                      f"haxis_state_forward={haxis_state_forward[0]}, "
                      f"haxis_state_latch={haxis_state_latch[0]}, "
                      f"haxis_latch_occurred={haxis_latch_occurred[0]}, "
                      f"haxis_stop_code={haxis_stop_code[0]}, "
                      f"haxis_reference_position={haxis_reference_position[0]}, "
                      f"haxis_motor_position={haxis_motor_position[0]}, "
                      f"haxis_position_error={haxis_position_error[0]}, "
                      f"haxis_auxiliary_position={haxis_auxiliary_position[0]}, "
                      f"haxis_velocity={haxis_velocity[0]}, "
                      f"haxis_torque={haxis_torque[0]}, "
                      f"haxis_analog_in={haxis_analog_in[0]}, "
                      f"a_encoder={a_encoder[0]}, "
                      f"b_encoder={b_encoder[0]}, "
                      f"c_encoder={c_encoder[0]}, "
                      f"a_position={a_position[0]:.3f}, "
                      f"b_position={b_position[0]:.3f}, "
                      f"c_position={c_position[0]:.3f}, "
                      f"simulate={simulate[0]}, "
                      f"shutdown={shutdown[0]}, "
                      f"counter={counter[0]}, "
                      f"jd={jd[0]:.8f}, " 
                      f"timestamp='{timestamp_d}'")
                # check shutdown flag
                if shutdown[0] > 0:
                    running = 0
                msleep(_delay)


# +
# main()
# -
if __name__ == '__main__':

    # get command line argument(s)
    _p = argparse.ArgumentParser(description='Galil_DMC_22x0_UDP_Read', formatter_class=argparse.RawTextHelpFormatter)
    _p.add_argument('-f', '--file', default=f"{BOK_UDP_MMAP_FILE}", help="""Input file [%(default)s]""")
    _p.add_argument('-d', '--delay', default=f"{BOK_DELAY}", help="""Delay (ms) [%(default)s]""")
    _args = _p.parse_args()

    # noinspection PyBroadException
    try:
        read_udp_mmap_file(_file=_args.file, _delay=int(_args.delay))
    except Exception as _e:
        print(f'{_e}')
        print(f'Use: python3 {sys.argv[0]} --help')
