/* ---------------------------------------------------------------------------*
 * fmuTemplate.c
 * Implementation of the FMI interface based on functions and macros to
 * be defined by the includer of this file. 
 * If FMI_COSIMULATION is defined, this implements "FMI for Co-Simulation 2.0",
 * otherwise "FMI for Model Exchange 2.0".
 * The "FMI for Co-Simulation 2.0", implementation assumes that exactly the
 * following capability flags are set to fmi2True:
 *    canHandleVariableCommunicationStepSize, i.e. fmi2DoStep step size can vary
 * and all other capability flags are set to default, i.e. to fmi2False or 0.
 *
 * Revision history
 *  07.03.2014 initial version released in FMU SDK 2.0.0
 *  02.04.2014 allow modules to request termination of simulation, better time
 *             event handling, initialize() moved from fmi2EnterInitialization to
 *             fmi2ExitInitialization, correct logging message format in fmi2DoStep.
 *  10.04.2014 use FMI 2.0 headers that prefix function and types names with 'fmi2'.
 *  13.06.2014 when fmi2setDebugLogging is called with 0 categories, set all
 *             categories to loggingOn value.
 *  09.07.2014 track all states of Model-exchange and Co-simulation and check
 *             the allowed calling sequences, explicit isTimeEvent parameter for
 *             eventUpdate function of the model, lazy computation of computed values.
 *
 * Author: Adrian Tirea
 * Copyright QTronic GmbH. All rights reserved.
 * ---------------------------------------------------------------------------*/

#include "fmuTemplate.h"

#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "generated\CoSimulation.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace  org::raspinloop;


static fmi2String logCategoriesNames[] = { "logAll", "logError", "logFmiCall", "logEvent" };

		boost::shared_ptr<TTransport> fmi_socket;
		boost::shared_ptr<TTransport> fmi_transport;
		boost::shared_ptr<TProtocol> fmi_protocol;
		boost::shared_ptr<fmi::CoSimulationClient> client;

		std::map<fmi2Component, fmi::Instance> instances;
		std::map<fmi2Component, std::map<int, char*> > valueRefStringBufferByInstance;
		 

		BOOL APIENTRY DllMain(HMODULE hModule,
			DWORD  ul_reason_for_call,
			LPVOID lpReserved
			)
		{
			switch (ul_reason_for_call)
			{
			case DLL_PROCESS_ATTACH:
			{
				fmi_socket = boost::shared_ptr<TTransport>(new TSocket("localhost", 9090));
				fmi_transport = boost::shared_ptr<TTransport>(new TBufferedTransport(fmi_socket));
				fmi_protocol = boost::shared_ptr<TProtocol>(new TBinaryProtocol(fmi_transport));
				client = boost::shared_ptr<fmi::CoSimulationClient>(new fmi::CoSimulationClient(fmi_protocol));
				fmi_transport->open();
				break;			
			}
			case DLL_PROCESS_DETACH:
			{

				fmi_transport->close();
				instances.clear();				
				break;
			}

			case DLL_THREAD_DETACH:
			case DLL_THREAD_ATTACH:
				// should not be called from everal thread.
				break;
			}
				
			return TRUE;

		}


		// ---------------------------------------------------------------------------
		// Private helpers used below to validate function arguments
		// ---------------------------------------------------------------------------


		fmi2Boolean isCategoryLogged(ModelInstance *comp, int categoryIndex);

		inline void FILTERED_LOG(const fmi::Instance& instance, fmi2Status status, unsigned int categoryIndex, const std::string& msg, ...) {
			if (isCategoryLogged((ModelInstance*)instance.componentRef, categoryIndex)) {
				va_list vl;
				va_start(vl, msg);
				((ModelInstance*)instance.componentRef)->functions->logger(((ModelInstance*)instance.componentRef)->functions->componentEnvironment,
					instance.instanceName.c_str(),
					status,
					logCategoriesNames[categoryIndex],
					msg.c_str(),
					vl);
				va_end(vl);
			}
		}


		static fmi2Boolean invalidNumber(fmi::Instance *inst, const char *f, const char *arg, int n, int nExpected) {
			if (n != nExpected) {
				inst->state = fmi::ModelState::modelError;
				FILTERED_LOG(*inst, fmi2Error, LOG_ERROR, "%s: Invalid argument %s = %d. Expected %d.", f, arg, n, nExpected);
				return fmi2True;
			}
			return fmi2False;
		}

		static fmi2Boolean invalidState(fmi::Instance* inst, const char *f, int statesExpected) {
			if (!inst)
				return fmi2True;
			if (!(inst->state & statesExpected)) {
				inst->state = fmi::ModelState::modelError;
				FILTERED_LOG(*inst, fmi2Error, LOG_ERROR, "%s: Illegal call sequence.", f);
				return fmi2True;
			}
			return fmi2False;
		}

		static fmi2Boolean nullPointer(fmi::Instance* inst, const char *f, const char *arg, const void *p) {
			if (!p) {
				inst->state = fmi::ModelState::modelError;
				FILTERED_LOG(*inst, fmi2Error, LOG_ERROR, "%s: Invalid argument %s = NULL.", f, arg);
				return fmi2True;
			}
			return fmi2False;
		}



		static fmi2Status unsupportedFunction(fmi::Instance* inst, const char *fName, int statesExpected) {

			if (invalidState(inst, fName, statesExpected))
				return fmi2Error;
			FILTERED_LOG(*inst, fmi2OK, LOG_FMI_CALL, fName);
			FILTERED_LOG(*inst, fmi2Error, LOG_ERROR, "%s: Function not implemented.", fName);
			return fmi2Error;
		}

		// ---------------------------------------------------------------------------
		// Private helpers logger
		// ---------------------------------------------------------------------------

		// return fmi2True if logging category is on. Else return fmi2False.
		fmi2Boolean isCategoryLogged(ModelInstance *comp, int categoryIndex) {
			if (categoryIndex < NUMBER_OF_CATEGORIES
				&& (comp->logCategories[categoryIndex] || comp->logCategories[LOG_ALL])) {
				return fmi2True;
			}
			return fmi2False;
		}

		// ---------------------------------------------------------------------------
		// FMI functions
		// ---------------------------------------------------------------------------
		fmi2Component fmi2Instantiate(fmi2String instanceName, fmi2Type fmuType, fmi2String fmuGUID,
			fmi2String fmuResourceLocation, const fmi2CallbackFunctions *functions,
			fmi2Boolean visible, fmi2Boolean loggingOn) {
			// ignoring arguments: fmuResourceLocation, visible
			
			if (!functions->logger) {
				return NULL;
			}

			if (fmuType != fmi2Type::fmi2CoSimulation) {
				functions->logger(functions->componentEnvironment, instanceName, fmi2Error, "error",
					"fmi2Instantiate: Only Cosimulation is supported!");
				return NULL;
			}

			if (!functions->allocateMemory || !functions->freeMemory) {
				functions->logger(functions->componentEnvironment, instanceName, fmi2Error, "error",
					"fmi2Instantiate: Missing callback function.");
				return NULL;
			}
			if (!instanceName || strlen(instanceName) == 0) {
				functions->logger(functions->componentEnvironment, "?", fmi2Error, "error",
					"fmi2Instantiate: Missing instance name.");
				return NULL;
			}
			if (!fmuGUID || strlen(fmuGUID) == 0) {
				functions->logger(functions->componentEnvironment, instanceName, fmi2Error, "error",
					"fmi2Instantiate: Missing GUID.");
				return NULL;
			}
			fmi::Instance inst;
			client->instanciate(inst, std::string(instanceName), fmi::Type::CoSimulation, std::string(fmuGUID), std::string(fmuResourceLocation), visible>0, loggingOn>0);
			ModelInstance *comp = (ModelInstance *)functions->allocateMemory(1, sizeof(ModelInstance));
			if (comp) {								
				// set all categories to on or off. fmi2SetDebugLogging should be called to choose specific categories.
				for (unsigned int i = 0; i < NUMBER_OF_CATEGORIES; i++) {
					comp->logCategories[i] = loggingOn;
				}
			}
			if (!comp) {
					functions->logger(functions->componentEnvironment, instanceName, fmi2Error, "error",
					"fmi2Instantiate: Out of memory.");
					return NULL;
			}					
			comp->functions = functions;
			comp->componentEnvironment = functions->componentEnvironment;
			comp->loggingOn = loggingOn;
			inst.componentRef = (int64_t)comp;
			inst.state = fmi::ModelState::modelInstantiated;
			instances.insert(std::pair<fmi2Component, fmi::Instance>(comp, inst));
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2Instantiate Done");
			comp->functions->logger(comp->functions->componentEnvironment, "unknown", fmi2Error, "error",
				"fmi2SetDebugLogging");
			return comp;
			}


		fmi2Status fmi2SetupExperiment(fmi2Component c, fmi2Boolean toleranceDefined, fmi2Real tolerance,
			fmi2Real startTime, fmi2Boolean stopTimeDefined, fmi2Real stopTime) {

			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2SetupExperiment", MASK_fmi2SetupExperiment))
				return fmi2Error;
			
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetupExperiment: toleranceDefined=%d tolerance=%g",
					toleranceDefined, tolerance);
			
			return static_cast<fmi2Status>(client->setupExperiment(inst, toleranceDefined > 0, tolerance, startTime, stopTimeDefined > 0, stopTime));			
		}

		fmi2Status fmi2EnterInitializationMode(fmi2Component c) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2EnterInitializationMode", MASK_fmi2EnterInitializationMode))
				return fmi2Error;
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2EnterInitializationMode");
		
			fmi2Status status  = static_cast<fmi2Status>(client->enterInitializationMode(inst));
			inst.state = fmi::ModelState::type::modelInitializationMode;
			return status;
		}

		fmi2Status fmi2ExitInitializationMode(fmi2Component c) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2ExitInitializationMode", MASK_fmi2ExitInitializationMode))
				return fmi2Error;
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2ExitInitializationMode");
			fmi2Status status = static_cast<fmi2Status>(client->exitInitializationMode(inst));
			inst.state = fmi::ModelState::type::modelStepComplete;
			return status;
		}

		fmi2Status fmi2Terminate(fmi2Component c) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2Terminate", MASK_fmi2Terminate))
				return fmi2Error;
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2Terminate");

			fmi2Status status = static_cast<fmi2Status>(client->terminate(inst));
			inst.state = fmi::ModelState::type::modelTerminated;
			return status;
		}

		fmi2Status fmi2Reset(fmi2Component c) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2Reset", MASK_fmi2Reset))
				return fmi2Error;
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2Reset");			
			fmi2Status status = static_cast<fmi2Status>(client->reset(inst));
			inst.state = fmi::ModelState::type::modelInstantiated;
			return status;
		}

		void fmi2FreeInstance(fmi2Component c) {
			if (!c) 
				return;
			std::map<fmi2Component, fmi::Instance>::iterator it = instances.find(c);
			if (it == instances.end())
				return;

			fmi::Instance& inst = instances[c];
			
			if (invalidState(&inst, "fmi2FreeInstance", MASK_fmi2FreeInstance))
				return;

			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2FreeInstance");			

			std::map<int, char*>& valueRefStringBuffer = valueRefStringBufferByInstance[c];
			for (std::map<int, char*>::iterator it = valueRefStringBuffer.begin(); it != valueRefStringBuffer.end(); ++it){
				((ModelInstance *)c)->functions->freeMemory(it->second);
			}	
			valueRefStringBuffer.clear();
			client->freeInstance(it->second);
			instances.erase(it);
		}

		// ---------------------------------------------------------------------------
		// FMI functions: class methods not depending of a specific model instance
		// ---------------------------------------------------------------------------

		const char* fmi2GetVersion() {
			return fmi2Version;
		}

		const char* fmi2GetTypesPlatform() {
			return fmi2TypesPlatform;
		}

		// ---------------------------------------------------------------------------
		// FMI functions: logging control, setters and getters for Real, Integer,
		// Boolean, String
		// ---------------------------------------------------------------------------

		fmi2Status fmi2SetDebugLogging(fmi2Component c, fmi2Boolean loggingOn, size_t nCategories, const fmi2String categories[]) {
			// ignore arguments: nCategories, categories
			ModelInstance* comp = static_cast<ModelInstance*>(c);
			comp->functions->logger(comp->functions->componentEnvironment, "unknown", fmi2Error, "error",
				"fmi2SetDebugLogging");

			unsigned int i, j;			
			fmi::Instance& inst = instances[c];
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetDebugLogging");
			if (invalidState(&inst, "fmi2SetDebugLogging", MASK_fmi2SetDebugLogging))
				return fmi2Error;
			
			comp->loggingOn = loggingOn;
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetDebugLogging");

				// reset all categories
				for (j = 0; j < NUMBER_OF_CATEGORIES; j++) {
					comp->logCategories[j] = fmi2False;
				}

			if (nCategories == 0) {
				// no category specified, set all categories to have loggingOn value
				for (j = 0; j < NUMBER_OF_CATEGORIES; j++) {
					comp->logCategories[j] = loggingOn;
				}
			}
			else {
				// set specific categories on
				for (i = 0; i < nCategories; i++) {
					fmi2Boolean categoryFound = fmi2False;
					for (j = 0; j < NUMBER_OF_CATEGORIES; j++) {
						if (strcmp(logCategoriesNames[j], categories[i]) == 0) {
							comp->logCategories[j] = loggingOn;
							categoryFound = fmi2True;
							break;
						}
					}
					if (!categoryFound) {
						comp->functions->logger(comp->componentEnvironment, inst.instanceName.c_str(), fmi2Warning,
							logCategoriesNames[LOG_ERROR],
							"logging category '%s' is not supported by model", categories[i]);
					}
				}
			}

			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetDebugLogging");
			return fmi2OK;
		}

		fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2GetReal", MASK_fmi2GetReal))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2GetReal", "vr[]", vr))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2GetReal", "value[]", value))
				return fmi2Error;
	
			std::vector<double> values;
			client->getReal(values, inst, std::vector<int32_t>(vr, vr + nvr));
			if (values.size() < nvr)
				return fmi2Error;
			for (size_t i = 0; i < nvr; i++) {
				value[i] = values[i];
				FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2GetReal: #r%u# = %.16g", vr[i], value[i]);
			}
			
			return fmi2OK;
		}

		fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2GetInteger", MASK_fmi2GetInteger))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2GetInteger", "vr[]", vr))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2GetInteger", "value[]", value))
				return fmi2Error;
						
			std::vector<int32_t> values;
			client->getInteger(values, inst, std::vector<int32_t>(vr, vr + nvr));
			if (values.size() < nvr)
				return fmi2Error;
			for (size_t i = 0; i < nvr; i++) {
				value[i] = values[i];
				FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2GetInteger: #i%u# = %d", vr[i], value[i]);
			}
			return fmi2OK;
		}

		fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2GetBoolean", MASK_fmi2GetBoolean))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2GetBoolean", "vr[]", vr))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2GetBoolean", "value[]", value))
				return fmi2Error;

			std::vector<bool> values;
			client->getBoolean(values, inst, std::vector<int32_t>(vr, vr + nvr));
			if (values.size() < nvr)
				return fmi2Error;
			for (size_t i = 0; i < nvr; i++) {
				value[i] = values[i];
				FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2GetBoolean: #b%u# = %s", vr[i], value[i] ? "true" : "false");
			}
			return fmi2OK;
		}

		fmi2Status fmi2GetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String value[]) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2GetString", MASK_fmi2GetString))
				return fmi2Error;
			if (nvr>0 && nullPointer(&inst, "fmi2GetString", "vr[]", vr))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2GetString", "value[]", value))
				return fmi2Error;			


			std::vector<std::string> values;
			client->getString(values, inst, std::vector<int32_t>(vr, vr + nvr));
			if (values.size() < nvr)
				return fmi2Error;
			
			std::map<int, char*>& valueRefStringBuffer = valueRefStringBufferByInstance[c];

			for (size_t i = 0; i < nvr; i++) {
				if (valueRefStringBuffer[vr[i]] != NULL){
					((ModelInstance*)c)->functions->freeMemory(valueRefStringBuffer[vr[i]]);
				}
				valueRefStringBuffer[vr[i]] = (char*)((ModelInstance*)c)->functions->allocateMemory(1 + values[i].size(), sizeof(char));
				strcpy((char *)valueRefStringBuffer[vr[i]], (char *)values[i].c_str());
				value[i] = valueRefStringBuffer[vr[i]];

				FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2GetString: #s%u# = '%s'", vr[i], value[i]);
			}
			return fmi2OK;
		}

		fmi2Status fmi2SetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[]) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2SetReal", MASK_fmi2SetReal))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2SetReal", "vr[]", vr))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2SetReal", "value[]", value))
				return fmi2Error;
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetReal: nvr = %d", nvr);
			std::map<int32_t, double> reals;

			for (size_t i = 0; i < nvr; i++) {
				FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetReal: #r%d# = %.16g", vr[i], value[i]);
				reals.insert(std::pair<int32_t, double>(vr[i], value[i]));
			}

			return static_cast<fmi2Status>(client->setReal(inst, reals));
		}

		fmi2Status fmi2SetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2SetInteger", MASK_fmi2SetInteger))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2SetInteger", "vr[]", vr))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2SetInteger", "value[]", value))
				return fmi2Error;
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetInteger: nvr = %d", nvr);

			std::map<int32_t, int32_t> integers;

			for (size_t i = 0; i < nvr; i++) {
				FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetInteger: #i%d# = %d", vr[i], value[i]);
					integers.insert(std::pair<int32_t, int32_t>(vr[i], value[i]));
			}

			return static_cast<fmi2Status>(client->setInteger(inst, integers));
	
		}

		fmi2Status fmi2SetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2SetBoolean", MASK_fmi2SetBoolean))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2SetBoolean", "vr[]", vr))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2SetBoolean", "value[]", value))
				return fmi2Error;
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetBoolean: nvr = %d", nvr);
				
			std::map<int32_t, bool> bools;

			for (size_t i = 0; i < nvr; i++) {
				FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetBoolean: #b%d# = %s", vr[i], value[i] ? "true" : "false");
				bools.insert(std::pair<int32_t, bool>(vr[i], value[i]>0));
			}

			return static_cast<fmi2Status>(client->setBoolean(inst, bools));
		}

		fmi2Status fmi2SetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String value[]) {			
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2SetString", MASK_fmi2SetString))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2SetString", "vr[]", vr))
				return fmi2Error;
			if (nvr > 0 && nullPointer(&inst, "fmi2SetString", "value[]", value))
				return fmi2Error;
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetString: nvr = %d", nvr);

				std::map<int32_t, std::string> strings;

			for (size_t i = 0; i < nvr; i++) {
				FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetString: #s%d# = '%s'", vr[i], value[i]);
				strings.insert(std::pair<int32_t, std::string>(vr[i], std::string(value[i])));
			}

			return static_cast<fmi2Status>(client->setString(inst, strings));
		}

		fmi2Status fmi2GetFMUstate(fmi2Component c, fmi2FMUstate* FMUstate) {
			fmi::Instance& inst = instances[c];
			return unsupportedFunction(&inst, "fmi2GetFMUstate", MASK_fmi2GetFMUstate);
		}
		fmi2Status fmi2SetFMUstate(fmi2Component c, fmi2FMUstate FMUstate) {
			fmi::Instance& inst = instances[c];
			return unsupportedFunction(&inst, "fmi2SetFMUstate", MASK_fmi2SetFMUstate);
		}
		fmi2Status fmi2FreeFMUstate(fmi2Component c, fmi2FMUstate* FMUstate) {
			fmi::Instance& inst = instances[c];
			return unsupportedFunction(&inst, "fmi2FreeFMUstate", MASK_fmi2FreeFMUstate);
		}
		fmi2Status fmi2SerializedFMUstateSize(fmi2Component c, fmi2FMUstate FMUstate, size_t *size) {
			fmi::Instance& inst = instances[c];
			return unsupportedFunction(&inst, "fmi2SerializedFMUstateSize", MASK_fmi2SerializedFMUstateSize);
		}
		fmi2Status fmi2SerializeFMUstate(fmi2Component c, fmi2FMUstate FMUstate, fmi2Byte serializedState[], size_t size) {
			fmi::Instance& inst = instances[c];
			return unsupportedFunction(&inst, "fmi2SerializeFMUstate", MASK_fmi2SerializeFMUstate);
		}
		fmi2Status fmi2DeSerializeFMUstate(fmi2Component c, const fmi2Byte serializedState[], size_t size,
			fmi2FMUstate* FMUstate) {
			fmi::Instance& inst = instances[c];
			return unsupportedFunction(&inst, "fmi2DeSerializeFMUstate", MASK_fmi2DeSerializeFMUstate);
		}

		fmi2Status fmi2GetDirectionalDerivative(fmi2Component c, const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
			const fmi2ValueReference vKnown_ref[], size_t nKnown,
			const fmi2Real dvKnown[], fmi2Real dvUnknown[]) {
			fmi::Instance& inst = instances[c];
			return unsupportedFunction(&inst, "fmi2GetDirectionalDerivative", MASK_fmi2GetDirectionalDerivative);
		}

		// ---------------------------------------------------------------------------
		// Functions for FMI for Co-Simulation
		// ---------------------------------------------------------------------------

		/* Simulating the slave */
		fmi2Status fmi2SetRealInputDerivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
			const fmi2Integer order[], const fmi2Real value[]) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2SetRealInputDerivatives", MASK_fmi2SetRealInputDerivatives)) {
				return fmi2Error;
			}
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2SetRealInputDerivatives: nvr= %d", nvr);
			FILTERED_LOG(inst, fmi2Error, LOG_ERROR, "fmi2SetRealInputDerivatives: ignoring function call."
				" This model cannot interpolate inputs: canInterpolateInputs=\"fmi2False\"");
			return fmi2Error;
		}

		fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
			const fmi2Integer order[], fmi2Real value[]) {
			unsigned int i;
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2GetRealOutputDerivatives", MASK_fmi2GetRealOutputDerivatives))
				return fmi2Error;
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2GetRealOutputDerivatives: nvr= %d", nvr);
			FILTERED_LOG(inst, fmi2Error, LOG_ERROR, "fmi2GetRealOutputDerivatives: ignoring function call."
				" This model cannot compute derivatives of outputs: MaxOutputDerivativeOrder=\"0\"");
				for (i = 0; i < nvr; i++) value[i] = 0;
			return fmi2Error;
		}

		fmi2Status fmi2CancelStep(fmi2Component c) {
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, "fmi2CancelStep", MASK_fmi2CancelStep)) {
				// always fmi2CancelStep is invalid, because model is never in modelStepInProgress state.
				return fmi2Error;
			}
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2CancelStep");
			FILTERED_LOG(inst, fmi2Error, LOG_ERROR, "fmi2CancelStep: Can be called when fmi2DoStep returned fmi2Pending."
				" This is not the case.");
			return fmi2Error;
		}

	

		

		fmi2Status fmi2DoStep(fmi2Component c, fmi2Real currentCommunicationPoint,
			fmi2Real communicationStepSize, fmi2Boolean noSetFMUStatePriorToCurrentPoint) {
			fmi::Instance& inst = instances[c];
			
			if (invalidState(&inst, "fmi2DoStep", MASK_fmi2DoStep))
				return fmi2Error;

			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "fmi2DoStep: "
				"currentCommunicationPoint = %g, "
				"communicationStepSize = %g, "
				"noSetFMUStatePriorToCurrentPoint = fmi2%s",
				currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPoint ? "True" : "False");
			
			if (!noSetFMUStatePriorToCurrentPoint && communicationStepSize <= 0) {
				FILTERED_LOG(inst, fmi2Error, LOG_ERROR,
					"fmi2DoStep: communication step size must be > 0. Fount %g.", communicationStepSize);				
				return fmi2Error;
			}


			return static_cast<fmi2Status>(client->doStep(inst, currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPoint > 0));

		}

		/* Inquire slave status */
		static fmi2Status getStatus(char* fname, fmi2Component c, const fmi2StatusKind s) {
			const char *statusKind[3] = { "fmi2DoStepStatus", "fmi2PendingStatus", "fmi2LastSuccessfulTime" };
			fmi::Instance& inst = instances[c];
			if (invalidState(&inst, fname, MASK_fmi2GetStatus)) // all get status have the same MASK_fmi2GetStatus
				return fmi2Error;
			FILTERED_LOG(inst, fmi2OK, LOG_FMI_CALL, "$s: fmi2StatusKind = %s", fname, statusKind[s]);

				switch (s) {
				case fmi2DoStepStatus: FILTERED_LOG(inst, fmi2Error, LOG_ERROR,
					"%s: Can be called with fmi2DoStepStatus when fmi2DoStep returned fmi2Pending."
					" This is not the case.", fname);
					break;
				case fmi2PendingStatus: FILTERED_LOG(inst, fmi2Error, LOG_ERROR,
					"%s: Can be called with fmi2PendingStatus when fmi2DoStep returned fmi2Pending."
					" This is not the case.", fname);
					break;
				case fmi2LastSuccessfulTime: FILTERED_LOG(inst, fmi2Error, LOG_ERROR,
					"%s: Can be called with fmi2LastSuccessfulTime when fmi2DoStep returned fmi2Discard."
					" This is not the case.", fname);
					break;
				case fmi2Terminated: FILTERED_LOG(inst, fmi2Error, LOG_ERROR,
					"%s: Can be called with fmi2Terminated when fmi2DoStep returned fmi2Discard."
					" This is not the case.", fname);
					break;
			}
			return fmi2Discard;
		}

		fmi2Status fmi2GetStatus(fmi2Component c, const fmi2StatusKind s, fmi2Status *value) {
			fmi::Instance& inst = instances[c];
			fmi2Status status = static_cast<fmi2Status>(client->getStatus(inst, static_cast<org::raspinloop::fmi::StatusKind::type>(s)));
			*value = status;
			return fmi2OK;

		}

		fmi2Status fmi2GetRealStatus(fmi2Component c, const fmi2StatusKind s, fmi2Real *value) {
			ModelInstance *comp = static_cast<ModelInstance*>(c);
			fmi::Instance& inst = instances[comp];
			fmi2Real status = static_cast<fmi2Real>(client->getRealStatus(inst, static_cast<org::raspinloop::fmi::StatusKind::type>(s)));
			*value = status;
			return fmi2OK;


		}

		fmi2Status fmi2GetIntegerStatus(fmi2Component c, const fmi2StatusKind s, fmi2Integer *value) {
			fmi::Instance& inst = instances[c];
			fmi2Integer status = static_cast<fmi2Integer>(client->getIntegerStatus(inst, static_cast<org::raspinloop::fmi::StatusKind::type>(s)));
			*value = status;
			return fmi2OK;

		}

		fmi2Status fmi2GetBooleanStatus(fmi2Component c, const fmi2StatusKind s, fmi2Boolean *value) {
			fmi::Instance& inst = instances[c];
			fmi2Boolean status = static_cast<fmi2Boolean>(client->getBooleanStatus(inst, static_cast<org::raspinloop::fmi::StatusKind::type>(s)));
			*value = status;
			return fmi2OK;					
		}

		fmi2Status fmi2GetStringStatus(fmi2Component c, const fmi2StatusKind s, fmi2String *value) {
			fmi::Instance& inst = instances[c];
			std::string status;
			client->getStringStatus(status, inst, static_cast<org::raspinloop::fmi::StatusKind::type>(s));
			if (!status.empty()){
				*value = status.c_str(); // WARNING char buffer won't live beyond this scope !!
				return fmi2OK;
			}
			return fmi2Discard;
		}

