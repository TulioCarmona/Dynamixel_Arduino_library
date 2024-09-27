//Dynamixel library for 1.0 protocol motors
//Author: Tulio Carmona and Erick Carmona
//https://github.com/TulioCarmona/Dynamixel_Arduino_library

// Intructions:
#define pingCmd (uint8_t)0x01
#define readCmd (uint8_t)0x02
#define writeCmd (uint8_t)0x03
#define reg_write (uint8_t)0x04
#define action (uint8_t)0x05
#define factory_reset (uint8_t)0x06
#define reboot (uint8_t)0x08
#define sync_write (uint8_t)0x83
#define bulk_reak (uint8_t)0x92
 


class dynamixelProtocol1
{
  private:
  uint8_t header = 0xFF;
  short int ctrlPin;
  HardwareSerial *serialH;
  uint8_t data[10];
  //uint8_t data2[20];

  public:
  dynamixelProtocol1(int dir_pin, HardwareSerial *serial){
    ctrlPin = dir_pin;
    serialH = serial;
  };
  //~dynamixelProtocol1();
  
  bool begindxl(int baudrate){
    Serial.println("Establishing conexion with dynamixel...");
    serialH->begin(baudrate);
    serialH->setTimeout(100);  
  }

  uint8_t getChecksum(uint8_t *firstVal, uint8_t length){
    //Checksum = ~( ID + Length + Instruction + Parameter1 + … Parameter N )
    uint8_t checksum = 0;
    for(uint8_t i=0; i < length; i++){
      checksum += *(firstVal + i);
    }
    return ~checksum;
  }

  uint8_t readError(uint8_t id, uint8_t *data){
    bool chain_error = false;
    uint8_t error = 0x00;
    for(int i = 0; i < 2; i ++){
      if(*(data+i) != 0xFF)
        chain_error = true;
        break;
    }
    if(chain_error == false){
      if(*(data+2) != id) //verifies id
        chain_error = true;
      else{
        error = *(data+4); //get error type
      }
    }
    if(chain_error == true)
      error = 0xFF;
    return error;
  }

  uint16_t getValue(uint8_t *data){
    uint8_t len = *(data+3)-2;
    uint16_t value = 0x00;
    for(int i = 0; i < len; i++){
      value += *(data+5+i) << (8*i);
    }
    return value;
  }

  void ping(uint8_t id){
    uint8_t data2[30];
    unsigned long t0;
    data[0] = header;
    data[1] = header;
    data[2] = id;	// id
    data[3] = 0x02;	// length = number of parameters + 2 (chksum and instruction)
    data[4] = pingCmd;	// instruction 
    data[5]	= getChecksum(&data[2], 3);

    Serial.println("Sending ping");
    digitalWrite(ctrlPin, HIGH);
    serialH->write(data,6);
    serialH->flush();
    
    digitalWrite(ctrlPin, LOW);

    t0 = millis();
    while(!serialH->available() && (millis()-t0) < 100);
    if(serialH->available()){
      serialH->readBytes(data2,6);
    }

    for(int i=0; i<6; i++){
      Serial.println(data2[i],HEX);
    }
    Serial.println();
  }

  void write1byte(uint8_t id, uint8_t addr, uint8_t value){
    bool endR = false;
    int index = 0;
    unsigned long t0;
    data[0] = header;
    data[1] = header;
    data[2] = id;
    data[3] = 0x04;
    data[4] = 0x03; //write instruction
    data[5] = addr;
    data[6] = value;
    data[7] = getChecksum(&data[2],5);
    //Send comand
    digitalWrite(ctrlPin, HIGH);
    serialH->write(data,8);
    
    serialH->flush();
    delay(10); //Compensa por la falta de lectura de la respuesta del motor
    /*digitalWrite(ctrlPin, LOW);

    t0 = millis();
    while(!serialH->available() && (millis()-t0) < 100);
    if(serialH->available()){
      serialH->readBytes(data2,6);
    }

    for(int i=0; i<6; i++){
      Serial.println(data2[i],HEX);
    }
    Serial.println();*/
  }

  void write2byte(uint8_t id, uint8_t addr, uint16_t value){
    uint8_t val_lb = value; //lower byte
    uint8_t val_ub = value >> 8; //upper byte
    data[0] = header;
    data[1] = header;
    data[2] = id;
    data[3] = 0x05;
    data[4] = 0x03; //write instruction
    data[5] = addr;
    data[6] = val_lb;
    data[7] = val_ub;
    data[8] = getChecksum(&data[2],6);
    //Send comand
    digitalWrite(ctrlPin, HIGH);
    serialH->write(data,9);

    serialH->flush();
    delay(10);//Compensa por la falta de lectura de la respuesta del motor
  }

  void write4byte(uint8_t id, uint8_t addr, uint16_t val1, uint16_t val2){
    uint8_t data[12];
    uint8_t val1_lb = val1; //lower byte
    uint8_t val1_ub = val1 >> 8; //upper byte
    uint8_t val2_lb = val2; //lower byte
    uint8_t val2_ub = val2 >> 8; //upper byte
    data[0] = header;
    data[1] = header;
    data[2] = id;
    data[3] = 0x07;
    data[4] = 0x03; //write instruction
    data[5] = addr;
    data[6] = val1_lb;
    data[7] = val1_ub;
    data[8] = val2_lb;
    data[9] = val2_ub;
    data[10] = getChecksum(&data[2],8);
    //Send comand
    digitalWrite(ctrlPin, HIGH);
    serialH->write(data,11);
    serialH->flush();
    delay(10);//Compensa por la falta de lectura de la respuesta del motor
  }

  void wheelMode(uint8_t id, uint8_t cw_limit){
    write4byte(id, cw_limit, 0, 0); //writes consecutive directions, first cw limit, then ccw limit
  }

  void jointMode(uint8_t id, uint8_t cw_limit){
    write4byte(id, cw_limit, 0, 4095);
  }

  void multi_turnMode(uint8_t id, uint8_t cw_limit){
    write4byte(id, cw_limit, 4095, 4095);
  }

  uint16_t movingSpeed(uint8_t speed, bool dir){
    uint16_t w_speed = 0;
    if(speed > 100 || speed < 0){
      Serial.println("Invalid speed vallue. Speed value must be between 0 and 100");
    }
    else{
      w_speed = float(speed)*1023/100;
      if(dir == 0){ //CW direction
        w_speed = w_speed + 1024;
      }
      //else: CCW direction
    }
    return w_speed;
  }

  uint8_t read1byte(uint8_t id, uint8_t addr){
    uint8_t data2[15];
    unsigned long t0;
    data[0] = header;
    data[1] = header;
    data[2] = id;	// id
    data[3] = 0x04;	// length = number of parameters + 2 (chksum and instruction)
    data[4] = readCmd;	// instruction read
    data[5] = addr; //address
    data[6] = 0x01; //lenght of data
    data[7]	= getChecksum(&data[2], 5);

    digitalWrite(ctrlPin, HIGH);
    serialH->write(data,8);

    serialH->flush();
    digitalWrite(ctrlPin, LOW);
    t0 = millis();
    while(!serialH->available() && (millis()-t0) < 100);
    if(serialH->available()){
      serialH->readBytes(data2,7);
    }
    /*for(int i=0; i<7; i++){
      Serial.println(data2[i],HEX);
    }
    Serial.println();*/
    uint8_t e = readError(id, &data2[0]);
    if(e == 0x00){
      uint8_t value = getValue(&data2[0]);
      return value;
    }
    else{
      Serial.print("Error: ");
      Serial.println(e);
    }
  }

  uint16_t read2byte(uint8_t id, uint8_t addr){
    uint8_t data2[15];
    unsigned long t0;
    data[0] = header;
    data[1] = header;
    data[2] = id;	// id
    data[3] = 0x04;	// length = number of parameters + 2 (chksum and instruction)
    data[4] = readCmd;	// instruction read
    data[5] = addr; //address
    data[6] = 0x02; //lenght of data
    data[7]	= getChecksum(&data[2], 5);

    digitalWrite(ctrlPin, HIGH);
    serialH->write(data,8);

    serialH->flush();
    digitalWrite(ctrlPin, LOW);

    t0 = millis();
    while(!serialH->available() && (millis()-t0) < 100);
    if(serialH->available()){
      serialH->readBytes(data2,8);
    }
    /*for(int i=0; i<7; i++){
      Serial.println(data2[i],HEX);
    }
    Serial.println();*/
    uint8_t e = readError(id, &data2[0]);
    if(e == 0x00){
      uint16_t value = getValue(&data2[0]);
      return value;
    }
    else{
      Serial.print("Error: ");
      Serial.println(e);
    }
  }

};