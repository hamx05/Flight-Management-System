# ✈️ Flight Management System - Reader-Writer Problem Solution

## 📖 Overview
This project implements a Flight Management System that solves the classic reader-writer problem using synchronization mechanisms like mutexes and semaphores. The system handles concurrent flight bookings and cancellations while maintaining data integrity in a multi-threaded environment.

## ✨ Features
- 🔐 **User Authentication**: Register and login functionality
- 🗓️ **Flight Management**: View available flights and seat status
- 💺 **Booking System**: Concurrent seat booking with synchronization
- ❌ **Cancellation System**: Thread-safe flight cancellation
- ⚙️ **Concurrency Control**: Implements mutexes and semaphores to prevent race conditions

## 🎯 Problem Statement
The system addresses the challenge of maintaining data consistency when multiple users simultaneously:
- 👀 View flight information (readers)
- ✏️ Book or cancel seats (writers)

## 💻 Technical Implementation
### 🔧 Core Components
- **Programming Language**: `C` 
- **Synchronization Tools**:
  - 🔒 Mutex locks for critical sections
  - 🚦 Semaphores for resource management
- **File Handling**: 📁 Persistent storage for flight and user data

### 🧪 Scenarios Handled
1. 🧵 Multiple threads attempting to book the same seat
2. 🚫 Concurrent cancellation attempts  
3. 👥 Simultaneous user registration/login

## 🚀 Installation & Usage
### 📋 Prerequisites
- GCC compiler
- POSIX threads library

### 🔨 Building and Running
```bash
# Clone the repository
git clone https://github.com/k232003-TalalAli/Flight-Management-System

# Run the main program
make run

# Run simulation tests
make simul
```

## 📂 Project Structure
```bash
/Flight-Management-System
│── main.c                 # Entry point and core logic
│── book_flight.c          # Booking functionality
│── cancellation.c         # Cancellation system
│── view_flights.c         # Flight information display
│── users.txt              # User database
│── flights.txt            # Flight information
│── Makefile               # Build configuration
|── OS_Project_Report.pdf  # Project Report
│── README.md              # Project documentation
```

## 🎥 Demonstration
### 📝 Booking Flow
1. 👤 User registers/login
2. 👀 Views available flights
3. ✅ Selects flight and seat
4. ⚡ System handles concurrent booking attempts

### ✂️ Cancellation Flow
1. 👤 User logs in
2. 🗑️ Accesses cancellation menu
3. ❌ Selects flight and seat to cancel
4. ✔️ System verifies and processes request

## 👥 Contributors
- [Talal Ali](https://github.com/k232003-TalalAli)
- [Muhammad Hammad](https://github.com/hamx05)
- [Daniyal Ahmed](https://github.com/danyalAhmed4)
- [Umer Taiyab](https://github.com/Umerhhjk)

## 🌱 Open Source
This project is open source and available for anyone to use or modify. Feel free to contribute!
