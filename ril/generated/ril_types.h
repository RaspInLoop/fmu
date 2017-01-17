/**
 * Autogenerated by Thrift Compiler (1.0.0-dev)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef ril_TYPES_H
#define ril_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/TBase.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>


namespace org { namespace raspinloop { namespace fmi {

struct ModelState {
  enum type {
    modelStartAndEnd = 1,
    modelInstantiated = 2,
    modelInitializationMode = 4,
    modelEventMode = 8,
    modelContinuousTimeMode = 16,
    modelStepComplete = 35,
    modelStepInProgress = 64,
    modelStepFailed = 128,
    modelStepCanceled = 256,
    modelTerminated = 512,
    modelError = 1024,
    modelFatal = 2048
  };
};

extern const std::map<int, const char*> _ModelState_VALUES_TO_NAMES;

struct Status {
  enum type {
    OK = 0,
    Warning = 1,
    Discard = 2,
    Error = 3,
    Fatal = 4,
    Pending = 5
  };
};

extern const std::map<int, const char*> _Status_VALUES_TO_NAMES;

struct Type {
  enum type {
    ModelExchange = 0,
    CoSimulation = 1
  };
};

extern const std::map<int, const char*> _Type_VALUES_TO_NAMES;

struct StatusKind {
  enum type {
    DoStepStatus = 0,
    PendingStatus = 1,
    LastSuccessfulTime = 2,
    Terminated = 3
  };
};

extern const std::map<int, const char*> _StatusKind_VALUES_TO_NAMES;

class EventInfo;

class Instance;

typedef struct _EventInfo__isset {
  _EventInfo__isset() : newDiscreteStatesNeeded(false), terminateSimulation(false), nominalsOfContinuousStatesChanged(false), valuesOfContinuousStatesChanged(false), nextEventTimeDefined(false), nextEventTime(false) {}
  bool newDiscreteStatesNeeded :1;
  bool terminateSimulation :1;
  bool nominalsOfContinuousStatesChanged :1;
  bool valuesOfContinuousStatesChanged :1;
  bool nextEventTimeDefined :1;
  bool nextEventTime :1;
} _EventInfo__isset;

class EventInfo : public virtual ::apache::thrift::TBase {
 public:

  EventInfo(const EventInfo&);
  EventInfo& operator=(const EventInfo&);
  EventInfo() : newDiscreteStatesNeeded(0), terminateSimulation(0), nominalsOfContinuousStatesChanged(0), valuesOfContinuousStatesChanged(0), nextEventTimeDefined(0), nextEventTime(0) {
  }

  virtual ~EventInfo() throw();
  bool newDiscreteStatesNeeded;
  bool terminateSimulation;
  bool nominalsOfContinuousStatesChanged;
  bool valuesOfContinuousStatesChanged;
  bool nextEventTimeDefined;
  double nextEventTime;

  _EventInfo__isset __isset;

  void __set_newDiscreteStatesNeeded(const bool val);

  void __set_terminateSimulation(const bool val);

  void __set_nominalsOfContinuousStatesChanged(const bool val);

  void __set_valuesOfContinuousStatesChanged(const bool val);

  void __set_nextEventTimeDefined(const bool val);

  void __set_nextEventTime(const double val);

  bool operator == (const EventInfo & rhs) const
  {
    if (!(newDiscreteStatesNeeded == rhs.newDiscreteStatesNeeded))
      return false;
    if (!(terminateSimulation == rhs.terminateSimulation))
      return false;
    if (!(nominalsOfContinuousStatesChanged == rhs.nominalsOfContinuousStatesChanged))
      return false;
    if (!(valuesOfContinuousStatesChanged == rhs.valuesOfContinuousStatesChanged))
      return false;
    if (!(nextEventTimeDefined == rhs.nextEventTimeDefined))
      return false;
    if (!(nextEventTime == rhs.nextEventTime))
      return false;
    return true;
  }
  bool operator != (const EventInfo &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const EventInfo & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(EventInfo &a, EventInfo &b);

inline std::ostream& operator<<(std::ostream& out, const EventInfo& obj)
{
  obj.printTo(out);
  return out;
}

typedef struct _Instance__isset {
  _Instance__isset() : instanceName(false), componentRef(false), GUID(false), state(false) {}
  bool instanceName :1;
  bool componentRef :1;
  bool GUID :1;
  bool state :1;
} _Instance__isset;

class Instance : public virtual ::apache::thrift::TBase {
 public:

  Instance(const Instance&);
  Instance& operator=(const Instance&);
  Instance() : instanceName(), componentRef(0), GUID(), state((ModelState::type)0) {
  }

  virtual ~Instance() throw();
  std::string instanceName;
  int64_t componentRef;
  std::string GUID;
  ModelState::type state;

  _Instance__isset __isset;

  void __set_instanceName(const std::string& val);

  void __set_componentRef(const int64_t val);

  void __set_GUID(const std::string& val);

  void __set_state(const ModelState::type val);

  bool operator == (const Instance & rhs) const
  {
    if (!(instanceName == rhs.instanceName))
      return false;
    if (!(componentRef == rhs.componentRef))
      return false;
    if (!(GUID == rhs.GUID))
      return false;
    if (!(state == rhs.state))
      return false;
    return true;
  }
  bool operator != (const Instance &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Instance & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(Instance &a, Instance &b);

inline std::ostream& operator<<(std::ostream& out, const Instance& obj)
{
  obj.printTo(out);
  return out;
}

}}} // namespace

#endif
