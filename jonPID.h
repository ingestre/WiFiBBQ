#pragma once

#include "Arduino.h"

class PID {
  private:
    double        _kP = 2.0;
    double        _kI = 0.2;
    double        _kD = 4.0;
    double        _target = 225.0;
    double        _terror = 0.0;
    double        _terrordtime = 0.0;
    double        _lastresult;
    unsigned long _updateInterval = 60000;
    unsigned long _lastupdated = 0;
    String        _PIDname = "bbqPID";
    bool          _running = false;
    
    double        calculateControl(double input);
    
  
  public:
  char            datalogbuffer[128];
  double          control;
  
  PID();
  PID(String PIDname);
  PID(String PIDname,unsigned long updateInterval);
  PID(String PIDname,unsigned long updateInterval,double target);
  PID(String PIDname,unsigned long updateInterval,double target, double kP, double kI, double kD);

  bool          setPIDname(String newpidname);
  String        getPIDname();
  bool          setUpdateInterval(unsigned long updateInterval);
  unsigned long getUpdateInterval();
  bool          setTarget(double target);
  double        getTarget();
  double        getLastResult();
  double        getkP();
  double        getkI();
  double        getkD();
  bool          setkP(double kP);
  bool          setkI(double kI);
  bool          setkD(double kD);
  double        getErrOverTime();
  bool          setErrOverTime(double eot);
  bool          isRunning();
  bool          toggleRunning();
  void          begin(double starttemp);
  void          end();
  bool          update(double probe);
};
