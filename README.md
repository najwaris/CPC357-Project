# Smart Indoor Air Quality Monitoring and Alert System
A simple Smart Indoor Air Quality Monitoring and Alert System where it will monitor the surrounding and alert the people by lighten up the red LED. 

## How it work
It will monitor the surrounding by taking into account 3 conditions which are:
  1. Temperature
  2. Humidity
  3. Gas / Smoke

## What sensors used
The sensors used in this system are:
  1. DHT11 for the temperature and humidity
  2. MQ2 for gas / smoke

## The workflow
  1. The sensor will read the data from the surrounding and send to V-One platform for data processing.
  2. If the data pass the threshold set by the system, the red LED will lights up indicate that it's dangerous and notify user.
  3. If the data doesnt pass the threshold, the yellow LED will light up and user will not be notified about anything.
