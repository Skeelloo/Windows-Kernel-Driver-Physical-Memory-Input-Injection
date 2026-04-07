# ⚙️ Abyss Kernel Driver
> An advanced, open-source Windows kernel driver designed for memory subversion, physical memory translation, and input injection.

[![C++](https://img.shields.io/badge/Language-C++-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows-lightgray.svg)](https://www.microsoft.com/en-us/windows/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Build](https://img.shields.io/badge/Build-Visual_Studio_2022-blueviolet.svg)](#)

---

## 📖 Overview

This repository contains an experimental **Windows Kernel Driver** focused on secure memory interaction and system subversion bypassing traditional user-mode hooks. 

The driver heavily utilizes physical memory translation (`CR3`), unhooked peripheral input via `MouClass`, and dynamic import mapping to maintain maximum stealth against standard operating system checks.

> **Disclaimer:** This project is intended for educational and research operations regarding the Windows Kernel model. Any usage in unintended environments is at your own risk.

---

## ⚡ Core Features

* **Physical Memory Translation:** Full CR3 translation layer with support for walking the PML4 down to physical pages.
* **Dynamic Import Resolution:** Functions like `MmCopyMemory`, `MmMapIoSpaceEx`, and `MmUnmapIoSpace` are mapped dynamically during execution to reduce static detection surfaces.
* **MouClass Input Injection:** Absolute stealth mouse movements bypassing the standard `mouse_event` API queue by directly interfacing with driver extensions.
* **Kernel Read/Write Interfacing:** Secure arbitrary memory read/write via physical maps.
* **Dynamic Version Support:** Automatically updates internal kernel offsets on the fly, supporting `Windows 10` up through `Windows 11 23H2+`.
* **Hardware ID Obfuscation (`spoof.h`):** Contains structures and operations to assist with MAC and HWID emulation.

---

## 🛠️ Build Instructions

### Prerequisites
* Windows 10 (1803+) or Windows 11 (Up to 23H2).
* Visual Studio 2022 with C++ Development Workloads.
* Windows Driver Kit (WDK) installed.

### Compilation
1. **Clone the Repository:**
   ```bash
   git clone https://github.com/your-username/abyss-kernel-driver.git
   cd abyss-kernel-driver
   ```

2. **Build the Solution:**
   * Open `driver.vcxproj` in Visual Studio.
   * Target architecture `x64` and set build mode to `Release`.
   * Compile the project. The output `.sys` will automatically generate.

---

## 🚀 Loading the Driver

Because this driver is un-signed, it cannot be loaded via standard methods like `sc create`. You will need to manually map the module into kernel space using a vulnerable driver exploit.

* **Recommended Mappers:** Utilize standard mapping technologies like `KDMapper` or `kdlib`.
* **Prerequisites:** Ensure **Core Isolation** and **Virtualization-Based Security (VBS)** are disabled in Windows Security before attempting to map physical memory.

---

## 📌 Notice
Feel free to fork or open a pull request! Make sure your kernel modifications are extensively BSOD-tested and stable across multiple Windows builds before pushing.
