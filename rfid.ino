#include <SPI.h>
#include <MFRC522.h>

#define SDA_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SDA_PIN, RST_PIN);

int recordNumber = 1;
unsigned long startTime;

int baseHour = 8, baseMinute = 0, baseSecond = 0;

struct StudentInfo {
  String uid;
  String rollNo;
  String studentName;
  String entryTime;
  bool present;
};

// Student list
StudentInfo studentList[] = {
  {"f34228f", "R01", "RONY ROY", "", false},
  {"a389bfbd", "R02", "PRIYA ROY", "", false}
};

const int totalStudents = sizeof(studentList) / sizeof(studentList[0]);

// Time function
String currentTime() {
  unsigned long totalSecs = (millis() - startTime) / 1000;

  int hour = baseHour + totalSecs / 3600;
  int minute = baseMinute + (totalSecs % 3600) / 60;
  int second = baseSecond + (totalSecs % 60);

  if (hour >= 24) hour %= 24;

  char timeString[9];
  sprintf(timeString, "%02d:%02d:%02d", hour, minute, second);
  return String(timeString);
}

// Find student index
int getStudentIndex(String cardId) {
  for (int i = 0; i < totalStudents; i++) {
    if (studentList[i].uid == cardId) return i;
  }
  return -1;
}

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  startTime = millis();

  Serial.println("S.No,Card Id,Roll Number,Name,Time In,Time Out,Status");
}

void loop() {

  // Wait for card
  if (!mfrc522.PICC_IsNewCardPresent() || 
      !mfrc522.PICC_ReadCardSerial()) return;

  // Read UID
  String cardId = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardId += String(mfrc522.uid.uidByte[i], HEX);
  }

  cardId.toLowerCase();

  String now = currentTime();
  String timeIn = "-", timeOut = "-", logStatus = "Unknown";

  int index = getStudentIndex(cardId);

  if (index != -1) {
    StudentInfo &student = studentList[index];

    if (!student.present) {
      student.present = true;
      student.entryTime = now;
      timeIn = now;
      logStatus = "STUDENT ENTERED";
    } else {
      timeIn = student.entryTime;
      timeOut = now;
      student.present = false;
      logStatus = "STUDENT EXITED";
    }

    Serial.print(recordNumber++);
    Serial.print(",");
    Serial.print(cardId);
    Serial.print(",");
    Serial.print(student.rollNo);
    Serial.print(",");
    Serial.print(student.studentName);
    Serial.print(",");
    Serial.print(timeIn);
    Serial.print(",");
    Serial.print(timeOut);
    Serial.print(",");
    Serial.println(logStatus);
  }
  else {
    Serial.print(recordNumber++);
    Serial.print(",");
    Serial.print(cardId);
    Serial.print(",Unknown,Unknown,");
    Serial.print(now);
    Serial.print(",-,");
    Serial.println("ACCESS DENIED");
  }

  delay(1000);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}