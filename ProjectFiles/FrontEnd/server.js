console.log("SERVER FILE STARTED");
const express = require("express");
console.log("express loaded");


const cors = require("cors");
console.log("cors loaded");


const net = require("net");
console.log("net loaded");


const app = express();
console.log("express app created");

app.use(cors());
app.use(express.json());
app.use(express.static(__dirname));

app.get("/", (req, res) => {
    res.sendFile(__dirname + "/index.html");
});

app.post("/ask", (req, res) => {
    const prompt = (req.body.prompt || "").trim();
    const sessionId = (req.body.sessionId || "default").trim();

    if (!prompt) {
        return res.status(400).json({ error: "Empty prompt" });
    }

    const client = new net.Socket();
    let fullResponse = "";
    let responded = false;
    let timeoutHandle;

    const llmHost = process.env.LLM_HOST || "localhost";

    client.connect(8080, llmHost, () => {
        // Send as SESSION_ID|prompt so the C++ backend can parse it
        client.write(sessionId + "|" + prompt + "\n");
    });

    client.on("data", (data) => {
        const chunk = data.toString();
        fullResponse += chunk;

        const delimiterIndex = fullResponse.indexOf("♦");

        if (delimiterIndex !== -1 && !responded) {
            responded = true;

            let rawResponse = fullResponse.slice(0, delimiterIndex);

            const finalResponse = rawResponse
                .split("\n")
                .filter(line =>
                    !line.includes("Ask me anything") &&
                    !line.includes("Thinking") &&
                    !line.includes("-->")
                )
                .join("\n")
                .trim();

            if (!res.headersSent) {
                res.json({ response: finalResponse });
            }

            clearTimeout(timeoutHandle);
            client.end();
            client.destroy();
        }
    });

    client.on("error", () => {
        clearTimeout(timeoutHandle);

        if (!res.headersSent) {
            res.status(500).json({
                error: "Connection to C++ LLM server failed"
            });
        }
    });

    client.on("close", () => {
        // Connection closed
    });

    timeoutHandle = setTimeout(() => {
        if (!responded) {
            let fallback = fullResponse.replace("♦", "").trim();
            if (!fallback) fallback = "No response from LLM";

            if (!res.headersSent) {
                res.json({ response: fallback });
            }

            client.destroy();
        }
    }, 300000);
});

app.listen(5000, "0.0.0.0");