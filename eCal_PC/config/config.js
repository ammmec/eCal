function makeTopic(topic, classroom, date = '') {
    return `${topic}/${classroom}${date ? '/' + date : ''}`;
}

function updateMeta(client, classroom, level, override) {
    let prevMeta = null;
    const topic = "meta/" + classroom;

    const handler = (recTopic, message) => {
        if (recTopic === topic) prevMeta = message;
    };

    client.on('message', handler);

    client.subscribe(topic, () => {
        setTimeout(() => {
            client.removeListener('message', handler);
            client.unsubscribe(topic);
            prevMeta = (prevMeta === null) ? 0xE0 : prevMeta;

            const meta = (prevMeta[0] & 0x9F) | ((1 << 7) | ((level & 0x03) << 5));

            const payload = new Uint8Array(1);

            if (override) payload[0] = meta;
            else if (level <= ((prevMeta[0] & 0x60) >> 5)) payload[0] = meta;
            else return;

            client.publish(topic, payload, { retain: true }, () => {
                console.log('Published ' + payload + ' to topic: ' + topic);
            });
        }, 2500);
    });
}

function buildConfig(layout, lines, static, energy, retry) {
    const byte1 = ((layout & 0x07) << 5) | (lines << 4) | (static << 3) | (energy << 2) | (retry & 0x30) >> 4;
    const byte2 = (retry & 0x0F) << 4;

    const payload = new Uint8Array(2);
    payload[0] = byte1;
    payload[1] = byte2;

    return payload;
}

function publishConfig(client, topic, config) {
    client.publish(topic, config, { retain: true }, () => {
        console.log('Published ' + config + ' to topic: ' + topic);
    });
}

function sendData(client, level, buildings, floors, classroom, override, config) {
    const aulaOptions = [
        /*    "A1S101", "A3204", "A4002", "A5001", "A5002", "A5101", "A5102",
            "A5103", "A5104", "A5105", "A5106", "A5201", "A5202", "A5203",
            "A5204", "A5205", "A5206", "A5E01", "A5E02", "A5S102", "A5S103",
            "A5S104", "A5S108", "A5S109", "A5S111", "A5S113", "A6001", "A6002",
            "A6101", "A6102", "A6103", "A6104", "A6105", "A6106", "A6201", "A6202",
            "A6203", "A6204", "A6205", "A6206", "A6E01", "A6E02", "B4002", "B5S101",
            "B5S201", "B5S202", "C5S202", "C5S203", "C6S301", "C6S302", "C6S303",
            "C6S306", "C6S308", "C6S309", "D6003", "D6003bis", "D6004"*/
        "A6001", "A5002", "A6101", "A5102"
    ];

    if (level == 'global') {
        publishConfig(client, "config", config);
        for (const aula of aulaOptions) {
            updateMeta(client, aula, 3, override);
        }
        return;
    }
    else if (level == 'building') { // Building
        for (const building of buildings) {
            publishConfig(client, "config/" + building, config);
        }
        for (aula of aulaOptions.filter(aula => buildings.some(b => aula.startsWith(b)))) {
            updateMeta(client, aula, 2, override);
        }
    }
    else if (level == 'floor') { // Floor
        let filtered = aulaOptions;
        if (buildings.length && floors.length) { // If a floor of a specific building is chosen
            // Send config to config/building/floor
            // Send meta to filtered classes
            for (const building of buildings) {
                for (const floor of floors) {
                    publishConfig(client, "config/" + building + "/" + floor, config);
                    for (aula of aulaOptions.filter(a => a.startsWith(building + floor))) {
                        updateMeta(client, aula, 1, override);
                    }
                }
            }
        }
        else if (!buildings.length && floors.length) { // If only a floor (no specific building) is chosen
            // Send config to filtered config/building/floor
            // Send meta to filtered classes
            const buildingOptions = ['A1', 'A3', 'A4', 'A5', 'A6', 'B4', 'B5', 'B6', 'C5', 'C6', 'D6'];

            for (const floor of floors) {
                for (const building of buildingOptions) {
                    const matchingAules = aulaOptions.filter(aula => aula.startsWith(building + floor));

                    if (matchingAules.length > 0) {
                        publishConfig(client, `config/${building}/${floor}`, config);

                        for (const aula of matchingAules) {
                            updateMeta(client, aula, 1, override);
                        }
                    }
                }
            }
        }
    }
    else { // Classroom specific
        for (const aula of classroom) {
            publishConfig(client, "config/" + aula[0] + '' + aula[1] + "/" + aula[2] + "/" + aula, config);
            updateMeta(client, aula, 0, override);
        }
        return;
    }
}

function sendMQTT() {
    const client = mqtt.connect('wss://u7febd84.ala.us-east-1.emqxsl.com:8084/mqtt', options); // Connect to MQTT

    client.on('error', (err) => {
        console.log('MQTT error');
        return false;
    });


    const layout = document.getElementById('layoutDropdown').options[layoutDropdown.selectedIndex].getAttribute('data-name');
    const lines = document.getElementById('linesCheckbox').checked;
    const static = document.getElementById('static').checked;
    const energy = document.getElementById('energy').checked;
    const retry = document.getElementById('numberInput').value;
    const level = document.querySelector('input[name="radioOption"]:checked').value;
    const building = Array.from(document.getElementById('dropdown1').selectedOptions).map(opt => opt.value);
    const floor = Array.from(document.getElementById('dropdown2').selectedOptions).map(opt => opt.value);
    const classroom = Array.from(document.getElementById('dropdown3').selectedOptions).map(opt => opt.value);
    const override = document.getElementById('override').checked;

    /*console.log("Layout: " + layout);
    console.log("Lines: " + lines);
    console.log("Static: " + static);
    console.log("Energy: " + energy);
    console.log("Retry: " + retry);
    console.log("Level: " + level);
    console.log("Building: " + building);
    console.log("Floor: " + floor);
    console.log("Classroom: " + classroom);
    console.log("Override: " + override);
    */

    sendData(client, level, building, floor, classroom, override, buildConfig(layout, lines, static, energy, retry));
    alert("Configuració enviada correctament");
}