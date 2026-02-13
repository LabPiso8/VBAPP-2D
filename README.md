# VBAPP-2D

VBAP MIDI Spatial Controller is a real-time spatial panning application based on the Vector Base Amplitude Panning (VBAP) algorithm. The project provides a two-dimensional graphical interface that allows users to position virtual sound sources within a circular space. The angular and radial coordinates of each source are calculated in real time and translated into MIDI data, including NRPN messages, enabling integration with professional digital mixing consoles.

The application is implemented in C++ using the JUCE framework. It supports real-time audio and MIDI device management and is designed with a modular and scalable architecture. The current implementation operates with the Yamaha LS9 digital mixing console, utilizing its MIDI control capabilities to map spatial parameters to console functions. Although compatibility is presently limited to this console, the internal design allows future adaptation to other digital mixing systems that support comparable MIDI-based control protocols.

Spatial processing is based on the VBAP algorithm developed by Ville Pulkki. VBAP distributes gain between adjacent loudspeakers using vector-based calculations to achieve precise spatial localization in multichannel environments. This approach extends beyond conventional stereo panning by supporting flexible loudspeaker configurations and more advanced immersive audio applications.

The system relies on the Eigen linear algebra library for matrix operations required by the VBAP gain calculations. In order to successfully compile the project, the Eigen directory must be manually added to the "Header Search Paths" section within the JUCE project configuration. This can be configured in Projucer or directly in the exported IDE project settings. Failure to include the Eigen path will result in compilation errors due to unresolved header dependencies. A C++17-compatible compiler is required.

The project is intended for research, live sound experimentation, immersive audio control, and professional multichannel workflows. Ongoing and future development may include expanded console compatibility, Open Sound Control (OSC) integration, enhanced spatial visualization tools, implementation of Virtual VBAP (VVBAP), and support for three-dimensional spatial control.

This repository is provided as an open technical framework for further development, experimentation, and integration within spatial audio systems.

