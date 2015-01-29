/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */


/**
 * Thrift files can namespace, package, or prefix their output in various
 * target languages.
 */
namespace cpp org.raspinloop.fmi
namespace java org.raspinloop.fmi

enum ModelState {
    modelStartAndEnd        = 1,
    modelInstantiated       = 2,
    modelInitializationMode = 4,

    // ME states
    modelEventMode          = 8,
    modelContinuousTimeMode = 16,
    // CS states
    modelStepComplete       = 35,
    modelStepInProgress     = 64,
    modelStepFailed         = 128,
    modelStepCanceled       = 256,

    modelTerminated         = 512,
    modelError              = 1024,
    modelFatal              = 2048,
}

/* Type definitions */
enum Status{
    OK = 0,
    Warning = 1,
    Discard = 2,
    Error = 3,
    Fatal = 4,
    Pending = 5
}

enum Type{
    ModelExchange = 0,
    CoSimulation = 1
}

enum StatusKind{
    DoStepStatus =0,
    PendingStatus = 1,
    LastSuccessfulTime = 2,
    Terminated = 3
}

struct EventInfo{
   1: bool newDiscreteStatesNeeded;
   2: bool terminateSimulation;
   3: bool nominalsOfContinuousStatesChanged;
   4: bool valuesOfContinuousStatesChanged;
   5: bool nextEventTimeDefined;
   6: double    nextEventTime;
}

struct Instance {
    1: string instanceName;
    2: i64 componentRef;
    3: string GUID; 
    4: ModelState state;  
}

service CoSimulation {

	string getVersion(),
	
	string getTypesPlatform(),
	
	Instance instanciate(1:string instanceName, 2:Type fmuType, 3:string fmuGUID,
                            4:string fmuResourceLocation, 5:bool visible, 6:bool loggingOn),
  
    Status setupExperiment(1:Instance c, 2:bool toleranceDefined, 3:double tolerance,
                            4:double startTime, 5:bool stopTimeDefined, 6:double stopTime),
	
	Status enterInitializationMode(1:Instance c),	
							
	Status exitInitializationMode(1:Instance c),

	Status terminate(1:Instance c),
	
	Status reset(1:Instance c),

	void freeInstance(1:Instance c),

	list<double> getReal (1:Instance c, 2: list<i32> refs),
	
	list<i32> getInteger (1:Instance c, 2: list<i32> refs),
	
	list<bool> getBoolean (1:Instance c, 2: list<i32> refs),
	
	list<string> getString (1:Instance c, 2: list<i32> refs),
	
	
	Status setReal (1:Instance c, 2: map<i32, double> ref_values),
	
	Status setInteger (1:Instance c, 2: map<i32, i32> ref_values),
	
	Status setBoolean (1:Instance c, 2: map<i32, bool> ref_values),
	
	Status setString (1:Instance c, 2: map<i32, string> ref_values),

	Status setRealInputDerivatives(1:Instance c, 2:map<i32, i32> ref_orders, 3:map<i32, double> ref_values),
	
	Status setRealOutputDerivatives(1:Instance c, 2:map<i32, i32> ref_orders, 3:map<i32, double> ref_values),
	
	Status cancelStep(1:Instance c),
	
	Status doStep(1:Instance c, 2:double currentCommunicationPoint, 3:double communicationStepSize, 4:bool noSetFMUStatePriorToCurrentPoint),
	
	Status getStatus(1:Instance c, 2: StatusKind s),
	
	i32 getIntegerStatus(1:Instance c, 2: StatusKind s),
	
	double getRealStatus(1:Instance c, 2: StatusKind s),
	
	bool getBooleanStatus(1:Instance c, 2: StatusKind s),
	
	string getStringStatus(1:Instance c, 2: StatusKind s),
}
