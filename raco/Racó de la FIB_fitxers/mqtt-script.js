function formatDate(date) {
  const year = date.getFullYear();
  const month = String(date.getMonth() + 1).padStart(2, '0'); // Months are 0-indexed, so +1
  const day = String(date.getDate()).padStart(2, '0');

  return `${day}${month}${year}`; // Outputs in the format "DDMMYYYY"
}

function makeTopic(topic, classroom, date = '') {
  return `${topic}/${classroom}${date ? '/' + date : ''}`;
}

function publishMessage(client, topic, msg) {
  let previousMessages = null;

  const handler = (recTopic, message) => {
    if (recTopic === topic && message.length >= 1 && message[0] !== 0x00) previousMessages = message;
  };

  client.on('message', handler);

  client.subscribe(topic, () => {
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
  });
}

function buildPayload(start, duration, type, name = '') {
  const byte1 = (((start - 8) & 0x0F) << 4) | (duration & 0x0F);
  let byte2;
  if (type === 'add') {
    byte2 = 1 << 4;
  } else if (type === 'cancel') {
    byte2 = 2 << 4;
  } else {
    byte2 = 0;
  }

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

function updateMeta(client, topic, ann, change) {
  let prevMeta = null;

  const handler = (recTopic, message) => {
    if (recTopic === topic) prevMeta = message;
  };

  client.on('message', handler);

  client.subscribe(topic, () => {
    setTimeout(() => {
      client.removeListener('message', handler);
      client.unsubscribe(topic);
      const meta = prevMeta[0] | ((ann << 3) | (change << 4));
      const payload = new Uint8Array(1);
      payload[0] = meta;

      client.publish(topic, payload, { retain: true }, () => {
        console.log(payload);
        console.log('Published to topic: ' + topic);
      });
    }, 5000);
  });
}

function sendMQTTMessage() {
  if (!document.getElementById('deviceCheckbox').checked) return false; // If there is no intention to send information to the broker, do not

  const announcement = (document.getElementById('MQTTmessage').value).normalize("NFD").replace(/[\u0300-\u036f]/g, "") + "\n\n"; // The announcement to send to the announcements topic
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

  let newAnn = false;

  // Send an announcement if it was written
  if (announcement != "\n\n") {
    const encoder = new TextEncoder();
    const announcementBytes = encoder.encode(announcement);
    publishMessage(client, makeTopic('announcements', sendClassroom, sendDate), announcementBytes);
    newAnn = true;
  }
  if (type === 'info') {
    if (sendDate === '') updateMeta(client, makeTopic("meta", sendClassroom, sendDate), true, false);
    alert("Missatge enviat!");
    return false;
  }

  if (type != "swap") {
    publishMessage(client, makeTopic('changes', sendClassroom, sendDate), buildPayload(sendHour, 2, type, type === 'add' ? className : ''));
    if (sendDate === '') updateMeta(client, makeTopic("meta", sendClassroom, sendDate), newAnn, true);
    alert("Canvi enviat!");
    return false;
  }

  payloadCancel = buildPayload(sendHour, 2, "cancel", '');
  payloadAdd = buildPayload(parseInt(((document.getElementById('toTime').value).split(':'))[0]), 2, "add", className);

  select = document.getElementById('toAulaS');

  if (formatDate(new Date(document.getElementById('dateChange').value)) === formatDate(new Date(document.getElementById('dateTo').value))
    && sendClassroom === select.options[select.selectedIndex].text) {
    if (announcement != "\n\n") {
      const encoder = new TextEncoder();
      const announcementBytes = encoder.encode(announcement);
      publishMessage(client, makeTopic('announcements', sendClassroom, sendDate), announcementBytes);
    }
    const concatPayload = new Uint8Array(payloadCancel.length + payloadAdd.length);
    concatPayload.set(payloadCancel, 0);
    concatPayload.set(payloadAdd, payloadCancel.length);

    publishMessage(client, makeTopic('changes', sendClassroom, sendDate), concatPayload);
    if (sendDate === '') {
      updateMeta(client, makeTopic("meta", sendClassroom, sendDate), newAnn, true);
    }
  }
  else {
    publishMessage(client, makeTopic('changes', sendClassroom, sendDate), payloadCancel);
    if (sendDate === '') updateMeta(client, makeTopic("meta", sendClassroom, sendDate), newAnn, true);
    sendClassroom = select.options[select.selectedIndex].text;
    sendDate = formatDate(new Date(document.getElementById('dateTo').value));
    if (sendDate === formatDate(new Date())) sendDate === '';
    publishMessage(client, makeTopic('changes', sendClassroom, sendDate), payloadAdd);
    if (sendDate === '') updateMeta(client, makeTopic("meta", sendClassroom, sendDate), newAnn, true);
  }
  alert("Canvi enviat!");
  return false;
}