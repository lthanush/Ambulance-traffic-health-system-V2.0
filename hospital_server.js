const express = require('express');
const bodyParser = require('body-parser');
const fs = require('fs');
const path = require('path');

const app = express();
const PORT = 80;

app.use(bodyParser.json());
app.use(express.static('public'));

let patientData = [];
const MAX_RECORDS = 100;

app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'dashboard.html'));
});

app.post('/patient-data', (req, res) => {
  const data = req.body;
  const timestamp = new Date().toISOString();
  
  const record = {
    heartRate: data.heartRate || 0,
    spo2: data.spo2 || 0,
    receivedAt: timestamp,
    ambulanceTime: data.timestamp || 0
  };
  
  patientData.unshift(record);
  if (patientData.length > MAX_RECORDS) {
    patientData.pop();
  }
  
  console.log(`[${timestamp}] Patient Data:`);
  console.log(`  HR: ${record.heartRate} BPM, SpO2: ${record.spo2}%`);
  
  if (record.heartRate > 120 || record.heartRate < 50) {
    console.log(`  ⚠️ Abnormal heart rate`);
  }
  if (record.spo2 < 90) {
    console.log(`  ⚠️ Low oxygen saturation`);
  }
  
  const logEntry = `${timestamp},${record.heartRate},${record.spo2}\n`;
  fs.appendFile('patient_logs.csv', logEntry, (err) => {
    if (err) console.error('Log write error:', err);
  });
  
  res.status(200).json({ status: 'received', timestamp: timestamp });
});

app.get('/api/recent-data', (req, res) => {
  res.json(patientData.slice(0, 20));
});

app.get('/api/current-data', (req, res) => {
  if (patientData.length > 0) {
    res.json(patientData[0]);
  } else {
    res.json({ heartRate: 0, spo2: 0, receivedAt: null });
  }
});

app.listen(PORT, () => {
  console.log('Hospital Patient Monitoring Server');
  console.log(`Server running on port ${PORT}`);
  console.log(`Dashboard: http://localhost:${PORT}`);
  console.log(`Waiting for ambulance data...\n`);
  
  if (!fs.existsSync('patient_logs.csv')) {
    fs.writeFileSync('patient_logs.csv', 'Timestamp,HeartRate,SpO2\n');
  }
});
