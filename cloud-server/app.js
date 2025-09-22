// ROTS Cloud Server - Main Application
const express = require('express');
const mqtt = require('mqtt');
const mysql = require('mysql2');
const cors = require('cors');
const bodyParser = require('body-parser');
const moment = require('moment');

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(bodyParser.json());
app.use(express.static('public'));

// Database connection
const db = mysql.createConnection({
  host: process.env.DB_HOST || 'localhost',
  user: process.env.DB_USER || 'root',
  password: process.env.DB_PASSWORD || 'password',
  database: process.env.DB_NAME || 'rots_db'
});

// MQTT connection
const mqttClient = mqtt.connect(process.env.MQTT_URL || 'mqtt://localhost:1883');

// Global variables
let connectedDevices = new Map();
let commandQueue = [];

// Database initialization
function initDatabase() {
  const createTables = `
    CREATE TABLE IF NOT EXISTS devices (
      id INT PRIMARY KEY AUTO_INCREMENT,
      device_id VARCHAR(50) UNIQUE NOT NULL,
      device_type ENUM('sender', 'receiver') NOT NULL,
      location VARCHAR(100),
      status ENUM('online', 'offline', 'error') DEFAULT 'offline',
      last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
      created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );
    
    CREATE TABLE IF NOT EXISTS commands (
      id INT PRIMARY KEY AUTO_INCREMENT,
      sender_id VARCHAR(50) NOT NULL,
      receiver_id VARCHAR(50) NOT NULL,
      odor_type VARCHAR(20) NOT NULL,
      intensity INT NOT NULL,
      duration INT NOT NULL,
      status ENUM('pending', 'sent', 'executed', 'failed') DEFAULT 'pending',
      created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
      executed_at TIMESTAMP NULL
    );
    
    CREATE TABLE IF NOT EXISTS logs (
      id INT PRIMARY KEY AUTO_INCREMENT,
      device_id VARCHAR(50) NOT NULL,
      log_type ENUM('info', 'warning', 'error') NOT NULL,
      message TEXT NOT NULL,
      created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );
  `;
  
  db.query(createTables, (err) => {
    if (err) {
      console.error('Database initialization failed:', err);
    } else {
      console.log('Database initialized successfully');
    }
  });
}

// MQTT event handlers
mqttClient.on('connect', () => {
  console.log('Connected to MQTT broker');
  
  // Subscribe to device topics
  mqttClient.subscribe('rots/status/+');
  mqttClient.subscribe('rots/error/+');
  mqttClient.subscribe('rots/heartbeat/+');
});

mqttClient.on('message', (topic, message) => {
  const deviceId = topic.split('/').pop();
  const messageType = topic.split('/')[1];
  
  console.log(`Received ${messageType} from ${deviceId}:`, message.toString());
  
  switch (messageType) {
    case 'status':
      handleDeviceStatus(deviceId, JSON.parse(message.toString()));
      break;
    case 'error':
      handleDeviceError(deviceId, JSON.parse(message.toString()));
      break;
    case 'heartbeat':
      handleDeviceHeartbeat(deviceId);
      break;
  }
});

// Device status handler
function handleDeviceStatus(deviceId, statusData) {
  connectedDevices.set(deviceId, {
    ...statusData,
    lastSeen: new Date(),
    status: 'online'
  });
  
  // Update database
  const query = 'UPDATE devices SET status = ?, last_seen = NOW() WHERE device_id = ?';
  db.query(query, ['online', deviceId]);
  
  // Log status
  logDeviceEvent(deviceId, 'info', `Device status updated: ${JSON.stringify(statusData)}`);
}

// Device error handler
function handleDeviceError(deviceId, errorData) {
  connectedDevices.set(deviceId, {
    ...connectedDevices.get(deviceId),
    status: 'error',
    lastSeen: new Date()
  });
  
  // Update database
  const query = 'UPDATE devices SET status = ? WHERE device_id = ?';
  db.query(query, ['error', deviceId]);
  
  // Log error
  logDeviceEvent(deviceId, 'error', `Device error: ${errorData.message}`);
}

// Device heartbeat handler
function handleDeviceHeartbeat(deviceId) {
  const device = connectedDevices.get(deviceId);
  if (device) {
    device.lastSeen = new Date();
    device.status = 'online';
  }
}

// Log device event
function logDeviceEvent(deviceId, type, message) {
  const query = 'INSERT INTO logs (device_id, log_type, message) VALUES (?, ?, ?)';
  db.query(query, [deviceId, type, message]);
}

// API Routes

// Get all devices
app.get('/api/devices', (req, res) => {
  const query = 'SELECT * FROM devices ORDER BY last_seen DESC';
  db.query(query, (err, results) => {
    if (err) {
      res.status(500).json({ error: 'Database error' });
    } else {
      res.json(results);
    }
  });
});

// Register device
app.post('/api/devices/register', (req, res) => {
  const { device_id, device_type, location } = req.body;
  
  const query = 'INSERT INTO devices (device_id, device_type, location) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE location = ?';
  db.query(query, [device_id, device_type, location, location], (err, results) => {
    if (err) {
      res.status(500).json({ error: 'Registration failed' });
    } else {
      res.json({ message: 'Device registered successfully' });
    }
  });
});

// Send odor command
app.post('/api/commands/send', (req, res) => {
  const { sender_id, receiver_id, odor_type, intensity, duration } = req.body;
  
  // Validate command
  if (!sender_id || !receiver_id || !odor_type || !intensity || !duration) {
    return res.status(400).json({ error: 'Missing required parameters' });
  }
  
  // Create command object
  const command = {
    message_type: 1, // ROTS_MSG_ODOR_COMMAND
    odor_type: getOdorTypeCode(odor_type),
    intensity: Math.min(Math.max(intensity, 0), 100),
    duration: Math.min(Math.max(duration, 1), 300),
    pump_config: [0, 0, 0, 0, 0], // Will be filled by recipe
    timestamp: Date.now(),
    checksum: 0 // Will be calculated
  };
  
  // Calculate checksum
  command.checksum = calculateChecksum(command);
  
  // Store in database
  const query = 'INSERT INTO commands (sender_id, receiver_id, odor_type, intensity, duration) VALUES (?, ?, ?, ?, ?)';
  db.query(query, [sender_id, receiver_id, odor_type, intensity, duration], (err, results) => {
    if (err) {
      res.status(500).json({ error: 'Failed to store command' });
    } else {
      // Send via MQTT
      const topic = `rots/command/${receiver_id}`;
      mqttClient.publish(topic, JSON.stringify(command));
      
      res.json({ 
        message: 'Command sent successfully',
        command_id: results.insertId
      });
    }
  });
});

// Get command history
app.get('/api/commands/history', (req, res) => {
  const query = 'SELECT * FROM commands ORDER BY created_at DESC LIMIT 100';
  db.query(query, (err, results) => {
    if (err) {
      res.status(500).json({ error: 'Database error' });
    } else {
      res.json(results);
    }
  });
});

// Get device logs
app.get('/api/devices/:deviceId/logs', (req, res) => {
  const deviceId = req.params.deviceId;
  const query = 'SELECT * FROM logs WHERE device_id = ? ORDER BY created_at DESC LIMIT 50';
  db.query(query, [deviceId], (err, results) => {
    if (err) {
      res.status(500).json({ error: 'Database error' });
    } else {
      res.json(results);
    }
  });
});

// Helper functions
function getOdorTypeCode(odorType) {
  const odorTypes = {
    'coffee': 1,
    'alcohol': 2,
    'lemon': 3,
    'mint': 4,
    'lavender': 5,
    'mixed': 6
  };
  return odorTypes[odorType.toLowerCase()] || 1;
}

function calculateChecksum(command) {
  let checksum = 0;
  const data = JSON.stringify(command);
  for (let i = 0; i < data.length; i++) {
    checksum += data.charCodeAt(i);
  }
  return checksum & 0xFFFF;
}

// Start server
app.listen(PORT, () => {
  console.log(`ROTS Cloud Server running on port ${PORT}`);
  initDatabase();
});

// Graceful shutdown
process.on('SIGINT', () => {
  console.log('Shutting down server...');
  mqttClient.end();
  db.end();
  process.exit(0);
});
