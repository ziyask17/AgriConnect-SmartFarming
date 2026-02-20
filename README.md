🌱 AgriConnect – IoT-Based Smart Agriculture System Auto-Irrigation | Real-Time Monitoring | Raspberry Pi MQTT Dashboard | ESP32 | ThingSpeak

AgriConnect is an IoT-powered smart farming system designed to automate irrigation and provide real-time monitoring of soil moisture, temperature, and humidity. The system uses ESP32 for sensing & publishing data, Raspberry Pi as an MQTT broker + Node-RED server, and ThingSpeak for cloud analytics.

🚀 Features

🌡️ Real-time monitoring of Soil Moisture, Temperature & Humidity

💧 Automatic irrigation based on soil-moisture threshold

📡 ESP32 → Raspberry Pi communication via MQTT

☁️ Cloud dashboard on ThingSpeak for historical analytics

🖥️ Local Node-RED dashboard for live visualization

🎛️ Manual pump control from Node-RED UI

🗄️ SQLite database logging on Raspberry Pi

🔁 Bidirectional MQTT communication (Pump ON/OFF)

Workflow

ESP32 reads sensor data (Soil Moisture + DHT11).

Publishes data to MQTT topics:

agriConnect/soil

agriConnect/temp

agriConnect/hum

Raspberry Pi (Mosquitto broker) receives data and:

Stores it in SQLite

Updates Node-RED dashboard

Sends data to ThingSpeak via HTTP

Pump is controlled through:

Auto mode (if moisture < threshold)

Manual Node-RED switch → MQTT topic: agriConnect/pumpControl

📡 MQTT Topics

Topic	Description
agriConnect/soil	Soil moisture values
agriConnect/temp	Temperature values
agriConnect/hum	Humidity values
agriConnect/pumpControl	Manual ON/OFF control
agriConnect/pumpStatus	Pump feedback status
📊 Node-RED Dashboard Overview

Real-time gauges for:

Soil Moisture

Temperature

Humidity

History graphs (time-series)

Pump ON/OFF switch via MQTT

☁️ ThingSpeak Cloud Analytics

Field 1 → Soil Moisture

Field 2 → Temperature

Field 3 → Humidity

Field 4 → Pump Status

Provides long-term graphing and analysis.
