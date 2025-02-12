# BitStack CLI

BitStack is a command-line tool for encoding and decoding files using bit-depth compression. This repository includes the source code and build instructions.

## 🚀 Features
- Encode and decode files using bit-depth settings.
- Supports encoding/decoding entire folders.
- CLI-based usage.

---

## 📌 Prerequisites
Before building the project, ensure you have the following installed:
- **CMake** (Version 3.10+)
- **C++ Compiler** (GCC, Clang, or MSVC)
- **Git**
- **Visual Studio (for Windows)**
- **MinGW (for Windows, if using make)**

---

## 🛠️ Build Instructions

### **1️⃣ Clone the Repository**
```sh
git clone https://github.com/yourusername/BitStack.git
cd BitStack
```

### **2️⃣ Configure the Project Using CMake**
```sh
mkdir build
cd build
cmake ..
```

### **3️⃣ Build the Project**
#### **For Windows (MSVC / Visual Studio):**
```sh
cmake --build . --config Debug
```
#### **For MinGW (Windows alternative):**
```sh
cmake -G "MinGW Makefiles" ..
cmake --build .
```
#### **For Linux/macOS:**
```sh
cmake --build .
```

### **4️⃣ Run the CLI**
After building, navigate to the output directory and run:
```sh
./btsk -?
```
For Windows:
```powershell
cd Debug
./btsk -?
```

---

## 🎯 Adding `btsk` to System PATH (Optional)
If you want to run `btsk` from anywhere:

### **On Windows (PowerShell):**
```powershell
[System.Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\\path\\to\\your\\build\\Debug", [System.EnvironmentVariableTarget]::Machine)
```
Restart PowerShell and run:
```sh
btsk -?
```

### **On Linux/macOS:**
```sh
echo 'export PATH="$HOME/path/to/build:$PATH"' >> ~/.bashrc
source ~/.bashrc
```
Now you can use `btsk` globally.

---

## 📂 Ignoring `build/` in Git
Since the `build/` directory contains compiled files, **DO NOT** push it to GitHub. Add it to `.gitignore`:

```sh
echo "build/" >> .gitignore
git rm -r --cached build/
git add .gitignore
git commit -m "Ignore build directory"
git push origin main
```

---

## 📜 License
This project is licensed under the MIT License.

---

## 🤝 Contributing
1. Fork the repository.
2. Create a feature branch (`git checkout -b feature-xyz`).
3. Commit your changes (`git commit -m "Added new feature"`).
4. Push to the branch (`git push origin feature-xyz`).
5. Open a Pull Request.

---

## 📬 Contact
For any questions or issues, open an issue on GitHub or reach out to [your contact info].

