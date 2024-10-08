from typing import List

import numpy as np

# Enable the output of the plot in the notebook
from scipy import signal
from scipy.interpolate import interp1d

INVALID_PUPIL_DIAMETER = -1

EYE_DATA_KEYS = (
    "TimeStamp",
    "Intensity_Left",
    "Pupil_Diameter_Left",
    "Intensity_Right",
    "Pupil_Diameter_Right",
)

EYE_DATA_DTYPE = np.dtype(
    {
        "names": EYE_DATA_KEYS
        + ("Filtered_Pupil_Diameter_Left", "Filtered_Pupil_Diameter_Right"),
        "formats": ("f8", "f8", "f8", "f8", "f8", "f8", "f8"),
    }
)


def interpolate_pupil(times: np.ndarray, diameters: np.ndarray) -> np.ndarray:
    mask = diameters != -1
    interpolation_func = interp1d(times[mask], diameters[mask], kind="linear")
    interpolated_diameters = diameters
    interpolated_diameters[~mask] = interpolation_func(times[~mask])
    return interpolated_diameters


def normalize(x: np.ndarray) -> np.ndarray:
    return (x - x.min()) / (x.max() - x.min())


def process_pupil_diameters(
    times: List[float], pupil_diameters: List[float], offset: float
) -> List[float]:
    interpolated_diameters = interpolate_pupil(
        np.array(times), np.array(pupil_diameters)
    )
    filtered_interpolated_diameters = signal.savgol_filter(
        interpolated_diameters, window_length=21, polyorder=3, mode="nearest"
    )
    return filtered_interpolated_diameters


def ecxlude_invalid_data_at_begining_and_end(eye_data):
    valid_pupil_diameters = (
        eye_data["Pupil_Diameter_Left"] != INVALID_PUPIL_DIAMETER
    ) & (eye_data["Pupil_Diameter_Right"] != INVALID_PUPIL_DIAMETER)
    inds = np.where(valid_pupil_diameters)[0]
    start, end = inds[0], inds[-1] + 1
    return eye_data[start:end]
