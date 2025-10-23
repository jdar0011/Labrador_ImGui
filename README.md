# About

- This application is a control interface for the [Espotek Labrador Portable Lab Bench](https://espotek.com/labrador/)
- The primary goals of the design it to be student friendly and accessible, meaning that advanced features are not present
- The original application for the board can be found [here](https://github.com/EspoTek/Labrador)
- Developed as an ECSE Final Year Project at Monash University
<p align="center">
  <img src="https://github.com/user-attachments/assets/72207e83-18da-40ee-9f17-82211c3f7350" alt="labrador-controller-home-page" width="750"/>
</p>

# Installation

### Windows
- v1.0.0 Release (October 2024) found [here](https://github.com/jdar0011/Labrador_ImGui/releases/tag/v.1.0.0)
- Download and run .msi file, application and relevant driver will be installed.

### macOS
- Beta Release (October 2025) found [here](https://github.com/jdar0011/Labrador_ImGui/releases/tag/Beta)
- Download and open .dmg file, code is current **unsigned** so macOS makes it difficult to open
- After dragging the LabradorImgui.app into the Application folder, run this line of code to allow the app to open (works on macOS Sequoia V15.6)
```
sudo xattr -cr /Applications/LabradorImgui.app
```

# Running the Code

### Windows
- This application is developed with Visual Studio 2022
- Clone the respository using Visual Studio 2022 to build and run the latest variant of the application

### macOS
- Download cmake (if you have homebrew (https://brew.sh/) the command is:)
```
brew install cmake
```
- Run cmake commands through package_mac.sh to build the app bundle by running the below commands from the repository top-level directory:
```
chmod +x package_mac.sh
./package_mac.sh
```
- Executable can be run via
```
./mac_appbundle/LabradorImgui.app/Contents/MacOS/LabradorImgui
```
- or you can open LabradorImgui.app in the mac_appbundle folder

# User Documentation

### General

#### About the board

- This application controls the EspoTek Labrador Board.
- Not all pins are utilised by this application. See Pinout below for a diagram showing the location of all pins used by this application.
- All Ground (GND) Pins are connected i.e. all GND connections offer a common reference point.
- [TODO: INSERT EXPLANATION FOR AC COUPLING SITTING at ~1.6 V]

### Troubleshooting

#### Safety Mode

- Safety Mode is indicated by the Labrador Board's LED flashing every 400 ms
- This will align with short blips on the Oscilloscope reading surrounded by null readings
- Fix this by unplugging the board for 30 seconds before plugging it back in

### Power Supply Unit (PSU)

#### Controls
- **Voltage control**: Adjusts the output voltage level
#### Specifications
- **Voltage Range**: 4.5 V - 12.0 V
- **Maximum Power Supply**: 0.75 W

### Signal Generator

#### Controls
- **Power button**: Turns the signal generator on or off.
- **Waveform selection**: Chooses the type of waveform output.
- **Frequency slider**: Sets the frequency of the signal.
- **Vpeak-peak slider**: Modifies the peak to peak amplitude of the waveform.
- **Offset slider**: Shifts the waveform's base voltage level
- **Duty Cycle slider**: Defines the percentage that the waveform is "on" (square wave only)

#### Specifications
- **Voltage Range**: 
  - 0.15 V to 9.5 V (DC Coupled)
  - -4.5 V to 4.5 V (AC Coupled)
- **Maximum Current**: 10 mA
- **Sample Rate**: 1 Msps

#### Additional Information
- **DC Coupled Pins**: Replicates the properties set in the application.
- **AC Coupled Pins**: No DC component meaning the offset set in the application may not have the desired effect.

### Plot Settings

#### Main Controls

- **Run / Stop**: Starts or stops the signal acquisition and display.
- **Auto Fits**: Sets zoom of plot to appropriate scale, based on calculated period for time axis or Vpp for voltage axis.

#### Display

- **Channel Switches**: Enables/disables respective channel reading and display.
- **Cursors**: Enables/disables measurement cursors. Both cursors must be active to display delta information under plot window.
- **Signal Properties**: Enables/disables signal property calculations displayed under plot window.

#### General Settings

- **Trigger**:
  - Aligns a 'trigger' event with the left most point in the plot window (e.g. OSC1 Rising)
  - **Level**: Sets the voltage level for triggering
- **Export**
  - To clipboard: Copies current data in time window to clipboard, can be easily pasted into Excel.
  - To CSV: Saves current data in time window to selected directory. Press Export button to choose directory.

#### Specifications

- **Sample Rate**: 750 ksps
- **Input Voltage Range**: -20 V to +20 V

#### Additional Information

- DC Coupled pins will read the raw voltages without filtering anything.
- AC Coupled pins filter out the very low frequency components (DC components) to display only AC signals

#### Math Mode
Syntax
- **Literals and Names**
  - Signals: osc1   osc2
  - Numbers: 3    -4.2    1e-3
  - Constants: pi   e
- **Operators**
  - Arithmetic: +   -   *   /   %   ^
  - Comparison: ==    !=    <   <=    >   >=
  - Logical: and    or    xor   not
- **Common Functions**
  - abs(x)    sgn(x)
  - floor(x)    ceil(x)   round(x)    trunc(x)    frac(x)
  - sqrt(x)   pow(x,y)
  - exp(x)    log(x)    log10(x)
  - sin(x)    cos(x)    tan(x)
  - asin(x)   acos(x)   atan(x)
  - sinh(x)   cosh(x)   tanh(x)
### Plot Window

#### Controls

- **Panning**:
  - Click and drag on the plot window to pan in any direction
  - Click and drag on an axis to pan in only that direction
- **Zooming**: 
  - Scroll to zoom anywhere on the plot
  - Scroll on an axis to zoom only on that axis

#### Measurments

- **Cursors**:
  - Activate cursors in Plot Settings
  - Measurements between the two cursors are displayed below the plot when they are both activated
  - Use these to measure the frequency/amplitude of displayed waveforms
- **Signal Properties**
  - Signal properties are calculated based on the plot window data
  - ∆T values represent an average time between trigger events (see more info in Plot Settings)
  - ∆V values represent the difference between Vmin and Vmax
### End
