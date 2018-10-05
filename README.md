# SoilSensorsProofing
Compare capacitive and resistive soil sensors.
sketchs for Wemos D1R1 (ESP8266)

Data uploaded to Thingspeak
[a link](https://thingspeak.com/channels/564209)

From my own experience, i have found that capacitive soil sensors are not adapted to measure soil moisture. Resistive sensors corrode, that's a fact, but if you measure only every hour, or every day, putting current only a few ms, your sensor can last very long. Capactive sensors doesn't corrode, but when they are inserted in the soil, they measure moisture at that time, and stay with that value till you put them out of the soil and insert them again. I tried 3 different capactive soil sensors, and it was each time the same issue.

if you have a look at the graphics on ThingSpeak, you can see where i have decided to put out the capacitive sensor and then insert it again it the soil. 

for resistive sensor, moisture is measured correctly, and we are reaching an asymptote 7 days after initial watering.

Here to share, do not hesitate to comment.
