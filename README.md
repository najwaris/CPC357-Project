# Smart Indoor Air Quality Monitoring and Alert System
A simple Smart Indoor Air Quality Monitoring and Alert System where it will monitor the surrounding and alert the people by lighten up the red LED. 

## Group members
  1. Nurul Najwa binti Mat Aris (158560)
  2. Nur Iffah Izzah binti Noor Hasim (157150)

## How it work
It will monitor the surrounding by taking into account 3 conditions which are:
  1. Temperature
  2. Humidity
  3. Gas / Smoke

## What sensors and actuators used
The sensors used in this system are:
  1. DHT11 for the temperature and humidity
  2. MQ2 for gas / smoke

The actuators used in this system are:
  1. Onboard buzzer
  2. Red and Yellow LED

## The workflow
  1. The sensor will read the data from the surrounding and process the data locally first.
  2. If the data pass the threshold set by the system, the red LED will lights up indicate that it's dangerous and notify user.
  3. If the data doesnt pass the threshold, the yellow LED will light up and user will not be notified about anything.
  4. The data will then sbe sent to the V-One cloud for data processing fur further alert to user where the cloud will send email alert to users if pass any of the thresholds. 

## The thresholds set in this system
  1. Maximum temperature is 35 
  2. Maximum humidity is 80
  3. Minimum humidity is 30
  4. Maximum gas concentration level is 300
