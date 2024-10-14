package com.example.myapplication;

import android.os.Bundle;
import android.util.Log;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import org.eclipse.paho.android.service.MqttAndroidClient;
import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.json.JSONException;
import org.json.JSONObject;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    private TextView temperatureTextView;
    private Switch fanSwitch;
    private Switch bulbSwitch;
    private MqttAndroidClient mqttAndroidClient;
    private final String serverUri = "tcp://192.168.130.174:1883"; // Updated MQTT broker IP address and port
    private final String tempHumdDataTopic = "sensor/data";
    private final String fanTopic = "cmnd/PlugControl/POWER";
    private final String bulbTopic = "bulb/control";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        temperatureTextView = findViewById(R.id.temperatureTextView);
        fanSwitch = findViewById(R.id.fanSwitch);
        bulbSwitch = findViewById(R.id.bulbSwitch);

        mqttAndroidClient = new MqttAndroidClient(getApplicationContext(), serverUri, MqttClient.generateClientId());

        try {
            MqttConnectOptions options = new MqttConnectOptions();
            options.setCleanSession(true); // Ensure clean session

            mqttAndroidClient.connect(options, null, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.d(TAG, "Connection succeeded");
                    showToast("Connection succeeded");
                    subscribeToTopic(tempHumdDataTopic);

                    mqttAndroidClient.setCallback(new MqttCallback() {
                        @Override
                        public void connectionLost(Throwable cause) {
                            Log.d(TAG, "Connection lost", cause);
                        }

                        @Override
                        public void messageArrived(String topic, MqttMessage message) throws Exception {
                            if (topic.equals(tempHumdDataTopic)) {
                                String messageContent = new String(message.getPayload());
                                Log.d(TAG, "Message arrived: " + messageContent);
                                updateUIWithSensorData(messageContent);
                            }
                        }

                        @Override
                        public void deliveryComplete(IMqttDeliveryToken token) {
                            Log.d(TAG, "Delivery complete");
                        }
                    });
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.e(TAG, "Connection failed", exception);
                    showToast("Connection failed: " + exception.getMessage());
                }
            });
        } catch (MqttException e) {
            Log.e(TAG, "Exception while connecting", e);
            showToast("Exception while connecting: " + e.getMessage());
        }

        fanSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            publishMessage(fanTopic, isChecked ? "ON" : "OFF");
        });

        bulbSwitch.setOnCheckedChangeListener((buttonView, isChecked) -> {
            publishMessage(bulbTopic, isChecked ? "ON" : "OFF");
        });
    }

    private void subscribeToTopic(String topic) {
        try {
            mqttAndroidClient.subscribe(topic, 0, null, new IMqttActionListener() {
                @Override
                public void onSuccess(IMqttToken asyncActionToken) {
                    Log.d(TAG, "Subscription succeeded");
                    showToast("Subscribed to " + topic);
                }

                @Override
                public void onFailure(IMqttToken asyncActionToken, Throwable exception) {
                    Log.e(TAG, "Subscription failed", exception);
                    showToast("Subscription failed: " + exception.getMessage());
                }
            });
        } catch (MqttException e) {
            Log.e(TAG, "Exception while subscribing", e);
            showToast("Exception while subscribing: " + e.getMessage());
        }
    }

    private void publishMessage(String topic, String message) {
        try {
            MqttMessage mqttMessage = new MqttMessage();
            mqttMessage.setPayload(message.getBytes());
            mqttAndroidClient.publish(topic, mqttMessage);
            Log.d(TAG, "Message published: " + message);
            showToast("Message published: " + message);
        } catch (MqttException e) {
            Log.e(TAG, "Exception while publishing", e);
            showToast("Exception while publishing: " + e.getMessage());
        }
    }

    private void updateUIWithSensorData(String sensorData) {
        // Assuming sensor data is in format {"temperature":xx.x,"humidity":xx.x}
        try {
            JSONObject jsonObject = new JSONObject(sensorData);
            double temperature = jsonObject.getDouble("temperature");
            double humidity = jsonObject.getDouble("humidity");
            runOnUiThread(() -> temperatureTextView.setText("Temperature: " + temperature
                    + "°C\nHumidity: " + humidity + "%"));
        } catch (JSONException e) {
            Log.e(TAG, "JSON parsing error", e);
        }
    }

    private void showToast(String message) {
        runOnUiThread(() -> Toast.makeText(this, message, Toast.LENGTH_SHORT).show());
    }
}
