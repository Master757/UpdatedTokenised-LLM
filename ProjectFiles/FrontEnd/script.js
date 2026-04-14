// Generate a unique session ID per browser tab
// sessionStorage is unique per tab, so each tab = separate user
if (!sessionStorage.getItem("sessionId")) {
    sessionStorage.setItem("sessionId", crypto.randomUUID());
}
const SESSION_ID = sessionStorage.getItem("sessionId");

const inputValue = document.getElementById("prompt");
async function sendPrompt() {
    const textarea = document.getElementById("prompt");
    const chatBox = document.getElementById("chatBox");
    const text = textarea.value.trim();
    if (!text) return;
    // Quit condition (kept from your reference)
    if (text.toLowerCase() === "quit") {
        document.body.innerHTML = "<h1>Thank you</h1>";
        return;
    }
    const typing = chatBox.querySelector(".typing-indicator");
    if (typing) typing.closest(".message-wrap")?.remove();
    //  Add user message 
    const userWrap = document.createElement("div");
    userWrap.className = "message-wrap user-wrap";
    userWrap.innerHTML = `
        <span class="message-label">You</span>
        <div class="user-message">${text}</div>
    `;
    chatBox.appendChild(userWrap);
    // Clear textarea
    textarea.value = "";
    const typingWrap = document.createElement("div");
    typingWrap.className = "message-wrap ai-wrap";
    typingWrap.innerHTML = `
        <span class="message-label">AI</span>
        <div class="typing-indicator">
            <span></span><span></span><span></span>
        </div>
    `;
    chatBox.appendChild(typingWrap);
    // Auto scroll
    chatBox.scrollTop = chatBox.scrollHeight;
    try {
        // Updated to use a relative path so it works through ngrok automatically
        const res = await fetch("/ask", { 
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ prompt: text, sessionId: SESSION_ID }),
        });

        const data = await res.json();
        console.log(data);

        // Remove typing indicator
        typingWrap.remove();
        const aiWrap = document.createElement("div");
        aiWrap.className = "message-wrap ai-wrap";
        aiWrap.innerHTML = `
            <span class="message-label">AI</span>
            <div class="ai-message">${data.response || "No response from LLM"}</div>
        `;
        chatBox.appendChild(aiWrap);

        // Auto scroll after response
        chatBox.scrollTop = chatBox.scrollHeight;
    } catch (error) {
        // Remove typing indicator on error
        typingWrap.remove();

        const errorWrap = document.createElement("div");
        errorWrap.className = "message-wrap ai-wrap";
        errorWrap.innerHTML = `
            <span class="message-label">AI</span>
            <div class="ai-message">Error connecting to backend server.</div>
        `;
        chatBox.appendChild(errorWrap);

        console.error(error);
        chatBox.scrollTop = chatBox.scrollHeight;
    }
}

// Enter to send (Shift + Enter = new line)
inputValue.addEventListener("keydown", function (e) {
    if (e.key === "Enter" && !e.shiftKey) {
        e.preventDefault();
        sendPrompt();
    }
});
