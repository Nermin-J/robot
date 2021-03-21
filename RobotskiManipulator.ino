#include <WiFi.h>
#include <Servo.h>

// Motor class to store necessary information about every motor
class motor
{
public:
  Servo servoObject = Servo();
  int currentPosition; // Current position (where the motor is now) to move from in next step
};

int pins[] = {12, 13, 14, 27};                            // Signal pins for servo motors (where the signal pins are attached)
#define numberOfMotors sizeof(pins) / sizeof(int)         // Number of motors we have in array "pins"
#define maxNumberOfPositions 50                           // Max number of positions we can save and run
int initialPositions[numberOfMotors] = {90, 35, 130, 45}; // initial position for all motors
int servosSP[numberOfMotors * maxNumberOfPositions];      // SP means "Saved Positions" - Array in which we save positions and let robot move regarding these positions
int index = 0;                                            // Index of the position we want to save in "save" mode
motor motors[numberOfMotors];

String message = "";   // Message sent from client
int motorStep = 2;     // How many steps we want to make before pause
int motor = 0;         // Index of a motor we want to move
String direction = ""; // Choosed direction in the mobile application
String motorString = "";

int shift = 0;              // Sets to value shift = motorStep or shift = -motorStep depending on selected direction
String button = "";         // Used to store message if one of special purpose buttons is pressed (STOP, SAVE, RUN, RESET)
int delayBetweenSteps = 40; // pause between two consecutive motor steps
String previousMode = "STOP";

// Communication configuration
char *ssid; // access point name
char *password;
WiFiServer server(80);

void setup()
{
  Serial.begin(115200);

  // Configure pins and set all servo motors in initial position
  for (int motorIndex = 0; motorIndex < numberOfMotors; motor++)
  {
    motors[motorIndex].servoObject.attach(pins[motorIndex]);
    motors[motorIndex].servoObject.write(initialPositions[motorIndex]);
    motors[motorIndex].currentPosition = initialPositions[motorIndex];
    delay(100);
  }

  // Establish Wifi connection and start server
  Serial.println();
  Serial.println();
  Serial.print("Connection to the network ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connection established.");
  Serial.print("Server's IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop()
{
  // First we read message sent from the client
  readMessage();
  if (motor != 0)
  {
    motors[motor - 1].currentPosition = motors[motor - 1].servoObject.read() + shift;
    motors[motor - 1].servoObject.write(motors[motor - 1].servoObject.read() + shift); // move the motor >shift< steps from current position
  }

  if (button == "SAVE" || button == "RESET" || button == "RUN")
  {
    if (button != previousMode)
    {
      previousMode = button;

      if (button == "SAVE") // If button "SAVE" is pressed in app, save current position of all motors
      {
        for (int i = 0; i < numberOfMotors; i++)
          servosSP[i + index] = motors[i].currentPosition;
        index += numberOfMotors;
      }

      else if (button == "RESET") // If button "RESET" is pressed in app, reset saved positions
      {
        memset(servosSP, 0, sizeof(servosSP));
        index = 0;
      }

      else // If button "RUN" is pressed in app, servo motors run following saved positions
        runServos();
    }
  }
  delay(delayBetweenSteps);
}

void readMessage()
{
  WiFiClient client = server.available(); // wait for client. If there is no client with prepared data, client = false

  if (client)
  {
    message = "";
    while (client.connected())
    {
      if (client.available()) // Returns amount of data sent from client
      {
        char character = client.read();
        if (character == '\n')
        {
          if (message.length() == 0)
          {
            //HEADER
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            break;
          }

          else
          {
            if (message.startsWith("GET /"))
            {
              if (message[5] == 'R' || message[5] == 'L') // Extract symbol for direction (R = right, L = left)
              {
                direction = message[5];
                shift = direction == "R" ? motorStep : -motorStep; // Sets motor shift regarding symbol for direction
                motorString = message[6];
                motor = motorString.toInt();
                Serial.print(direction);
                Serial.println(motor);
                previousMode = "STOP";
              }

              else
              {
                motor = 0;
                shift = 0;
                button = message.substring(5, message.length() - 9);
                Serial.println(button);
              }
            }
            message = "";
          }
        }

        else if (character != '\r')
          message += character;
      }
    }
    // Stop connection
    client.stop();
    Serial.println("Client Disconnected");
  }
}

int runServos()
{
  bool nuls = true;
  for (int i = 0; i < numberOfMotors; i++) // Checks if all saved positions are 0. If they are, that means we just did a reset and all saved positions are set to 0 using memset()
  {
    if (servosSP[i] != 0)
    {
      nuls = false;
      break;
    }
  }

  if (!nuls) // If saved positions are not 0, we start moving infinitely until we get "STOP" or "PAUSE" message
  {
    while (1)
    {
      for (int i = 0; i < index; i += numberOfMotors)
      {
        for (int j = 0; j < numberOfMotors; j++)
        {
          while (motors[j].currentPosition != servosSP[i + j])
          {
            readMessage();
            if (button == "STOP")
            {
              shift = 0;
              motor = 0;
              return 1;
            }
            else if (button == "PAUSE")
            {
              while (button != "RUN")
                readMessage();
            }

            if (motors[j].currentPosition < servosSP[i + j])
            {
              motors[j].currentPosition = motors[j].currentPosition + 1;
              motors[j].servoObject.write(motors[j].currentPosition + 1);
            }
            else
            {
              motors[j].currentPosition = motors[j].currentPosition - 1;
              motors[j].servoObject.write(motors[j].currentPosition - 1);
            }
            delay(delayBetweenSteps / 2);
          }
          delay(delayBetweenSteps * 5);
        }
      }
    }
  }
}
