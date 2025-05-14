function formatDate(date) {
  const year = date.getFullYear();
  const month = String(date.getMonth() + 1).padStart(2, '0'); // Months are 0-indexed, so +1
  const day = String(date.getDate()).padStart(2, '0');

  return `${day}${month}${year}`; // Outputs in the format "DDMMYYYY"
}

function publishMessage(client, topic, msg) {
  let previousMessages = null;
  client.subscribe(topic);

  const handler = (recTopic, message) => {
    if (recTopic === topic) previousMessages = message;
  };

  client.on('message', handler);

  setTimeout(() => {
    client.removeListener('message', handler);
    client.unsubscribe(topic);
    let fullPayload;
    if (previousMessages) {
      const combined = new Uint8Array(previousMessages.length + msg.length);
      combined.set(previousMessages, 0);
      combined.set(msg, previousMessages.length);
      fullPayload = combined;
    } else {
      fullPayload = msg;
    }
    client.publish(topic, fullPayload, { retain: true }, () => {
      console.log(fullPayload);
      console.log('Published to topic: ' + topic);
    });
  }, 5000);
}

function buildPayload(start, duration, type, name = '') {
  const byte1 = (((start - 8) & 0x0F) << 4) | (duration & 0x0F);
  const byte2 = type == 'other' ? 0 : type === 'add' ? (1) << 4 : (2) << 4;

  const nameLength = name.length;
  const nameBytes = [...name].map(c => c.charCodeAt(0)); // Convert name to byte array

  const payload = new Uint8Array(2 + (nameLength > 0 ? 1 + nameLength : 0));
  payload[0] = byte1;
  payload[1] = byte2;

  if (nameLength > 0) {
    payload[2] = nameLength;
    payload.set(nameBytes, 3);
  }

  return payload;
}

function sendMQTTMessage() {
  if (!document.getElementById('deviceCheckbox').checked) return false; // If there is no intention to send information to the broker, do not

  const options = {
    clean: true,
    connectTimeout: 4000,
    clientId: 'web_client_' + Math.random().toString(16).substr(2, 8),
    username: 'elki0_0',
    password: 'szUfgVCW#5x4=8z',
    reconnectPeriod: 1000
  };

  const announcement = (document.getElementById('MQTTmessage').value).normalize("NFD").replace(/[\u0300-\u036f]/g, "");; // The announcement to send to the announcements topic
  let type = document.getElementById('type').value === 'info' ? 'info' : document.querySelector('input[name="option"]:checked')?.value; // Type of message to send

  const client = mqtt.connect('wss://u7febd84.ala.us-east-1.emqxsl.com:8084/mqtt', options); // Connect to MQTT

  client.on('error', (err) => {
    console.log('MQTT error');
    return false;
  });

  // Check which date to send the information to. Will have to be done twice in case of a swap
  let sendDate = type === 'info' ? formatDate(new Date(document.getElementById('infoDate').value)) : formatDate(new Date(document.getElementById('dateChange').value));
  if (sendDate === formatDate(new Date())) sendDate = '';

  // Get 1st classroom to send the message to
  let select = document.getElementById(type === 'info' ? 'aula' : 'fromAulaS');
  let sendClassroom = select.options[select.selectedIndex].text;
  let sendHour = parseInt(((document.getElementById('fromTime').value).split(':'))[0]);

  let className = "GEI AE 10T"; // As it is a prototype, the name is a placeholder

  // Send an announcement if it was written
  if (announcement != '') publishMessage(client, 'announcements/' + sendClassroom + (sendDate === '' ? '' : ('/' + sendDate)), announcement);
  if (type === 'info') return false;

  publishMessage(client, 'changes/' + sendClassroom + (sendDate === '' ? '' : ('/' + sendDate)), buildPayload(sendHour, 2, type === "swap" ? "cancel" : type, type === 'add' ? className : ''));

  if (type != "swap") return false;

  type = "add";
  select = document.getElementById('toAulaS');
  sendClassroom = select.options[select.selectedIndex].text;
  sendDate = formatDate(new Date(document.getElementById('dateTo').value));
  if (sendDate === formatDate(new Date())) sendDate = '';
  sendHour = parseInt(((document.getElementById('toTime').value).split(':'))[0]);
  if (announcement != '') publishMessage(client, 'announcements/' + sendClassroom + (sendDate === '' ? '' : ('/' + sendDate)), announcement);

  publishMessage(client, 'changes/' + sendClassroom + (sendDate === '' ? '' : ('/' + sendDate)), buildPayload(sendHour, 2, type, type === 'add' ? className : ''));


  return false;
}