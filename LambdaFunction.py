import json
import boto3
import time
from decimal import Decimal
from datetime import datetime

# Initialize the SNS client and DynamoDB resource
sns = boto3.client('sns')
dynamodb = boto3.resource('dynamodb')

# Define your SNS Topic ARN for email notifications
SNS_TOPIC_ARN = 'arn:aws:sns:us-east-1:211125391782:RaspberryPiNotification'

# Set thresholds
TEMPERATURE_THRESHOLD = 30.0  # Fan turns ON if temp exceeds this
LIGHT_THRESHOLD = 900  # Bulb turns ON if light level is below this

# Store the last notification time
last_notification_time = 0
NOTIFICATION_INTERVAL = 60 * 60  # 60 minutes in seconds

# DynamoDB Table
TABLE_NAME = 'Sensor_data'

def lambda_handler(event, context):
    global last_notification_time
    
    # Initialize the DynamoDB table resource
    table = dynamodb.Table(TABLE_NAME)

    # Log the received IoT event
    print("Received IoT event:", json.dumps(event))

    # Extract necessary info from IoT event
    device_id = event.get('device_id')  # Make sure this matches "DeviceId" case-sensitivity
    if not device_id:
        print("Error: DeviceId is missing from the event.")
        return {
            'statusCode': 400,
            'body': json.dumps('Error: DeviceId is missing from the event.')
        }
    
    # Convert timestamp or use the current time if not provided
    timestamp = str(event.get('timestamp', int(datetime.now().timestamp())))  # Ensure timestamp is a string
    temperature = event.get('temperature', 0.0)  # Ensure default is float
    light_level = event.get('light_level', 0)  # Ensure default is int
    smoke_detected = event.get('smoke_detected', False)
    touch_detected = event.get('touch_detected', False)

    # Debugging log to check if all required fields are correct
    print(f"DeviceId: {device_id}, Timestamp: {timestamp}, Temperature: {temperature}, LightLevel: {light_level}")
    
    # Initialize the list of messages to be sent
    messages = []
    
    # Get current time (for interval check)
    current_time = time.time()

    # Check if 60 minutes have passed since the last notification
    if current_time - last_notification_time < NOTIFICATION_INTERVAL:
        print("60 minutes haven't passed since the last notification. No message will be sent.")
    else:
        # Bulb control logic: Only LDR controls the bulb (light level)
        if light_level < LIGHT_THRESHOLD:
            messages.append("Bulb is ON because the light level is below the threshold.")
        
        # Buzzer control logic based on the truth table
        if smoke_detected:
            messages.append("Buzzer is ON because smoke is detected in your home!")
        elif touch_detected:
            messages.append("Buzzer is ON because the touch sensor was triggered in your home!")

        # Fan control logic based on temperature (DHT) and smoke
        if smoke_detected and temperature <= TEMPERATURE_THRESHOLD:
            messages.append("Fan is ON because smoke is detected despite normal temperature.")
        elif temperature > TEMPERATURE_THRESHOLD and not smoke_detected:
            messages.append("Fan is ON because the temperature exceeds 30°C.")
        elif temperature > TEMPERATURE_THRESHOLD and smoke_detected:
            messages.append("Fan is ON because both high temperature and smoke are detected.")
    
    # Store the sensor data in DynamoDB
    try:
        # Ensure the partition key name matches the table schema exactly ("DeviceId")
        item = {
            'DeviceId': str(device_id),  # Make sure device_id is a string
            'Timestamp': timestamp,  # Sort key must match "Timestamp" in the table
            'Temperature': Decimal(str(temperature)),  # Convert to Decimal for DynamoDB
            'LightLevel': Decimal(str(light_level)),  # Convert to Decimal for DynamoDB
            'SmokeDetected': smoke_detected,
            'TouchDetected': touch_detected
        }
        
        # Debug print statement to verify item contents
        print("Item to store in DynamoDB:", json.dumps(item, default=str))
        
        response = table.put_item(Item=item)
        print(f"Sensor data for {device_id} stored in DynamoDB successfully. Response: {response}")
    except Exception as e:
        print(f"Error storing data to DynamoDB: {e}")
    
    # If there are any messages to send
    if messages:
        for message in messages:
            print("Message to send:", message)
            
            # Publish each message to SNS Topic (which will send the email)
            try:
                sns.publish(
                    TopicArn=SNS_TOPIC_ARN,
                    Message=message,
                    Subject="Home Sensor Status Alert"
                )
                print("Notification sent to user via SNS email.")
            except Exception as sns_error:
                print(f"Error sending SNS notification: {sns_error}")
        
        # Update the last notification time
        last_notification_time = current_time
    else:
        print("No conditions met. No notification sent.")

    return {
        'statusCode': 200,
        'body': json.dumps('Function executed. Sensor data stored and notifications sent based on conditions.')
    }
