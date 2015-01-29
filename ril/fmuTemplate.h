/* ---------------------------------------------------------------------------*
 * fmuTemplate.h
 * Definitions by the includer of this file
 * Copyright QTronic GmbH. All rights reserved.
 * ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "fmi2Functions.h"
#include "generated\ril_types.h"

//fmi2Status setString(fmi2Component comp, fmi2ValueReference vr, fmi2String value);

// categories of logging supported by model.
// Value is the index in logCategories of a ModelInstance.
const static unsigned int  LOG_ALL = 0;
const static unsigned int LOG_ERROR = 1;
const static unsigned int LOG_FMI_CALL = 2;
const static unsigned int LOG_EVENT = 3;

const unsigned int NUMBER_OF_CATEGORIES = 4;

// ---------------------------------------------------------------------------
// Function calls allowed state masks for both Model-exchange and Co-simulation
// ---------------------------------------------------------------------------
const static int MASK_fmi2GetTypesPlatform	= (org::raspinloop::fmi::ModelState::type::modelStartAndEnd | org::raspinloop::fmi::ModelState::type::modelInstantiated | org::raspinloop::fmi::ModelState::type::modelInitializationMode
	| org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode
	| org::raspinloop::fmi::ModelState::type::modelStepComplete | org::raspinloop::fmi::ModelState::type::modelStepInProgress | org::raspinloop::fmi::ModelState::type::modelStepFailed | org::raspinloop::fmi::ModelState::type::modelStepCanceled
	| org::raspinloop::fmi::ModelState::type::modelTerminated | org::raspinloop::fmi::ModelState::type::modelError);
const static int  MASK_fmi2GetVersion = MASK_fmi2GetTypesPlatform;
const static int  MASK_fmi2SetDebugLogging = (org::raspinloop::fmi::ModelState::type::modelInstantiated | org::raspinloop::fmi::ModelState::type::modelInitializationMode
	| org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode
	| org::raspinloop::fmi::ModelState::type::modelStepComplete | org::raspinloop::fmi::ModelState::type::modelStepInProgress | org::raspinloop::fmi::ModelState::type::modelStepFailed | org::raspinloop::fmi::ModelState::type::modelStepCanceled
	| org::raspinloop::fmi::ModelState::type::modelTerminated | org::raspinloop::fmi::ModelState::type::modelError);
const static int  MASK_fmi2Instantiate = (org::raspinloop::fmi::ModelState::type::modelStartAndEnd);
const static int  MASK_fmi2FreeInstance = (org::raspinloop::fmi::ModelState::type::modelInstantiated | org::raspinloop::fmi::ModelState::type::modelInitializationMode
	| org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode
	| org::raspinloop::fmi::ModelState::type::modelStepComplete | org::raspinloop::fmi::ModelState::type::modelStepFailed | org::raspinloop::fmi::ModelState::type::modelStepCanceled
	| org::raspinloop::fmi::ModelState::type::modelTerminated | org::raspinloop::fmi::ModelState::type::modelError);
const static int  MASK_fmi2SetupExperiment = org::raspinloop::fmi::ModelState::type::modelInstantiated;
const static int  MASK_fmi2EnterInitializationMode = org::raspinloop::fmi::ModelState::type::modelInstantiated;
const static int  MASK_fmi2ExitInitializationMode = org::raspinloop::fmi::ModelState::type::modelInitializationMode;
const static int  MASK_fmi2Terminate = (org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode
	| org::raspinloop::fmi::ModelState::type::modelStepComplete | org::raspinloop::fmi::ModelState::type::modelStepFailed);
const static int  MASK_fmi2Reset = MASK_fmi2FreeInstance;
const static int  MASK_fmi2GetReal = (org::raspinloop::fmi::ModelState::type::modelInitializationMode
	| org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode
	| org::raspinloop::fmi::ModelState::type::modelStepComplete | org::raspinloop::fmi::ModelState::type::modelStepFailed | org::raspinloop::fmi::ModelState::type::modelStepCanceled
	| org::raspinloop::fmi::ModelState::type::modelTerminated | org::raspinloop::fmi::ModelState::type::modelError);
const static int  MASK_fmi2GetInteger = MASK_fmi2GetReal;
const static int  MASK_fmi2GetBoolean = MASK_fmi2GetReal;
const static int  MASK_fmi2GetString = MASK_fmi2GetReal;
const static int  MASK_fmi2SetReal = (org::raspinloop::fmi::ModelState::type::modelInstantiated | org::raspinloop::fmi::ModelState::type::modelInitializationMode
	| org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode
	| org::raspinloop::fmi::ModelState::type::modelStepComplete);
const static int  MASK_fmi2SetInteger = (org::raspinloop::fmi::ModelState::type::modelInstantiated | org::raspinloop::fmi::ModelState::type::modelInitializationMode
	| org::raspinloop::fmi::ModelState::type::modelEventMode
	| org::raspinloop::fmi::ModelState::type::modelStepComplete);
const static int  MASK_fmi2SetBoolean = MASK_fmi2SetInteger;
const static int  MASK_fmi2SetString = MASK_fmi2SetInteger;
const static int  MASK_fmi2GetFMUstate = MASK_fmi2FreeInstance;
const static int  MASK_fmi2SetFMUstate = MASK_fmi2FreeInstance;
const static int  MASK_fmi2FreeFMUstate = MASK_fmi2FreeInstance;
const static int  MASK_fmi2SerializedFMUstateSize = MASK_fmi2FreeInstance;
const static int  MASK_fmi2SerializeFMUstate = MASK_fmi2FreeInstance;
const static int  MASK_fmi2DeSerializeFMUstate = MASK_fmi2FreeInstance;
const static int  MASK_fmi2GetDirectionalDerivative = (org::raspinloop::fmi::ModelState::type::modelInitializationMode
	| org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode
	| org::raspinloop::fmi::ModelState::type::modelStepComplete | org::raspinloop::fmi::ModelState::type::modelStepFailed | org::raspinloop::fmi::ModelState::type::modelStepCanceled
	| org::raspinloop::fmi::ModelState::type::modelTerminated | org::raspinloop::fmi::ModelState::type::modelError);

// ---------------------------------------------------------------------------
// Function calls allowed state masks for Model-exchange
// ---------------------------------------------------------------------------
const static int  MASK_fmi2EnterEventMode = (org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode);
const static int  MASK_fmi2NewDiscreteStates = org::raspinloop::fmi::ModelState::type::modelEventMode;
const static int  MASK_fmi2EnterContinuousTimeMode = org::raspinloop::fmi::ModelState::type::modelEventMode;
const static int  MASK_fmi2CompletedIntegratorStep = org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode;
const static int  MASK_fmi2SetTime = (org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode);
const static int  MASK_fmi2SetContinuousStates = org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode;
const static int  MASK_fmi2GetEventIndicators = (org::raspinloop::fmi::ModelState::type::modelInitializationMode
	| org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode
	| org::raspinloop::fmi::ModelState::type::modelTerminated | org::raspinloop::fmi::ModelState::type::modelError);
const static int  MASK_fmi2GetContinuousStates = MASK_fmi2GetEventIndicators;
const static int  MASK_fmi2GetDerivatives = (org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode
	| org::raspinloop::fmi::ModelState::type::modelTerminated | org::raspinloop::fmi::ModelState::type::modelError);
const static int  MASK_fmi2GetNominalsOfContinuousStates = (org::raspinloop::fmi::ModelState::type::modelInstantiated
	| org::raspinloop::fmi::ModelState::type::modelEventMode | org::raspinloop::fmi::ModelState::type::modelContinuousTimeMode
	| org::raspinloop::fmi::ModelState::type::modelTerminated | org::raspinloop::fmi::ModelState::type::modelError);

// ---------------------------------------------------------------------------
// Function calls allowed state masks for Co-simulation
// ---------------------------------------------------------------------------
const static int  MASK_fmi2SetRealInputDerivatives = (org::raspinloop::fmi::ModelState::type::modelInstantiated | org::raspinloop::fmi::ModelState::type::modelInitializationMode
	| org::raspinloop::fmi::ModelState::type::modelStepComplete);
const static int  MASK_fmi2GetRealOutputDerivatives = (org::raspinloop::fmi::ModelState::type::modelStepComplete | org::raspinloop::fmi::ModelState::type::modelStepFailed | org::raspinloop::fmi::ModelState::type::modelStepCanceled
	| org::raspinloop::fmi::ModelState::type::modelTerminated | org::raspinloop::fmi::ModelState::type::modelError);
const static int  MASK_fmi2DoStep = org::raspinloop::fmi::ModelState::type::modelStepComplete;
const static int  MASK_fmi2CancelStep = org::raspinloop::fmi::ModelState::type::modelStepInProgress;
const static int  MASK_fmi2GetStatus = (org::raspinloop::fmi::ModelState::type::modelStepComplete | org::raspinloop::fmi::ModelState::type::modelStepInProgress | org::raspinloop::fmi::ModelState::type::modelStepFailed
	| org::raspinloop::fmi::ModelState::type::modelTerminated);
const static int  MASK_fmi2GetRealStatus = MASK_fmi2GetStatus;
const static int  MASK_fmi2GetIntegerStatus = MASK_fmi2GetStatus;
const static int  MASK_fmi2GetBooleanStatus = MASK_fmi2GetStatus;
const static int  MASK_fmi2GetStringStatus = MASK_fmi2GetStatus;

typedef struct {
    const fmi2CallbackFunctions *functions;
    fmi2Boolean loggingOn;
    fmi2Boolean logCategories[NUMBER_OF_CATEGORIES];
    fmi2ComponentEnvironment componentEnvironment;   
} ModelInstance;
