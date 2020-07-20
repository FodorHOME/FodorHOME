bool cool() {
  static unsigned long bufferEmptyingStartTime;
  static unsigned long bufferLastFilled = 0;
  static bool isBufferLastFilledActive = false;
  
  auto bufferEmptyStop = [](){
    fullEmpty = false;
    currentJob = waterJob{StopNext};
  };

  if (cooling && levelOf(Buffer) > 0) if (digitalRead(flowPump) == RelayOff) digitalWrite(flowPump, RelayOn);
  else if (digitalRead(flowPump) == RelayOn) digitalWrite(flowPump, RelayOff);

  if (!cooling) return Continue;
  
  if (!isBufferEmptying()) {
    //start filling buffer if not full if not currently emptying 
    if (levelOf(Buffer) < 2) {
      currentJob = waterJob{NoStopNext, fromWell, toBuffer};
      return End;
    } else {
      currentJob = waterJob{StopNext};
      if (isBufferFilling()) {
        bufferLastFilled = millis();
        isBufferLastFilledActive = true;
      }
    }

    if (isWateringEmptying()) { //if watering is already running, return to run water()
     if (levelOf(Watering) > 0) return Continue;
     else { //if emptied, stop
       currentJob = waterJob{StopNext};
     }
    }

    if (bufferTemp > bufferTreshold) { //start buffer emptying if temperature exceeded
      if ((millis() - bufferLastFilled < bufferFilledTooSoonTreshold) && isBufferLastFilledActive) {
        fullEmpty = true;
        if (debug) Serial.println("fullEmpty true");
      }
      bufferEmptyingStartTime = millis();
      currentJob = waterJob{NoStopNext, fromBuffer, toWatering};
      return End;
    }
    
  } else {
    if (levelOf(Watering) == 2) { //if watering tank full, start the öntözést az öntözőből 
      bufferEmptyStop();
      beginWatering(1500, Cooling);//todo measure empty time
      return Continue;
    }
    if (fullEmpty) { //stop if empty or timer ran out or watering tank is full
      if (levelOf(Buffer) == 0) bufferEmptyStop();
      else return End;
    } else {
      if (millis() - bufferEmptyingStartTime > bufferEmptyingDuration) bufferEmptyStop();
      else return End;
    }
  }

  return Continue;
}
