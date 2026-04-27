const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const net = require('net');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

const CPP_PORT = 8080;
const PROXY_PORT = 5000;

app.use(express.static(path.join(__dirname, 'public')));

wss.on('connection', (ws) => {
    console.log('Browser connected to Proxy (WebSocket)');

    // Connect to C++ TCP Server
    const tcpClient = new net.Socket();
    
    tcpClient.connect(CPP_PORT, '127.0.0.1', () => {
        console.log('Proxy connected to C++ Server (TCP)');
    });

    // Forward messages from C++ to Browser
    tcpClient.on('data', (data) => {
        const message = data.toString('utf8');
        console.log(`Received from C++: ${message}`);
        if (ws.readyState === WebSocket.OPEN) {
            ws.send(message);
        }
    });

    // Forward messages from Browser to C++
    ws.on('message', (message) => {
        const textMessage = message.toString('utf8');
        console.log(`Received from Browser: ${textMessage}`);
        if (tcpClient.writable) {
            tcpClient.write(textMessage);
        }
    });

    // Handle close events
    ws.on('close', () => {
        console.log('Browser disconnected');
        tcpClient.destroy();
    });

    tcpClient.on('close', () => {
        console.log('C++ Server connection closed');
        if (ws.readyState === WebSocket.OPEN) {
            ws.close();
        }
    });

    tcpClient.on('error', (err) => {
        console.error('TCP Error:', err.message);
        ws.send(`AI: Error connecting to backend server: ${err.message}`);
    });
});

server.listen(PROXY_PORT, () => {
    console.log(`Proxy server running on http://localhost:${PROXY_PORT}`);
});
