# About
- Installer project for the [Labrador_ImGui application](https://github.com/jdar0011/Labrador_ImGui) 
- Installer is built using the [Wix Toolset](https://wixtoolset.org/) v3.11
# Visual Studio 2022
- You will need to download [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) (although you should already have it if you have been working on Labrador_ImGui)
# WiX Toolset
- You will need to download the WiX extension for Visual Studio. In Visual Studio, go to Extensions->Manage Extensions and search Online for WiX v3 - Visual Studio 2022 Extension, and download and install.
- You will also need to download and install the WiX toolset from [here](https://github.com/wixtoolset/wix3/releases/tag/wix3112rtm). Download wix311.exe and run, and click Install.
# Building the Installer
- Open LabradorInstaller.sln in Visual Studio 2022
- Build on the x64 Release configuration
- The installer, LabradorInstaller.msi, can be found in LabradorInstaller/bin/Release
# Configuring the Installation
- This only needs to be considered if files are added to the project that need to be included in the Program Files folder
- The instructions for the installer are in LabradorInstaller.wxs, which is written in XML and tells WiX how to build the Installer
- If any files are added to the project that Labrador_ImGui.exe will need to execute, these need to be referenced in the wxs file. 
- It may be worth looking into the [documentation for the WiX Toolset](https://wixtoolset.org/docs/intro/) to understand out how this is done, or you can try to follow what has been done in the file.
- The files in Labrador_ImGui repository will be the exact files that are copied into Program Files/Labrador after installation
- The requirements folder contains the .exe which installs the Labrador device driver (Driver_Installer.exe). This will run at the beginning of the installation prior to installation of the Labrador application as a [Custom Action](https://wixtoolset.org/docs/v3/xsd/wix/customaction/)
# Possible Improvements
- Another thing that could be done, which would involve work in the Labrador_ImGui repo, would be to bundle all data files inside the Labrador_ImGui.exe, so that we don't have to load into Program Files. We could also run the Driver Installer inside the Labrador_ImGui.exe, meaning we wouldn't need an installer at all, we could just distribute the Labrador_ImGui.exe, increasing portability.
 
