//Motores
int dir[]   = {2,5};
int steps[] = {3,6};
int resetp[] = {4,7};
int num_mot = 2;

// Leds
int blue = 13;
int green = 12;
int red = 11;

//FINALES de CARRERA
int endstop[] = {
 A0, A1};

// Variables de motores
long velocidad[] = {
 10000,10000};
long pasos[] = {
 0,0};
long distance[] = {
 0,0};
boolean m_act[] = {
 false,false};

// Otras variables
boolean paused = false;
boolean programmed = false;
String msg;
long mymillis[] = {0,0};
long tiempo[] = {0,0};

void setup() {
 Serial.begin(9600);
 for (int i=0;i<num_mot;i++){
   pinMode(steps[i], OUTPUT);
   pinMode(dir[i], OUTPUT);
   pinMode(resetp[i], OUTPUT);
   digitalWrite(resetp[i], LOW);
 }
 pinMode(blue, OUTPUT);
 pinMode(green, OUTPUT);
 pinMode(red, OUTPUT);
 aviso(green,1);
}

void loop() {
 if( Serial.available() > 0 ){
   msg = Serial.readString();
   msg.toLowerCase();
   Serial.println(msg);
   if (msg.substring(0, 3) == "set") {
     String pro = msg.substring(4, 5);
     int mot = msg.substring(5, 6).toInt();
     int num = msg.substring(7, 9).toInt();
     int expo = msg.substring(9, 10).toInt();

     if ((pro == "v" || pro == "p")&& mot < num_mot) {
       if (pro == "v") velocidad[mot] = pow(10, expo) * num;
       if (pro == "p") pasos[mot] = pow(10, expo) * num;
       Serial.print("Set ");
       Serial.print(pro);
       Serial.print(mot);
       Serial.print(" to ");
       Serial.println(pow(10, expo) * num);
       aviso(green,2);
     }
     else {
       aviso(red,1);
       Serial.println("Set - Comandos:");
       Serial.println("set [v/p][0/1] ##^");
       Serial.println("ex: set p0 503");
     }
   }
   else if (msg.substring(0, 4) == "prog") {
     if ((msg.substring(11, 14)).toInt() < 999) {
       programmed = true;
       int mot = msg.substring(5, 6).toInt();
       int num = msg.substring(7, 9).toInt();
       int expo = msg.substring(9, 10).toInt();
       mymillis[mot] = 0;
       tiempo[mot] = msg.substring(11, 14).toInt() * 60000;
       pasos[mot] = long(pow(10, expo) * num);
       distance[mot] = pasos[mot];
       Serial.print("Prog: ");
       Serial.print(pasos[mot]);
       Serial.print(" on ");
       Serial.print(mot);
       Serial.print(" for ");
       Serial.print(tiempo[mot]/1000);
       Serial.println("s");
       aviso(green,2);
     }
     else {
       aviso(red,1);
       Serial.println("Prog - Comandos:");
       Serial.println("prog [0/1] ### ###");
       Serial.println("ex: prog 1 103 060");
     }
   }
   else if (msg.substring(0, 4) == "move") {
     programmed = false;
     if ((msg.substring(7, 10)).toInt() < 999) {
       int mot = msg.substring(5, 6).toInt();
       int num = msg.substring(7, 9).toInt();
       int expo = msg.substring(9, 10).toInt();
       pasos[mot] = long(pow(10, expo) * num);
       Serial.print("P: ");
       Serial.print(pasos[mot]);
       Serial.print(" on ");
       Serial.println(mot);
       aviso(green,2);
     }
     else {
       aviso(red,1);
       Serial.println("Move - Comandos:");
       Serial.println("move [0/1] ##^");
       Serial.println("ex: move 1 103");
     }
   }
   else if (msg.substring(0, 4) == "turn") {
     programmed = false;
     if ((msg.substring(5, 8)).toInt() < 999) {
       int mot = msg.substring(5, 6).toInt();
       int deg = msg.substring(7, 10).toInt();
       pasos[mot] = long(deg / 1.8);
       aviso(green,2);
     }
     else {
       aviso(red,1);
       Serial.println("Turn - Comandos:");
       Serial.println("turn [0/1] ###º");
       Serial.println("ex: turn 0 090");
     }
   }
   else if(msg.substring(0, 4) == "stop"){
     paused = true;
     aviso(green,2);
   }
   else if(msg.substring(0, 4) == "cont"){
     paused = false;
     aviso(green,2);
   }
   else if(msg.substring(0, 5) == "reset"){
     reset();
     aviso(green,2);
   }
   else if (msg.substring(0, 6) == "status") {
     int status_es[2];
     for (int i = 0;i<num_mot; i++) {
       status_es[i] = analogRead(endstop[i]);
       Serial.print("M");
       Serial.print(i);
       Serial.print(" P: ");
       Serial.print(pasos[i]);
       Serial.print(" V: ");
       Serial.println(velocidad[i]);
     }
     Serial.print("ES: ");
     Serial.print(status_es[0]);
     Serial.print(" ");
     Serial.println(status_es[1]);
     if(paused) Serial.print("pauseON");
     else Serial.print("pauseOFF");
     if(programmed) Serial.println(" programON");
     else Serial.println(" programOFF");
   }
   else {
     aviso(red,1);
     Serial.println("Lista de comandos:");
     Serial.println("set move turn status stop cont");
     Serial.println("‘comando –’ para +info");
   }
 }
 comprobar_pos();
 if(!paused) mover_motor();
 msg = "";
}

void aviso(int pin, int loops) {
 for (int i = 0; i < loops; i++) {
   digitalWrite(pin, HIGH);
   delay(100);
   digitalWrite(pin, LOW);
   if((i+1) < loops) delay(100);
 }
}

void mover_motor(){
 for (int i=0;i<num_mot;i++){
   if (velocidad[i] < 1000 || velocidad[i] > 100000) velocidad[i] = 1000;
   if (pasos[i] < 0) pasos[i] = 0;
   if(pasos[i] > 0 && (!programmed || ((millis() - mymillis[i]) > tiempo[i]/distance[i]))){
     if(programmed){
       mymillis[i] = millis();
       Serial.print(i);
       Serial.print(" step ");
       Serial.println(pasos[i]);
     }
     if (m_act[i] == false) {
       m_act[i] = true;
       digitalWrite(resetp[i], LOW);
       delay(100);
       digitalWrite(resetp[i], HIGH);
       //digitalWrite(dir[i], HIGH);
     }
     digitalWrite(steps[i], HIGH);
     digitalWrite(steps[i], LOW);
     if (pasos[i] == 1) digitalWrite(resetp[i], LOW);
     else delayMicroseconds(velocidad[i]);
     pasos[i--];
     if (pasos[i] < 1) m_act[i] = false;
   }
 }
}

void comprobar_pos() {
 int status_es[2];
 for (int i=0;i<num_mot;i++){
   status_es[i] = analogRead(endstop[i]);
   if (status_es[i] > 1020 && pasos[i] > 0){
     pasos[i] = 0;
     Serial.println("EndStop activado!");
   }
 }
}

void reset(){
 for (int i=0;i<num_mot;i++){
   pasos[i] = 0;
   distance[i] = 0;
   velocidad[i] = 1000;
   m_act[i] = false;
   tiempo[i] = 0;
 }
 programmed = false;
 msg = "";
}
