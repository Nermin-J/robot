Online programming of a Robot 

Robot consists of 4 servo motors.

![image](https://user-images.githubusercontent.com/81052940/111934864-94d1d480-8ac2-11eb-8542-c54bbc017d75.png)

![image](https://user-images.githubusercontent.com/81052940/111934024-ec6f4080-8ac0-11eb-9d76-8b46e6650de3.png)

Every servo motor has initial position defined in program. We use mobile app to move robot in desired positions.
![image](https://user-images.githubusercontent.com/81052940/111934110-1294e080-8ac1-11eb-8a34-f69083cae8f4.png)

First of all, we must connect client app (android app) with server (ESP32 Wi-Fi module) entering IP address of the server in mobile app. 

Robot can move in "automatic" and "manual" modes.
Manual mode implies moving regarding clicked direction button. 

When we click "SAVE" button, we saves current position of the robot. Then we can move robot on the other position and click save. When we save desired number of position, we can click button "RUN". After that, robot enters "automatic" mode which causes robot to move regarding saved positions. Any time we can click "STOP". In that case, robot will stop in current position and when we click "RUN" again, he will continue moving in automatic mode. 

"RESET" button resets saved positions.
