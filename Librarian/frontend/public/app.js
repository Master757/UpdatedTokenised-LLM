const chatContainer = document.getElementById('chat-container');
const messageInput = document.getElementById('message-input');
const sendBtn = document.getElementById('send-btn');
const statusText = document.getElementById('connection-status');
const pulseDot = document.querySelector('.pulse-dot');

let ws;

function connect() {
    // Connect to the Node.js Proxy on the same host
    ws = new WebSocket(`ws://${window.location.host}`);

    ws.onopen = () => {
        statusText.textContent = 'Connected';
        pulseDot.classList.remove('disconnected');
        
        // Let's clear the chat just in case of reconnect
        chatContainer.innerHTML = '';
        appendMessage('System', 'Connected to Librarian AI proxy.', 'ai');
    };

    ws.onmessage = (event) => {
        let msg = event.data;
        // The server might prefix with "AI: "
        if (msg.startsWith('AI: ')) {
            msg = msg.substring(4);
        }
        appendMessage('AI', msg, 'ai');
    };

    ws.onclose = () => {
        statusText.textContent = 'Disconnected';
        pulseDot.classList.add('disconnected');
        appendMessage('System', 'Connection lost. Reconnecting...', 'ai');
        setTimeout(connect, 3000); // Auto-reconnect
    };

    ws.onerror = (err) => {
        console.error('WebSocket error:', err);
    };
}

function appendMessage(sender, text, type) {
    const msgDiv = document.createElement('div');
    msgDiv.classList.add('message', type);
    
    // Convert newlines to <br> for HTML display
    const formattedText = text.replace(/\n/g, '<br>');
    msgDiv.innerHTML = formattedText;
    
    chatContainer.appendChild(msgDiv);
    
    // Auto-scroll to bottom
    chatContainer.scrollTop = chatContainer.scrollHeight;
}

function sendMessage() {
    const text = messageInput.value.trim();
    if (text && ws && ws.readyState === WebSocket.OPEN) {
        appendMessage('You', text, 'user');
        ws.send(text);
        messageInput.value = '';
    }
}

sendBtn.addEventListener('click', sendMessage);

messageInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter') {
        sendMessage();
    }
});

// Initialize connection
connect();
