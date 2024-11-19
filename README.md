# VisionScopePro

## Overview
VisionScopePro is a cutting-edge extended reality (XR) platform developed under the Human-Machine Perception Lab at the University of Nevada, Reno. This project integrates a suite of ophthalmic diagnostic tools, enabling a seamless transition between tests in XR environments. It leverages real-time eye-tracking and patient data management to deliver highly accurate vision assessments. The system is specifically designed for Pico 4 Enterprise headsets and is targeted toward enhancing accessibility and efficiency in vision diagnostics.

## Purpose
VisionScopePro is designed to modernize ophthalmic diagnostic testing and vision screening by incorporating XR technology. With a focus on underserved regions, the system provides a portable, cost-effective, and comprehensive solution for early detection of eye conditions.

### Key Objectives:
- **Dynamic Test Flow**: Unified framework for transitioning between tests, with automated test descriptions and intuitive menu navigation.
- **Comprehensive Vision Screening**: A robust suite of XR-based tests, including RAPD, visual field, dynamic visual acuity (DVA), and color vision assessments.
- **Real-Time Eye-Tracking**: Advanced eye movement analysis for more accurate diagnostics.
- **Data Management**: Patient profiles and test results are logged, tracked, and exportable for longitudinal analysis and diagnosis.
- **User-Friendly Design**: Simplified workflows with clear explanations for each test, aiding patients and operators.

## Features
- **Automated Test Transitions**: Dynamic management of test sequences, including pre-test explanations and seamless transitions between tests such as RAPD, Static VA, Color Vision, Contrast Sensitivity, Visual Field, and DVA.
- **RAPD (Rapid Automated Pupil Test)**: Developed by Nasif Zaman with Prithul Sarker, this fully automated pupil reflex test has been enhanced with XR capabilities.
- **PeriMapXR (Automated Perimetry Test)**: Developed by JoJo Petersky, this visual field testing module adapted for XR environments will undergo clinical trials with results to be published upon completion.
- **Color Vision Testing**: Includes both Ishihara plates and advanced cone contrast testing for precision analysis.
- **Dynamic Visual Acuity (DVA)**: Head-mounted XR-based testing developed as part of a NASA-funded initiative to detect vision changes from spaceflight, validated against traditional methods.
- **Contrast Sensitivity Testing**: Evaluates a patient’s ability to detect subtle contrasts in visual stimuli.
- **Interactive Amsler Grid**: Assists in detecting macular defects through direct user interaction.
- **Patient Tracking System**: Stores patient profiles, including test history, timestamps, and result data for each assessment.

## Integration of Recent Research
VisionScopePro incorporates methodologies and tests from cutting-edge research, including:
- **RAPD Testing**: Based on research by Nasif Zaman and Prithul Sarker, *"Test–Retest Reliability of Virtual Reality Devices in Quantifying for Relative Afferent Pupillary Defect"* (Translational Vision Science & Technology, June 2023). This study demonstrated the feasibility of automating the Swinging Flashlight Test (SFT) using virtual reality, ensuring reproducibility and reliability across test sessions and devices.
- **Color Deficiency Testing and Aid**: Incorporates findings from XR-based studies on color-deficient observers (Nasif Zaman and Alireza Tavakkoli, Optica Fall Vision Meeting 2023).
- **Dynamic Visual Acuity (DVA) Assessment**: Integrates findings from studies on XR-based DVA testing to address visual-vestibular conflict and spaceflight-induced sensorimotor adaptations (Nasif Zaman et al., NASA-funded research).

## Installation
VisionScopePro will be distributed via APK files available in the repository's Releases section.

### Prerequisites
To run VisionScopePro, you will need:
- **Pico 4 Enterprise headset**: Ensure the device is fully charged and set up with the Pico SDK installed.
- **File Transfer Software**: Upload the APK via USB connection on the PICO Developer app.
- **Unreal Engine 5 (optional)**: Required if you plan to modify the codebase or rebuild the application.

### Setup
Detailed instructions for setting up VisionScopePro will be available upon the project's next development milestone.

### Notes
- For updates, check the Releases section regularly for the latest APKs.
- Contact the project lead for troubleshooting or additional setup assistance.

## Contributing
VisionScopePro is a proprietary project. Contributions are restricted to the Human-Machine Perception Lab team. External collaborations are welcomed through prior approval from the project lead or advisor.

## License
VisionScopePro is proprietary software. All rights are reserved by the Human-Machine Perception Lab at the University of Nevada, Reno. Unauthorized copying, modifying, distribution, or use of this software is strictly prohibited without explicit permission from our contributors or the HMP Lab.

## Acknowledgments
This project is supported by NASA grants and the collaborative efforts of the Human-Machine Perception Lab at UNR. Special thanks to Nasif Zaman, Prithul Sarker, and other collaborators for their contributions to RAPD and DVA testing research.

## Contact
For more information, inquiries, or feedback, please contact:

**Project Lead**: JoJo Petersky  
- GitHub: [@hotcuppajojo](https://github.com/hotcuppajojo)  
- Email: [jpetersky@unr.edu](mailto:jpetersky@unr.edu)  
- LinkedIn: [https://www.linkedin.com/in/jojopetersky/](https://www.linkedin.com/in/jojopetersky/)

**Advisor**: Dr. Alireza Tavakkoli  
- Phone: (775) 682-8426  
- Email: [tavakkol@unr.edu](mailto:tavakkol@unr.edu)  
- Website: [https://www.cse.unr.edu/~alit/](https://www.cse.unr.edu/~alit/)

**RAPD/Data Analysis Contributor**: Prithul Sarker
- GitHub: [@prithuls](https://github.com/prithuls)  
- Email: [prithulsarker@unr.edu](mailto:prithulsarker@unr.edu)
- LinkedIn: [https://www.linkedin.com/in/prithulsarker/](https://www.linkedin.com/in/prithulsarker/)

**RAPD/CS/Color/VA/Amsler Contributor**: Nasif Zaman
- GitHub: [@Znasif](https://github.com/Znasif)  
- Email: [vznasif@gmail.com](mailto:vznasif@gmail.com)
- LinkedIn: [https://www.linkedin.com/in/nasif-zaman/](https://www.linkedin.com/in/nasif-zaman/)