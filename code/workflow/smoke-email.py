import json
sensor_data = parameter[1]
max_index = len(parameter[1])
latest_data = sensor_data[max_index-1]
latest_sensor_data = latest_data["Gas detector"] 
threshold = 300 
if int(latest_sensor_data) > threshold:
    msgbody='<p>The current gas level '+latest_sensor_data+' is above threshold, the reading is more than '+str(threshold)+'.</p><br>'
    output[1] = "Abnormal Reading Detected - Smoke!"
    output[2] = msgbody
    output[3] = 1
else:
    output[1] = ""
    output[2] = ""
    output[3] = ""
