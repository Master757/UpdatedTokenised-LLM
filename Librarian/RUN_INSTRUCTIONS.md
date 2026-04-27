# Librarian AI: Run Instructions

Follow these steps to compile and run the Librarian AI system. 

## ** NOTE: THIS IS RUN ON A WSL ENVIRONEMNT SO USE DEBIAN OR UBUNTU FOR BEST RESPONSE**

## Prerequisites
- **C++ Compiler**: `g++`
- **Libraries**: `libcurl`, `sqlite3`
- **Node.js**: For the frontend proxy.
- **Gemini API Key**: You must provide your own API key in `backend/src/serverMain.cpp`.

---

## Step 1: Configuration
Before compiling, open `backend/src/serverMain.cpp` and replace the placeholder API key with your actual Google Gemini API Key:
```cpp
#define API_KEY "YOUR_ACTUAL_KEY_HERE"
```

## Step 2: Compile the Backend
From the project root directory, run the following command to build the C++ server:
```bash
g++ -Ibackend/include backend/src/langModelManagement.cpp backend/src/serverMain.cpp backend/src/fileManager.cpp -o backend/build/library_server -lcurl -lsqlite3
```

## Step 3: Start the C++ Server
Run the compiled executable:
```bash
./backend/build/library_server
```
*Note: Keep this terminal window open.*

## Step 4: Start the Frontend Proxy
In a new terminal window, navigate to the `frontend` directory and start the Node.js server:
```bash
cd frontend
node server.js
```
*Note: Keep this terminal window open.*

## Step 5: Open the Interface
Open your web browser and navigate to:
**http://localhost:5000**

---
### System Architecture
- **Port 8080**: C++ Backend (Raw TCP).
- **Port 5000**: Node.js Proxy & Web Interface.
- **Database**: SQLite (`library.db`).
- **History**: Chat logs are stored in `Archives/` and cleaned up on disconnect.
