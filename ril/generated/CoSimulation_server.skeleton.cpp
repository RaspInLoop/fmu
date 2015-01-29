// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "CoSimulation.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::org::raspinloop::fmi;

class CoSimulationHandler : virtual public CoSimulationIf {
 public:
  CoSimulationHandler() {
    // Your initialization goes here
  }

  void getVersion(std::string& _return) {
    // Your implementation goes here
    printf("getVersion\n");
  }

  void getTypesPlatform(std::string& _return) {
    // Your implementation goes here
    printf("getTypesPlatform\n");
  }

  void instanciate(Instance& _return, const std::string& instanceName, const Type::type fmuType, const std::string& fmuGUID, const std::string& fmuResourceLocation, const bool visible, const bool loggingOn) {
    // Your implementation goes here
    printf("instanciate\n");
  }

  Status::type setupExperiment(const Instance& c, const bool toleranceDefined, const double tolerance, const double startTime, const bool stopTimeDefined, const double stopTime) {
    // Your implementation goes here
    printf("setupExperiment\n");
  }

  Status::type enterInitializationMode(const Instance& c) {
    // Your implementation goes here
    printf("enterInitializationMode\n");
  }

  Status::type exitInitializationMode(const Instance& c) {
    // Your implementation goes here
    printf("exitInitializationMode\n");
  }

  Status::type terminate(const Instance& c) {
    // Your implementation goes here
    printf("terminate\n");
  }

  Status::type reset(const Instance& c) {
    // Your implementation goes here
    printf("reset\n");
  }

  void freeInstance(const Instance& c) {
    // Your implementation goes here
    printf("freeInstance\n");
  }

  void getReal(std::vector<double> & _return, const Instance& c, const std::vector<int32_t> & refs) {
    // Your implementation goes here
    printf("getReal\n");
  }

  void getInteger(std::vector<int32_t> & _return, const Instance& c, const std::vector<int32_t> & refs) {
    // Your implementation goes here
    printf("getInteger\n");
  }

  void getBoolean(std::vector<bool> & _return, const Instance& c, const std::vector<int32_t> & refs) {
    // Your implementation goes here
    printf("getBoolean\n");
  }

  void getString(std::vector<std::string> & _return, const Instance& c, const std::vector<int32_t> & refs) {
    // Your implementation goes here
    printf("getString\n");
  }

  Status::type setReal(const Instance& c, const std::map<int32_t, double> & ref_values) {
    // Your implementation goes here
    printf("setReal\n");
  }

  Status::type setInteger(const Instance& c, const std::map<int32_t, int32_t> & ref_values) {
    // Your implementation goes here
    printf("setInteger\n");
  }

  Status::type setBoolean(const Instance& c, const std::map<int32_t, bool> & ref_values) {
    // Your implementation goes here
    printf("setBoolean\n");
  }

  Status::type setString(const Instance& c, const std::map<int32_t, std::string> & ref_values) {
    // Your implementation goes here
    printf("setString\n");
  }

  Status::type setRealInputDerivatives(const Instance& c, const std::map<int32_t, int32_t> & ref_orders, const std::map<int32_t, double> & ref_values) {
    // Your implementation goes here
    printf("setRealInputDerivatives\n");
  }

  Status::type setRealOutputDerivatives(const Instance& c, const std::map<int32_t, int32_t> & ref_orders, const std::map<int32_t, double> & ref_values) {
    // Your implementation goes here
    printf("setRealOutputDerivatives\n");
  }

  Status::type cancelStep(const Instance& c) {
    // Your implementation goes here
    printf("cancelStep\n");
  }

  Status::type doStep(const Instance& c, const double currentCommunicationPoint, const double communicationStepSize, const bool noSetFMUStatePriorToCurrentPoint) {
    // Your implementation goes here
    printf("doStep\n");
  }

  Status::type getStatus(const Instance& c, const StatusKind::type s) {
    // Your implementation goes here
    printf("getStatus\n");
  }

  int32_t getIntegerStatus(const Instance& c, const StatusKind::type s) {
    // Your implementation goes here
    printf("getIntegerStatus\n");
  }

  double getRealStatus(const Instance& c, const StatusKind::type s) {
    // Your implementation goes here
    printf("getRealStatus\n");
  }

  bool getBooleanStatus(const Instance& c, const StatusKind::type s) {
    // Your implementation goes here
    printf("getBooleanStatus\n");
  }

  void getStringStatus(std::string& _return, const Instance& c, const StatusKind::type s) {
    // Your implementation goes here
    printf("getStringStatus\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<CoSimulationHandler> handler(new CoSimulationHandler());
  shared_ptr<TProcessor> processor(new CoSimulationProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

