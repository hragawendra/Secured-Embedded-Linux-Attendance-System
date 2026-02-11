# 🔐 Secured Embedded Linux Attendance System

## 📌 Project Overview

The **Secured Embedded Linux Attendance System** is a hybrid architecture project combining an ARM7-based embedded system (LPC2129) with a Linux-based authentication server.

The embedded system handles real-time RFID scanning, RTC timekeeping, LCD display, and UART communication, while the Linux system manages employee authentication, database storage, logging, and security control.

This project demonstrates hardware-software co-design using Embedded C and Linux System Programming.

---

## 🏗️ System Architecture

```

RFID Card
↓
LPC2129 (ARM7 MCU)
↓ UART (9600 baud)
Linux Attendance Server
↓
Employee Database + Attendance Logs

```

---

## ⚙️ Embedded System (LPC2129)

### 🔹 Features

- RTC integration using I2C
- 16x2 LCD (4-bit mode)
- UART0 ↔ Linux communication
- UART1 ↔ RFID reader
- Interrupt-driven card detection
- LED & Buzzer indication
- Master Mode support
- Timeout handling for server response

### 🔹 Data Packet Sent to Linux

```

ID,DD/MM/YY,DAY,HH:MM:SS,AM/PM\n

```

### 🔹 Linux Response Codes

| Code | Meaning |
|------|----------|
| '1'  | Valid User |
| '0'  | Invalid User |
| 'M'  | Master Mode Activated |
| 'A'  | Add Employee Mode |
| 'R'  | Remove Employee Mode |
| '!'  | System Locked |

---

## 🖥️ Linux Attendance Server

The Linux application:

- Communicates via `/dev/ttyUSB0`
- Runs at 9600 baud
- Maintains employee database
- Handles secure admin authentication
- Logs attendance monthly
- Logs unknown IDs separately
- Implements password-based master control

---

## 🗂️ Employee Database

Stored in:

```

employee.dat

```

Format:

```

ID NAME

```

Data is loaded into a linked list structure at runtime.

---

## 🔐 Security Features

### 🔹 Master ID
Special RFID card:
```

4900C8FDDCA0

```

### 🔹 Password Protection
- Default Password: `admin@123`
- Stored in `config.bin`
- XOR encryption (Key: 0x5A)
- Silent password input (no terminal echo)
- Change password feature

### 🔹 Security Lock
- 3 incorrect attempts → system locked
- Lock resets automatically next day

---

## 📊 Attendance Logging

Monthly logs created automatically:

```

JANUARY
FEBRUARY
MARCH
...

```

Each entry contains:

```

ID | Name | Status | Date | Day | Time

```

Unknown IDs are logged in:

```

Anonymous_Login.log

```

---

## 📁 Project Structure

```

LPC2129 Embedded C Program/
│── main.c
│── i2c_driver.c
│── rtc_driver.c
│── lcd_4bit_driver.c
│── uart0_driver.c
│── uart1_driver.c
│── uart0_interrupt.c
│── uart1_interrupt.c
│── delay.c
│── header.h

Linux Program/
│── linux.c
│── employee.dat
│── config.bin

```

---

## 🛠️ Technologies Used

- Embedded C
- Linux System Programming
- UART Communication
- I2C Protocol
- Interrupt Handling
- Linked List Data Structures
- File Handling
- Basic Encryption (XOR)
- Real-Time Clock Integration

---

## 🚀 Engineering Concepts Demonstrated

- Hybrid Embedded + Linux Architecture
- Interrupt-driven communication
- Real-time hardware-software synchronization
- Secure authentication system
- State machine implementation
- File-based database management
- Timeout and fault handling logic

---

## 🔮 Future Improvements

- Replace XOR encryption with stronger encryption
- Integrate SQLite database
- Add TCP/IP server capability
- Develop Web Dashboard
- Add Biometric authentication
- Cloud synchronization

---

## 👨‍💻 Author

**Hanumapuram Ragawendra Reddy**  
