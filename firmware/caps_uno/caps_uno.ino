#include <Servo.h>
Servo servo1, servo2, servo3, servo4;

#define servoPin1 2
#define servoPin2 3
#define servoPin3 4
#define servoPin4 5
#define infraRed1 6
#define infraRed2 7
#define inductive 8
#define capacitive 9

int trigPin = 10;
int echoPin = 11;

int ir1=0;
int ir2=0;
int ind=0;
int cap=0;

bool lanjutTahap2 = false;
bool tahap2selesai = true;
bool ambilsampah = false;
int counterAmbilSampah = 0;
bool servoDiam = false;
bool orangTerdeteksi = false;
unsigned long terakhirGerak = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(servoPin1, OUTPUT);
  pinMode(servoPin2, OUTPUT);
  pinMode(servoPin3, OUTPUT);
  pinMode(servoPin4, OUTPUT);
  pinMode(infraRed1, INPUT);
  pinMode(infraRed2, INPUT);
  pinMode(inductive, INPUT);
  pinMode(capacitive, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo3.attach(servoPin3);
  servo4.attach(servoPin4);
  servo1.write(47);
  servo2.write(88);
  servo3.write(92);
  servo4.write(100);

  delay(2000);

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int jarakGerak = bacaJarak(trigPin, echoPin);
  // Tahap 1: sensor infrared dan inductive
  ir1 = digitalRead(infraRed1);
  ind = digitalRead(inductive);
  // Tahap 2: sensor infrared dan capacitive
  ir2 = digitalRead(infraRed2); // Asumsi pakai sensor infrared yg sama
  cap = digitalRead(capacitive);

  Serial.print("ir1 : ");
  Serial.print(ir1);
  Serial.print(" | ir2 : ");
  Serial.println(ir2);
  Serial.print("ind : ");
  Serial.print(ind);
  Serial.print(" | cap : ");
  Serial.println(cap);
  Serial.print("Jarak : ");
  Serial.println(jarakGerak);

  // Sensor gerak → servo
  if (jarakGerak > 0 && jarakGerak < 15 && !orangTerdeteksi) {
    Serial.println("👤 Orang terdeteksi. Membuka tutup...");
    servo4.write(5); // Buka
    orangTerdeteksi = true;
    terakhirGerak = millis();
    delay(5000);
    ambilsampah = true;

    if (servoDiam){
      counterAmbilSampah = 0;
      servoDiam = false;
    }
  }

  // Tutup setelah 5 detik
  if (orangTerdeteksi && millis() - terakhirGerak > 5000) {
    servo4.write(100); // Tutup
    orangTerdeteksi = false;
    Serial.println("🔒 Menutup tutup tong.");
  }
  
  if (tahap2selesai && ambilsampah && !servoDiam){
    servo1.write(30);
    servo2.write(55);
    delay(1500);
    servo1.write(90);
    delay(2000);
    servo1.write(50);
    delay(1500);
    servo2.write(90);
    delay(1000);
    
    ambilsampah = true;
    counterAmbilSampah++;

    if (counterAmbilSampah >= 3){
      servoDiam = true;
      servo1.write(47);
      servo2.write(88);
      servo3.write(92);

      ir1 = digitalRead(infraRed1);

      if (ambilsampah && ir1 == HIGH){
        ambilsampah = false;
      }
    }
  }

  ir1 = digitalRead(infraRed1);
  
  if (ambilsampah && tahap2selesai && ir1 == LOW){
    Serial.println("Tahap 1 : IR1 Aktif, menunggu sensor logam...");
    delay(1000);
    
    counterAmbilSampah = 0;
    servoDiam = false;

    ind = digitalRead(inductive);

    if (ind == LOW) {
      Serial.println("Sampah logam terdeteksi");
      servo2.write(5); // servo ke kanan
      delay(2000);
      servo2.write(90);
      tahap2selesai = true;
      ambilsampah = true;
    } else {
      Serial.println("Sampah non-logam, lanjut ke tahap 2");
      servo2.write(175); // servo ke kiri
      delay(2000);
      servo2.write(90);
      lanjutTahap2 = true;
      tahap2selesai = false;
      ambilsampah = false;
    }
  }else if (ambilsampah && ir1 == HIGH){
    delay(1000);
    ambilsampah = true;
  }

  if (lanjutTahap2 && !tahap2selesai && ir2 == LOW) {
    Serial.println("Tahap 2: IR2 aktif, menunggu sensor kapasitif...");
    delay(1000); // tunggu objek sampai bawah

    cap = digitalRead(capacitive); // baca ulang setelah delay

    if (cap == HIGH) {
      Serial.println("Sampah non-organik terdeteksi");
      servo3.write(175); // servo ke kanan
    } else {
      Serial.println("Sampah organik terdeteksi");
      servo3.write(5); // servo ke kiri
    }
    delay(2000);
    servo3.write(90);
    lanjutTahap2 = false;
    tahap2selesai = true;
    ambilsampah = true;
  }
}

int bacaJarak(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long durasi = pulseIn(echoPin, HIGH, 30000);
  if (durasi == 0) return -1;
  return durasi * 0.034 / 2;
}
