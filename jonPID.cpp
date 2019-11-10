#include "jonPID.h"


PID::PID() {
}

PID::PID(String PIDname) {
  this->_PIDname = PIDname;
}
  
PID::PID(String PIDname,unsigned long updateInterval) {
  this->_PIDname = PIDname;
  this->_updateInterval = updateInterval;
}
  
PID::PID(String PIDname,unsigned long updateInterval,double target) { 
  this->_PIDname = PIDname;
  this->_updateInterval = updateInterval;
  this->_target = target;
}
  
PID::PID(String PIDname,unsigned long updateInterval,double target, double kP, double kI, double kD) {
  this->_PIDname = PIDname;
  this->_updateInterval = updateInterval;
  this->_target = target;
  this->_kP = kP; 
  this->_kI = kI; 
  this->_kD = kD; 
}

double PID::calculateControl(double input) {
  // Actually does the PID calculation here. returns the control value (0.0-100.0)
  this->_lastupdated=millis();
  //this->_lastupdated += this->_updateInterval;
  // First calculate current error. This is simple. It's just the target minus the current temp (passed in as param)
  this->_terror = this->_target - input;
  // Calculate the control variable.
  this->control = ( this->_kP * this->_terror ) + (this->_kI * this->_terrordtime) - ( this->_kD * (input - this->_lastresult));
  // Clip control value to give percentage
  if (this->control < 0.0)   this->control=0.0;
  if (this->control > 100.0) this->control=100.0;
  // Store input for next time round the loop. Needed to calculate Derivative part of PID
  this->_lastresult = input;
  // If control value within range, then update error over time variable for next time. Needed to calculate Integral part of PID 
  // We dont want to do this when control is at bottom/top of its range because of huge ramp up/down of error over time.
  if (this->control > 0.0 && this->control < 100.0) this->_terrordtime += this->_terror;
  // Return the control value.
  return this->control;
}

bool PID::setPIDname(String newpidname) {
  this->_PIDname = newpidname;
  return true;
}
String PID::getPIDname() {
  return this->_PIDname;
}


bool PID::setUpdateInterval(unsigned long updateInterval) {
  if (updateInterval>=10000 && updateInterval<=300000) {
    this->_updateInterval = updateInterval;  
    return true;
  } else return false;
}
unsigned long PID::getUpdateInterval() {
  return this->_updateInterval;  
}

bool PID::setTarget(double target) {
  if (target>=50 && target<=250) {
    this->_target = target;
    return true;
  } else return false;
}

double PID::getLastResult() {
  return this->_lastresult;
}

double PID::getTarget() {
  return this->_target;
}
double PID::getkP() {
  return this->_kP;
}
double PID::getkI() {
  return this->_kI;
}
double PID::getkD() {
  return this->_kD;
}
bool PID::setkP(double kP) {
  if (kP>=0 && kP<=10.0) {
    this->_kP = kP;
    return true;
  } else return false;
}

bool PID::setkI(double kI) {
  if (kI>=0 && kI<=10.0) {
    this->_kI = kI;
    return true;
  } else return false;
}
bool PID::setkD(double kD) {
  if (kD>=0 && kD<=10.0) {
    this->_kD = kD;
    return true;
  } else return false;
}

double PID::getErrOverTime() {
  return this->_terrordtime;
}

bool PID::setErrOverTime(double eot) {
  this->_terrordtime = eot;
  return true;
}

bool PID::isRunning() {
  return this->_running;
}

bool PID::toggleRunning() {
  this->_running = !(this->_running);
  return this->_running;
}

void PID::begin(double starttemp){
  this->_running = true;  
  this->_lastresult = starttemp;
  this->_terrordtime = 0.0;
}

void PID::end() {
  this->_running = false;  
}

bool PID::update(double probe) {
  if (this->_running) {
    if ( (millis() - this->_lastupdated) > this->_updateInterval ) {
      double diff = probe - this->_lastresult; // Need to store this as next line will alter _lastresult
      this->control = calculateControl(probe);
      sprintf(this->datalogbuffer,"%.2f,%.2f,%d, %.2f,%.2f,%.2f, %.2f,%.2f,%.2f, %.2f,%.2f\0", probe,
              this->control,(int)(this->_target), this->_kP,this->_kI,this->_kD,
              this->_terror * this->_kP, this->_terrordtime * this->_kI, diff * this->_kD,
              this->_terror,this->_terrordtime);
      return true;
    } else { return false; }
  } else { return false; }
}
