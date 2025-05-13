// MQTT broker details
const broker = 'wss://test.mosquitto.org:8081'; // public test broker
const topic = 'fib/raco/test';

const client = mqtt.connect(broker);

client.on('connect', () => {
  document.getElementById('status').textContent = '✅ Connectat al servidor MQTT';
});

client.on('error', (err) => {
  document.getElementById('status').textContent = '❌ Error de connexió: ' + err.message;
});

document.getElementById('sendBtn').addEventListener('click', () => {
  const message = document.getElementById('mqttInput').value;
  if (message.trim() === '') return;

  client.publish(topic, message, () => {
    document.getElementById('status').textContent = '📤 Missatge enviat!';
    document.getElementById('mqttInput').value = '';
  });
});
