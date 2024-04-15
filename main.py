from utils.helperFunctions import *
import sys
import os
from PIL import Image


def main(fn, dir_saved_files, saved_image_dir, show_diameter_plot=True):
    df, filtered_right, filtered_left, intensity_right, intensity_left = (
        initialPreprocessFile(file_name=fn, dir_saved_files=dir_saved_files)
    )

    arr_illum_start_end = getIlluminationChangeValsTimestamps(
        df=df, intensity_right=intensity_right, intensity_left=intensity_left
    )

    hm_illum, reps = getHashmapOfIlluminationLevelsTimestamps(arr_illum_start_end)

    RAPD_score_arr, all_filtered_timestamp_to_df, all_filtered = getRAPDGraphPoints(
        hm_illum=hm_illum,
        reps=reps,
        filtered_right=filtered_right,
        filtered_left=filtered_left,
    )
    if len(RAPD_score_arr[1]) == 0:
        raise Exception("No RAPD score found")
    if len(RAPD_score_arr[1]) == 1:
        print(RAPD_score_arr[1][0])
        return

    try:
        if show_diameter_plot:
            saved_img_fn = getDiameterandRAPDGraph(
                RAPD_score_arr=RAPD_score_arr,
                file_name=fn,
                df=df,
                filtered_right=filtered_right,
                filtered_left=filtered_left,
                all_filtered_timestamp_to_df=all_filtered_timestamp_to_df,
                all_filtered=all_filtered,
                saved_image_dir=saved_image_dir,
            )
        else:
            saved_img_fn = getRAPDGraph(
                RAPD_score_arr=RAPD_score_arr,
                file_name=fn,
                saved_image_dir=saved_image_dir,
            )
    except Exception as e:
        print(e)
        print("Error in plotting graph")
        return

    ## Open the image
    im = Image.open(f"{saved_image_dir}/" + saved_img_fn)
    im.show()


if __name__ == "__main__":
    ## Get arguments
    args = sys.argv
    if len(args) < 2:
        dir_csv_input = input(
            "Enter current working directory and csv file name (with a space in between): "
        )
        dir_csv_input = dir_csv_input.split(" ")
        cwd = "".join(dir_csv_input[0:-1])
        dir_csv_file = dir_csv_input[-1]
    else:
        cwd = " ".join(v for v in args[1:-1])
        dir_csv_file = args[-1]

    ## Directory name
    # cwd = os.getcwd()
    print("cwd: ", cwd)
    print(".csv file: ", dir_csv_file)

    dir_saved_files = os.path.join(cwd, "RAPD", "Saved", "Processed_Data")
    saved_image_dir = os.path.join(cwd, "RAPD", "Saved", "Processed_Data", "Images")

    if os.path.isdir(saved_image_dir) is False:
        os.mkdir(saved_image_dir)

    main(
        fn=dir_csv_file,
        dir_saved_files=dir_saved_files,
        saved_image_dir=saved_image_dir,
        show_diameter_plot=True,
    )
