import argparse
import logging
from pathlib import Path

import numpy as np
import pandas as pd
from bokeh.io import output_notebook
from bokeh.layouts import gridplot
from bokeh.models import (
    BoxZoomTool,
    ColumnDataSource,
    CustomJS,
    HoverTool,
    PanTool,
    ResetTool,
    SaveTool,
    Slider,
    Span,
    WheelZoomTool,
)
from bokeh.plotting import figure, output_file, save, show

from utils import (
    EYE_DATA_DTYPE,
    EYE_DATA_KEYS,
    ecxlude_invalid_data_at_begining_and_end,
    process_pupil_diameters,
)


def main():
    """
    How to run the script:
        python visualize_rapd_csv.py \
            --csv-path /home/reza/workspace/projects/RAPD/scripts/visualization/example_rapd.csv \
            --html-path /home/reza/workspace/projects/RAPD/scripts/visualization/example_rapd.html
    """
    parser = argparse.ArgumentParser("Visualize RAPD csv file.")
    parser.add_argument(
        "--csv-path", type=Path, required=True, help="Path to csv file."
    )
    parser.add_argument(
        "--html-path",
        type=Path,
        default=None,
        help="Path to html file. If not provided rapd.html file will be saved in the csv file folder.",
    )
    args = parser.parse_args()

    if not args.csv_path.exists():
        raise ValueError(f"{args.csv_path} does not exist!")

    if args.html_path is None:
        args.html_path = args.csv_path.with_suffix(".html")

    tools = [
        WheelZoomTool(),
        BoxZoomTool(dimensions="width"),
        PanTool(dimensions="width"),
        SaveTool(),
        ResetTool(),
    ]

    logging.info(f"Reading {args.csv_path}")
    df = pd.read_csv(args.csv_path)

    data = np.zeros(len(df), dtype=EYE_DATA_DTYPE)
    for key in EYE_DATA_KEYS:
        data[key] = df[key]

    data = ecxlude_invalid_data_at_begining_and_end(data)
    data["Filtered_Pupil_Diameter_Left"] = process_pupil_diameters(
        data["TimeStamp"], data["Pupil_Diameter_Left"], 0.0
    )
    data["Filtered_Pupil_Diameter_Right"] = process_pupil_diameters(
        data["TimeStamp"], data["Pupil_Diameter_Right"], 0.0
    )

    # Convert DataFrame to a ColumnDataSource
    data_cd = ColumnDataSource(data={k: data[k].tolist() for k in EYE_DATA_DTYPE.names})
    highlight_cd = ColumnDataSource(
        data={
            k: [data[k][0]] for k in ["TimeStamp", "Intensity_Left", "Intensity_Right"]
        }
    )

    # Create a new plot with a title and axis labels
    p1 = figure(
        width=1600,
        height=400,
        title="Left and Right Pupil Diameter (mm) Over Time",
        x_axis_label="timestamp",
        tools=tools,
    )

    # Add line renderer with legend and line thickness
    colors = ["blue", "cyan", "red", "magenta"]
    QUERY_KEYS = [
        "Filtered_Pupil_Diameter_Left",
        "Pupil_Diameter_Left",
        "Filtered_Pupil_Diameter_Right",
        "Pupil_Diameter_Right",
    ]
    for i, key in enumerate(QUERY_KEYS):
        obj = p1.line(
            x="TimeStamp",
            y=key,
            source=data_cd,
            legend_label=key.replace("_", " "),
            line_width=2,
            color=colors[i],
        )
        # Hide raw pupil diameters by default.
        obj.visible = key in [
            "Filtered_Pupil_Diameter_Left",
            "Filtered_Pupil_Diameter_Right",
        ]
    hover = HoverTool(tooltips=[(k.replace("_", " "), "@" + k) for k in QUERY_KEYS])
    p1.add_tools(hover)

    # Add vertical line to the plot
    vline1 = Span(
        location=data["TimeStamp"][0],
        dimension="height",
        line_color="black",
        line_width=1,
    )
    p1.add_layout(vline1)

    # Create a new plot with a title and axis labels
    p2 = figure(
        width=1600,
        height=250,
        title="Left and Right Screeen Light Over Time",
        x_axis_label="timestamp",
        tools=tools,
        x_range=p1.x_range,
    )

    colors = ["blue", "red"]
    for i, key in enumerate(
        [
            "Intensity_Left",
            "Intensity_Right",
        ]
    ):
        p2.line(
            x="TimeStamp",
            y=key,
            source=data_cd,
            legend_label=key.replace("_", " "),
            line_width=2,
            color=colors[i],
        )
        p2.scatter(x="TimeStamp", y=key, source=highlight_cd, color=colors[i], size=10)

    # Add vertical line to the plot
    vline2 = Span(
        location=data["TimeStamp"][0],
        dimension="height",
        line_color="black",
        line_width=1,
    )
    p2.add_layout(vline2)

    # Create a slider
    slider = Slider(
        start=0,
        end=len(df) - 1,
        value=0,
        step=1,
        title="Time Index",
        sizing_mode="stretch_width",
    )

    # Define a callback to update the graph when the slider changes
    highlight_callback = CustomJS(
        args=dict(
            source=highlight_cd,
            time=data["TimeStamp"].tolist(),
            intensity_left=data["Intensity_Left"].tolist(),
            intensity_right=data["Intensity_Right"].tolist(),
        ),
        code="""
        var index = cb_obj.value;
        source.data['TimeStamp'][0] = time[index];
        source.data['Intensity_Left'][0] = intensity_left[index];
        source.data['Intensity_Right'][0] = intensity_right[index];
        source.change.emit();
    """,
    )
    slider.js_on_change("value", highlight_callback)

    # Callback to update vertical lines on slider value change
    line_callback = CustomJS(
        args=dict(vline1=vline1, vline2=vline2, times=data["TimeStamp"]),
        code="""
        const t = times[cb_obj.value];
        vline1.location = t;
        vline2.location = t;
    """,
    )
    slider.js_on_change("value", line_callback)

    output_notebook()
    p1.legend.click_policy = "hide"
    p2.legend.click_policy = "hide"

    grid = gridplot([[p1], [p2], [slider]])

    logging.info(f"Saving {args.html_path}")
    output_file(filename=args.html_path, title="RAPD Data")
    save(grid)


if __name__ == "__main__":
    logging.getLogger().setLevel(logging.INFO)
    main()
