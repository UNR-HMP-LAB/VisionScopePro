import math
from matplotlib import pyplot as plt
import numpy as np
import pandas as pd
from scipy.ndimage import gaussian_filter1d
import os


def isRightEyeFirstIlluminated(key_str):
    '''
        Returns True if the right eye is illuminated first
        Arguments:
            key_str: The key of the hashmap which follows right_illum then left_illum as string
                    example: '1.0_0.0_0.0_1.0'
    '''
    temp_arr = key_str.split('_')
    return temp_arr[0] != '0.0'


def calculateWindowPeak_val(
    filtered_arr,
    window_start_time_index
):
    '''
        Returns the maximum value in the window.
        As we consider the highest value occurs within the first 100 timestamp window, 
        the window_start_time_index is the index of the first timestamp in the window and
        The window_end_time_index is 100 + the index of the first timestamp in the window.
        Arguments:
            filtered_arr: The filtered data
            window_start_time_index: The index of the first timestamp in the window
    '''
    return max(filtered_arr[window_start_time_index: window_start_time_index + 100])


def calculateWindowPeak_val_timestamp(
    filtered_arr,
    window_start_time_index
):
    '''
        Returns the timestamp of the maximum value in the window.
        As we consider the highest value occurs within the first 100 timestamp window, 
        the window_start_time_index is the index of the first timestamp in the window and
        The window_end_time_index is 100 + the index of the first timestamp in the window.
        Arguments:
            filtered_arr: The filtered data
            window_start_time_index: The index of the first timestamp in the window
    '''
    temp = max(
        filtered_arr[window_start_time_index: window_start_time_index + 100])
    return window_start_time_index + \
        np.where(
            filtered_arr[window_start_time_index: window_start_time_index + 100] == temp)[0][0]


def calculateWindowValley_val(
    filtered_arr,
    window_start_time_index,
    window_end_time_index
):
    '''
        Returns the minimum value in the window.
        As the minimum value can occur any time at the window,
        the window_start_time_index is the index of the first timestamp in the window and
        The window_end_time_index is the index of the last timestamp in the window.
        Arguments:
            filtered_arr: The filtered data
            window_start_time_index: The index of the first timestamp in the window
            window_end_time_index: The index of the last timestamp in the window
    '''
    return min(filtered_arr[window_start_time_index: window_end_time_index])


def calculateWindowValley_val_timestamp(
    filtered_arr,
    window_start_time_index,
    window_end_time_index
):
    '''
        Returns the timestamp of the minimum value in the window.
        As the minimum value can occur any time at the window,
        the window_start_time_index is the index of the first timestamp in the window and
        The window_end_time_index is the index of the last timestamp in the window.
        Arguments:
            filtered_arr: The filtered data
            window_start_time_index: The index of the first timestamp in the window
            window_end_time_index: The index of the last timestamp in the window
    '''
    temp = min(filtered_arr[window_start_time_index: window_end_time_index])
    return window_start_time_index + \
        np.where(filtered_arr[window_start_time_index: window_end_time_index] == temp)[
            0][0]


def getRAPDScore(filtered_right,
                 filtered_left,
                 illumination_change_timestamp,
                 reps,
                 isRightEyeFirstIllum,
                 isPreviousIllumDark
                 ):
    '''
        Returns the RAPD score of the given data.
        Arguments:
            filtered_right: The filtered right eye data
            filtered_left: The filtered left eye data
            illumination_change_timestamp: The timestamps (index) of the illumination changes
            reps: The number of illumination changes
            isRightEyeFirstIllum: True if the right eye is illuminated first
    '''
    max_filtered_right, min_filtered_right = [], []
    max_filtered_left, min_filtered_left = [], []
    max_filtered_right_timestamp, min_filtered_right_timestamp = [], []
    max_filtered_left_timestamp, min_filtered_left_timestamp = [], []
    if isPreviousIllumDark:
        start_index = 2  # The first two illumination changes are not considered as the value drops are high
    else:
        start_index = 0
    for i in range(start_index, 6):
        max_filtered_right.append(calculateWindowPeak_val(
            filtered_right, illumination_change_timestamp[i]))
        max_filtered_left.append(calculateWindowPeak_val(
            filtered_left, illumination_change_timestamp[i]))
        min_filtered_right.append(calculateWindowValley_val(
            filtered_right, illumination_change_timestamp[i], illumination_change_timestamp[i+1]))
        min_filtered_left.append(calculateWindowValley_val(
            filtered_left, illumination_change_timestamp[i], illumination_change_timestamp[i+1]))

        max_filtered_right_timestamp.append(calculateWindowPeak_val_timestamp(
            filtered_right, illumination_change_timestamp[i]))
        max_filtered_left_timestamp.append(calculateWindowPeak_val_timestamp(
            filtered_left, illumination_change_timestamp[i]))
        min_filtered_right_timestamp.append(calculateWindowValley_val_timestamp(
            filtered_right, illumination_change_timestamp[i], illumination_change_timestamp[i+1]))
        min_filtered_left_timestamp.append(calculateWindowValley_val_timestamp(
            filtered_left, illumination_change_timestamp[i], illumination_change_timestamp[i+1]))

    CA_right, CA_left = [], []
    if isRightEyeFirstIllum:
        for i in range(0, len(max_filtered_right), 2):
            CA_right.append(
                (max_filtered_right[i]-min_filtered_right[i])/max_filtered_right[i])
            CA_left.append(
                (max_filtered_left[i+1]-min_filtered_left[i+1])/max_filtered_left[i+1])
    else:
        for i in range(0, len(max_filtered_right), 2):
            CA_right.append(
                (max_filtered_right[i+1]-min_filtered_right[i+1])/max_filtered_right[i+1])
            CA_left.append(
                (max_filtered_left[i]-min_filtered_left[i])/max_filtered_left[i])

    RAPD_score = 10 * math.log(np.mean(CA_right)/np.mean(CA_left))
    return RAPD_score, max_filtered_right, min_filtered_right, \
        max_filtered_left, min_filtered_left, \
        max_filtered_right_timestamp, min_filtered_right_timestamp, \
        max_filtered_left_timestamp, min_filtered_left_timestamp


def getIlluminationChangeTimestamp_previousMethod(df):
    '''
        Returns the timestamps (index) of the illumination changes.
        Does not work when the left eye is illuminated first.
        Arguments:
            df: The dataframe of the data
    '''
    illumination_change_timestamp = [i for i in range(
        1, len(df)) if df.iloc[i, 3] != df.iloc[i-1, 3]]
    illumination_change_timestamp.append(len(df) - 1)
    return illumination_change_timestamp


def getIlluminationChangeTimestamp(df):
    '''
        Returns the timestamps (index) of the illumination changes.
        Should work when the left eye is illuminated first.
        Arguments:
            df: The dataframe of the data
    '''
    illumination_change_timestamp = [i for i in range(
        1, len(df)) if df.iloc[i, 3] != df.iloc[i-1, 3]]
    illumination_change_timestamp.append(len(df) - 1)
    for i in range(1, len(df)):
        if df.iloc[i, 1] != df.iloc[i-1, 1] and i not in illumination_change_timestamp:
            illumination_change_timestamp.append(i)
    return illumination_change_timestamp


def initialPreprocessFile(file_name,
                          dir_saved_files):
    '''
        Returns the preprocessed data and performs initial preprocessing.
        Arguments:
            file_name: The name of the file
            dir_saved_files: The directory of the saved files
    '''
    
    if ".csv" not in file_name:
        raise Exception("File is not a csv file")
    if os.path.isfile(os.path.join(dir_saved_files, file_name)) is False:
        raise Exception("File does not exist")
    df = pd.read_csv(os.path.join(dir_saved_files, file_name))
    df = df.dropna()
    df = df.astype(float)

    df = df[df.Pupil_Diameter_Right != -1]
    df = df[df.Pupil_Diameter_Left != -1]
    df = df.reset_index(drop=True)
    df['index'] = df.index

    intensity_left = list(df['Intensity_Left'])
    intensity_right = list(df['Intensity_Right'])

    # Gaussian filter with sigma = 6
    filtered_right = gaussian_filter1d(df['Pupil_Diameter_Right'], 6)
    filtered_left = gaussian_filter1d(df['Pupil_Diameter_Left'], 6)

    return df, filtered_right, filtered_left, intensity_right, intensity_left


def getIlluminationChangeValsTimestamps(df, intensity_right, intensity_left):
    '''
        Returns the timestamps (index) of the illumination changes and the values of the illumination changes.
        Arguments:
            df: The dataframe of the data
            intensity_right: The intensity list of the right eye
            intensity_left: The intensity list of the left eye
    '''
    curr_intensity_duo = (0, 0)
    arr_illum_start_end = []
    for idx, (v1, v2) in enumerate(zip(intensity_right, intensity_left)):
        if (v1, v2) != curr_intensity_duo:
            curr_intensity_duo = (v1, v2)
            arr_illum_start_end.append(
                (curr_intensity_duo, idx, df['TimeStamp'][idx]))
    arr_illum_start_end.append((curr_intensity_duo, idx, df['TimeStamp'][idx]))
    return arr_illum_start_end


def getHashmapOfIlluminationLevelsTimestamps(arr_illum_start_end):
    '''
        Returns a hashmap of the illumination levels, 
            their corresponding timestamps,
            if the previous illumination dark,
        Returns a second value which is number of repitions.
        Hashmap example:
            key: 1.0_0.0_0.0_1.0 and value: [[585, 835, 1085, 1335, 1585, 1836, 2084], True]
            The key represents the illumination level order (right -> left -> right -> left)
            Initially right eye is illuminated (the first 2 values in the key),
            then left eye is illuminated (the last 2 values in the key)
        Arguments:
            arr_illum_start_end: The array of the illumination levels and their corresponding timestamps
    '''
    hm_illum = {}
    curr_arr_idx = 0
    is_prev_illum_dark = True
    while curr_arr_idx < len(arr_illum_start_end) - 1:
        (start_, end_), idx, timestamp = arr_illum_start_end[curr_arr_idx]
        if (start_, end_) == (0, 0):
            is_prev_illum_dark = True
            curr_arr_idx += 1
            continue
        (start_next, end_next), _, _ = arr_illum_start_end[curr_arr_idx + 1]
        curr_illum_lvl, curr_illum_lvl_idx = [
            (start_, end_), (start_next, end_next)], 0
        reps = 0  # Repetitions
        while curr_arr_idx + curr_illum_lvl_idx < len(arr_illum_start_end):
            if arr_illum_start_end[curr_arr_idx + curr_illum_lvl_idx][0] not in curr_illum_lvl:
                break
            reps += 1
            curr_illum_lvl_idx += 1
        reps //= 2  # Because 1 repetition means both left and right eye
        key = str(start_) + '_' + str(end_) + '_' + \
            str(start_next) + '_' + str(end_next)
        if key not in hm_illum:
            hm_illum[key] = [[]]
        hm_illum[key].append(is_prev_illum_dark)
        for i in range(reps * 2 + 1):
            # print(curr_arr_idx + i)
            hm_illum[key][0].append(arr_illum_start_end[curr_arr_idx + i][1])
        curr_arr_idx += reps * 2
        is_prev_illum_dark = False
    return hm_illum, reps


def getXaxisValueRAPDGraph(key):
    '''
        Returns the x-axis value for the RAPD graph.
        Here we assume that at least one illumination level is 1.0
        Example: '0.16_0.0_0.0_1.0'
        Arguments:
            key: The key of the hashmap of the illumination levels
    '''
    split_key = key.split('_')
    if split_key[0] == '1.0' or split_key[2] == '1.0':
        letfEyeVaries = True
    else:
        letfEyeVaries = False
    # If leftEyeVaries, then the RAPD Score will be on the right side of the graph
    # or positive value
    # with open('./configs/VRIllumVsTransmittance.json') as f:
    #     data_VR_illum = json.load(f)
    data_VR_illum = {
        "1.0" : 1,
        "0.3" : 0.5,
        "0.16": 0.25
    }
    for key in data_VR_illum:
        if key == '1.0':
            continue
        if letfEyeVaries:
            if split_key[1] == key or split_key[3] == key:
                return - round(math.log(data_VR_illum[key], 10), 2)
        else:
            if split_key[0] == key or split_key[2] == key:
                return round(math.log(data_VR_illum[key], 10), 2)
    return 0


def getRAPDGraph(RAPD_score_arr, file_name, saved_image_dir):
    '''
        Returns the RAPD graph.
        Arguments:
            RAPD_score_arr: The RAPD score list
                First Value: The x-axis value
                Second Value: The y-axis value
            file_name: The name of the file
            saved_image_dir: The directory to save the image
    '''
    temp_title_2 = 'RAPD_score_' + file_name[:-4] + '.png'
    fig = plt.figure(figsize = (10, 6))
    linear_model = np.polyfit(RAPD_score_arr[0], RAPD_score_arr[1], 1)
    linear_intersection = - (linear_model[1]/linear_model[0])

    ax = plt.subplot(1, 1, 1)
    x_axis_points = np.linspace(
        min(RAPD_score_arr[0]), max(RAPD_score_arr[0]), 100)
    ax.plot(x_axis_points, linear_model[0] *
            np.linspace(-0.6, 0.6, 100) + linear_model[1])
    ax.scatter(RAPD_score_arr[0], RAPD_score_arr[1])
    ax.axhline(y=0, color='r', linestyle='--')
    ax.axvline(x=linear_intersection, color='b', linestyle='--')
    x_ticks = np.append(ax.get_xticks(), linear_intersection)
    ax.set_xticks(x_ticks)
    ax.set_xlabel("Filter in log units")
    ax.set_ylabel("RAPDx score in log units")
    ## OD/OS
    if linear_intersection >= 0:
        temp_title_ODOS =  file_name[:-4] + ", RAPD_score: " + str("{:.2f}".format(linear_intersection)) + ", OD (Right Eye)"
    else:
        temp_title_ODOS =  file_name[:-4] + ", RAPD_score: " + str("{:.2f}".format(linear_intersection)) + ", OS (Left Eye)"

    ax.set_title(temp_title_ODOS)
    fig.savefig(f"{saved_image_dir}/"+temp_title_2, dpi = 300)
    plt.ioff()

    return temp_title_2


def getDiameterandRAPDGraph(RAPD_score_arr,
                            file_name,
                            df,
                            filtered_right,
                            filtered_left,
                            all_filtered_timestamp_to_df, 
                            all_filtered,
                            saved_image_dir
                            ):
    '''
        Returns the Diameter and RAPD graph.
        Arguments:
            RAPD_score_arr: The RAPD score list
                First Value: The x-axis value
                Second Value: The y-axis value
            file_name: The name of the file
            df: The dataframe of the file
            filtered_right: The filtered right pupil diameter list
            filtered_left: The filtered left pupil diameter list
            all_filtered_timestamp_to_df: The list of the filtered timestamp to the dataframe
            all_filtered: The list of the filtered timestamp
            saved_image_dir: The directory to save the image
    '''
    illumination_change_timestamp = getIlluminationChangeTimestamp(df)

    temp_title = 'Pupil_Diameter_image_' + file_name[:-4] + '.png'
    temp_title_2 = 'RAPD_score_' + file_name[:-4] + '.png'

    fig = plt.figure(figsize = (12, 8))
    ax = plt.subplot(2, 1, 1)
    ax.plot(df['index'], filtered_left, label="Left Pupil")
    ax.plot(df['index'], filtered_right, label="Right Pupil")
    for i in range(len(illumination_change_timestamp)):
        temp = illumination_change_timestamp[i]
        if i < len(illumination_change_timestamp) - 2:
            if i % 2 == 0:
                ax.axvline(x=df['index'][temp], color='r', linestyle='--')
            else:
                ax.axvline(x=df['index'][temp], color='g', linestyle='--')
        else:
            if i % 2 == 0:
                ax.axvline(x=df['index'][temp], color='r',
                           linestyle='--', label='Right Illumination')
            else:
                ax.axvline(x=df['index'][temp], color='g',
                           linestyle='--', label='Left Illumination')
    ax.scatter(all_filtered_timestamp_to_df, all_filtered, color='black')
    ax.set_xlabel("TimeStamp", fontsize=16)
    ax.set_ylabel("Pupil Diameter", fontsize=16)
    ax.set_title(temp_title[:-4])
    ax.legend()

    ax = plt.subplot(2, 1, 2)
    linear_model = np.polyfit(RAPD_score_arr[0], RAPD_score_arr[1], 1)
    linear_intersection = - (linear_model[1]/linear_model[0])
    x_axis_points = np.linspace(
        min(RAPD_score_arr[0]), max(RAPD_score_arr[0]), 100)
    ax.plot(x_axis_points, linear_model[0] *
            x_axis_points + linear_model[1])
    ax.scatter(RAPD_score_arr[0], RAPD_score_arr[1])
    ax.axhline(y=0, color='r', linestyle='--')
    ax.axvline(x=linear_intersection, color='b', linestyle='--')
    x_ticks = np.append(ax.get_xticks(), linear_intersection)
    ax.set_xticks(x_ticks)

    ## OD/OS
    if linear_intersection >= 0:
        temp_title_ODOS =  file_name[:-4] + ", RAPD_score: " + str("{:.2f}".format(linear_intersection)) + ", OD (Right Eye)"
    else:
        temp_title_ODOS =  file_name[:-4] + ", RAPD_score: " + str("{:.2f}".format(linear_intersection)) + ", OS (Left Eye)"

    ax.set_title(temp_title_ODOS)
    ax.set_xlabel("Filter in log units")
    ax.set_ylabel("RAPDx score in log units")
    plt.tight_layout()

    fig.savefig(f"{saved_image_dir}/"+temp_title_2, dpi = 300)
    plt.ioff()

    return temp_title_2


def getRAPDGraphPoints(hm_illum,
                       reps,
                       filtered_right,
                       filtered_left
                       ):
    '''
        Returns the RAPD graph points.
        Arguments:
            hm_illum: The hashmap of the illumination levels and their corresponding timestamps
            reps: The number of repetitions
            filtered_right: The filtered right eye data list
            filtered_left: The filtered left eye data list
    '''
    RAPD_score_arr = [[], []]
    RAPD_score_hm = {}
    all_filtered_timestamp_to_df = []
    all_filtered = []
    for key in hm_illum:
        isRightEyeFirstIllum = isRightEyeFirstIlluminated(key)
        temp_score, max_filtered_right, min_filtered_right, \
            max_filtered_left, min_filtered_left, \
            max_filtered_right_timestamp, min_filtered_right_timestamp, \
            max_filtered_left_timestamp, min_filtered_left_timestamp = getRAPDScore(filtered_right=filtered_right,
                                                                                    filtered_left=filtered_left,
                                                                                    illumination_change_timestamp=hm_illum[
                                                                                        key][0],
                                                                                    reps=reps,
                                                                                    isRightEyeFirstIllum=isRightEyeFirstIllum,
                                                                                    isPreviousIllumDark=hm_illum[key][1])
        for arr in [max_filtered_right, min_filtered_right, max_filtered_left, min_filtered_left]:
            all_filtered.extend(arr)
        for arr in [max_filtered_right_timestamp, min_filtered_right_timestamp, max_filtered_left_timestamp, min_filtered_left_timestamp]:
            all_filtered_timestamp_to_df.extend(arr)
        x_axis_val = getXaxisValueRAPDGraph(key)
        if x_axis_val not in RAPD_score_hm:
            RAPD_score_hm[x_axis_val] = []
        RAPD_score_hm[x_axis_val].append(temp_score)
    for x_axis_val in RAPD_score_hm:
        if len(RAPD_score_hm[x_axis_val]) > 1:
            temp_score = np.mean(RAPD_score_hm[x_axis_val])
        else:
            temp_score = RAPD_score_hm[x_axis_val][0]
        RAPD_score_arr[0].append(x_axis_val)
        RAPD_score_arr[1].append(temp_score)

    return RAPD_score_arr, all_filtered_timestamp_to_df, all_filtered
