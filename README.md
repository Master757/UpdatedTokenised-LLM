# Custom C++ / Node.js Local LLM Server  

## Architecture
* **Backend:** C++ TCP Server utilizing `libcurl` to communicate with the Ollama API.
* **Frontend:** Node.js Express server serving HTML, CSS, and Vanilla JavaScript.
* **AI Engine:** Ollama running local models (e.g., Llama 3.1, Mistral, or custom Modelfiles).
* **Tunneling:** ngrok for secure public access.

## Project Structure
```
InternetCoding/
│
├── FrontEnd/
│   ├── server.js
│   ├── script.js
│   ├── index.html
│   └── style.css
│
├── Backend/
│   ├── main.cpp
│   ├── LLM_Manager/
│   │   └── LLM_Manager.cpp
│   ├── ServerClass/
│   │   ├── TCP_server.cpp
│   │   ├── DatabaseManager.cpp
│   │   └── DatabaseManager.h
│   ├── sessions/              
│   └── Dockerfile.backend
│
├── docker-compose.yml
├── Modelfile
└── README.md
```

## Prerequisites [NOTE: DOCKER HAS ALL IT CONTAINED]
Ensure the following are installed on your *Linux/WSL* environment:

* `g++` compiler  
* `libcurl` development packages  
* Node.js and `npm`  
* Ollama  
* ngrok  

---

## Step 1: Install Dependencies
Open your terminal and run the following commands to install the required system libraries and Node.js packages.

```bash
# Update package lists
sudo apt-get update

# Install C++ curl library
sudo apt-get install libcurl4-openssl-dev

# Navigate to your frontend directory and install Node.js dependencies
cd FrontEnd
npm install
```

---

## Step 2: Configure the AI Brain
This application requires Ollama to be running in the background. Start Ollama and pull your preferred model.

```bash
# Start the Ollama service (if not already running)
ollama serve &

# Pull the base model
ollama pull mistral
```

**Note:** If you are using a custom personality, ensure you have built it using:

```bash
ollama create your_model_name -f Modelfile
```

---

## Step 3: Compile the C++ Backend
Navigate to the Backend directory and compile the source code. You must link the curl library using the `-lcurl` flag.

```bash
cd Backend

g++ main.cpp ServerClass/TCP_server.cpp ServerClass/DatabaseManager.cpp LLM_Manager/LLM_Manager.cpp -o ai_chat_server -lcurl
```

---

## Step 4: Run the Full Stack
You will need three separate terminal windows to run the complete application stack.

### --> Terminal 1: Start the C++ TCP Server
This server manages the raw socket connections and the buffer parsing for the LLM.

```bash
cd Backend
./ai_chat_server
```

### --> Terminal 2: Start the Node.js Web Server
This server hosts the web UI and communicates with the C++ backend.

```bash
cd FrontEnd
node server.js
```

### --> Terminal 3: Start ngrok
To expose your local Node.js server to the internet, create a secure tunnel using ngrok. This command tells ngrok to create a tunnel for HTTP traffic on port 5000.

```bash
ngrok http 5000
```

You can get your authentication key from the website:

```
https://ngrok.com/
```

Then activate it:

```bash
ngrok config add-authtoken YOUR_AUTH_TOKEN_HERE
```

---

## Testing the Backend Directly (Telnet)
If you wish to bypass the Node.js frontend and test the raw C++ socket server directly, you can use telnet.

1. Ensure the C++ server is running.  
2. Open a new terminal and connect to the socket:

```bash
telnet localhost 8080
```

---

## Multi-Session Token System
The application now uses a **UUID-based session system** to isolate users:
*   **Independent Balances:** Every new browser tab generates a unique Session ID. This means User A and User B have completely separate 100-token starting balances.
*   **Persistent Storage:** Data is stored in the `Backend/sessions/` directory.
    *   `balance_UUID.txt`: Tracks the remaining tokens for that specific browser tab.
    *   `history_UUID.txt`: Stores the private chat history for that session.
*   **Session Behavior:** If you refresh a tab, your balance and history stay! If you open a *new* tab, you start fresh as a new user.
*   **Security:** If a session's balance hits 0, that specific user is locked out, but other users can continue chatting normally.

## Docker Incorporation
This project also supports containerized deployment using **Docker**. Docker allows the frontend and backend services to run in isolated containers while maintaining consistent environments across machines.

### Docker Prerequisites
Ensure the following are installed:

* Docker  
* Docker Compose  

### Running the Application with Docker

From the root project directory:

```bash
docker compose build
docker compose up
```

This will start:

* **Frontend Container** → Node.js server on port **5000**
* **Backend Container** → C++ TCP server on port **8080**

Once running, access the application at:

```
http://localhost:5000
```

### Stopping Docker Services

```bash
docker compose down
```

### Notes
* The Docker setup assumes **Ollama is running on the host machine**.
* The backend container communicates with Ollama using the host networking bridge.
