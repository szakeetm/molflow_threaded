#include "Simulation.h"
#include "IntersectAABB_shared.h"
#include "Worker.h"
#include "Geometry_shared.h"
#include <thread>
#include "Random.h"
#include "GLApp/MathTools.h"

/**
* \brief Destructor for the SubProcessSuperStructure class deleting the ay tracing tree (aabbTree)
*/
SubProcessSuperStructure::~SubProcessSuperStructure()
{
	SAFE_DELETE(aabbTree);
}

/**
* \brief Copies various states/parameters from worker to the simulation
*/
void Simulation::CopyMyStateFromControl() {
	prState = PROCESS_READY;
	prParam = 0;

	if (LockMutex(worker->workerControl.mutex)) {
		prState = worker->workerControl.states[prIdx];
		prParam = worker->workerControl.cmdParam[prIdx];
		prParam2 = worker->workerControl.cmdParam2[prIdx];
		worker->workerControl.cmdParam[prIdx] = 0;
		worker->workerControl.cmdParam2[prIdx] = 0;

		ReleaseMutex(worker->workerControl.mutex);

	}
	else {
		printf("Subprocess %d couldn't connect to Molflow.\n",prIdx);
		SetErrorSub("No connection to main program. Closing subprocess.");
		std::this_thread::sleep_for(std::chrono::seconds(5));
		end = true;
	}
}

/**
* \brief Getter that returns the process state
* \return process state
*/
size_t Simulation::GetMyState() {
	return prState;
}

/**
* \brief Sets local and master (worker) state (and status)
* \param state new state
* \param status new status
* \param changeState if state should be changed for the worker
* \param changeStatus if status should be changed for the worker
*/

void Simulation::SetLocalAndMasterState(size_t state, const std::string& status, bool changeState, bool changeStatus) {

	prState = state;
	if (LockMutex(worker->workerControl.mutex)) {
		if (changeState) worker->workerControl.states[prIdx] = state;
		if (changeStatus) worker->workerControl.statusStr[prIdx] = status;
		ReleaseMutex(worker->workerControl.mutex);
	}
}

/**
* \brief Sets error state for the simulation with error message
* \param message error message
*/
void Simulation::SetErrorSub(const std::string& message) {
	
	SetLocalAndMasterState(PROCESS_ERROR, "Error: " + message + "\n");

}

/**
* \brief Returns a string that contains various information about the simulation status
* \return string of status
*/
std::string Simulation::GetMyStatusAsText() {

	char ret[1024];
	size_t count = totalDesorbed;
	size_t max = myOtfp.desorptionLimit / myOtfp.nbProcess;

		if (max != 0) {
			double percent = (double)(count)*100.0 / (double)(max);
			sprintf(ret, "(%s) MC %zd/%zd (%.1f%%)", worker->GetGeometry()->GetName().c_str(), count, max, percent);
		}
		else {
			sprintf(ret, "(%s) MC %zd", worker->GetGeometry()->GetName().c_str(), count);
		}

	return ret;

}

/**
* \brief Sets simulation state to ready
*/
void Simulation::SetReady() {

	if (loadOK)
		SetLocalAndMasterState(PROCESS_READY, GetMyStatusAsText());
	else
		SetLocalAndMasterState(PROCESS_READY, "(No geometry)");

}

/**
* \brief Sets status from a string at the master worker
* \param status string containing new status
*/
void Simulation::SetStatusStringAtMaster(const std::string& status ) {

	if (LockMutex(worker->workerControl.mutex)) {
		worker->workerControl.statusStr[prIdx] = status;
		ReleaseMutex(worker->workerControl.mutex);
	}

}

/**
* \brief Resizes the temporary particle log (log since last update)
*/
void Simulation::ResizeTmpLog() {
	LockMutex(worker->logMutex);
	myLogTarget = std::max((myOtfp.enableLogging ? myOtfp.logLimit : 0) - worker->log.size(),(size_t)0) / myOtfp.nbProcess;
	ReleaseMutex(worker->logMutex);
	tmpParticleLog.clear();tmpParticleLog.shrink_to_fit(); tmpParticleLog.reserve(myLogTarget);
}

/**
* \brief Constructs facet temp vars for the intersect routine (ray tracing)
*/
void Simulation::ConstructFacetTmpVars() {
	std::vector<SubProcessFacetTempVar>(worker->GetGeometry()->GetNbFacet()).swap(myTmpFacetVars);
}

/**
* \brief Main loop of the simulation, listens to various state changes
* \param index process index
* \return status if successful
*/
int Simulation::mainLoop(int index) {
	bool eos = false;
	prIdx = index;

	//InitSimulation(); //Creates sHandle instance
	randomGenerator.GetSeed(); //By this point this is a unique thread with its own id

	// Sub process ready
	SetReady();

	// Main loop
	while (!end) {
		CopyMyStateFromControl();
		switch (prState) {

		case COMMAND_LOAD:
			//printf("COMMAND: LOAD (%zd,%llu)\n", prParam, prParam2);
			LoadSimulation();
			if (loadOK) {
				//sHandle->desorptionLimit = prParam2; // 0 for endless
				SetReady();
			}
			break;

		case COMMAND_UPDATEPARAMS:
			//printf("COMMAND: UPDATEPARAMS (%zd,%I64d)\n", prParam, prParam2);
			myOtfp = worker->ontheflyParams;
			ResizeTmpLog();
			SetLocalAndMasterState(prParam, GetMyStatusAsText());
			break;

		/*
		case COMMAND_RELEASEDPLOG:
			printf("COMMAND: UPDATEPARAMS (%zd,%I64d)\n", prParam, prParam2);
			CLOSEDP(dpLog);
			SetLocalAndMasterState(prParam, GetMyStatusAsText());
			break;
			*/

		case COMMAND_START:
			//printf("COMMAND: START (%zd,%llu)\n", prParam, prParam2);
			if (loadOK) {
				if (StartSimulation())
					SetLocalAndMasterState(PROCESS_RUN, GetMyStatusAsText());
				else {
					if (GetMyState() != PROCESS_ERROR)
						SetLocalAndMasterState(PROCESS_DONE, GetMyStatusAsText());
				}
			}
			else
				SetErrorSub("No geometry loaded");
			break;

		case COMMAND_PAUSE:
			//printf("COMMAND: PAUSE (%zd,%llu)\n", prParam, prParam2);
			if (!lastHitUpdateOK) {
				// Last update not successful, retry with a longer timeout
				if (GetMyState() != PROCESS_ERROR) {
					UpdateMCHits(30000);
					UpdateLog(30000);
				}
			}
			SetReady();
			break;

		case COMMAND_RESET:
			//printf("COMMAND: RESET (%zd,%llu)\n", prParam, prParam2);
			ResetSimulation();
			SetReady();
			break;

		case COMMAND_EXIT:
			//printf("COMMAND: EXIT (%zd,%llu)\n", prParam, prParam2);
			end = true;
			break;

		case COMMAND_CLOSE:
			//printf("COMMAND: CLOSE (%zd,%llu)\n", prParam, prParam2);
			ClearSimulation();
			SetReady();
			break;

		case PROCESS_RUN:
			SetStatusStringAtMaster(GetMyStatusAsText()); //update hits only
			eos = SimulationRun();      // Run during 1 sec
			if (GetMyState() != PROCESS_ERROR) {
				UpdateMCHits(20); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
				UpdateLog(20);
			}
			if (eos) {
				if (GetMyState() != PROCESS_ERROR) {
					// Max desorption reached
					SetLocalAndMasterState(PROCESS_DONE, GetMyStatusAsText());
					printf("COMMAND: PROCESS_DONE (Max reached)\n");
				}
			}
			break;

		default:
			std::this_thread::sleep_for(std::chrono::milliseconds(WAITTIME));
			break;
		}
	}

	// Release
	SetLocalAndMasterState(PROCESS_KILLED, "");
	return 0;
}

/**
* \brief Puts various variables to default state to 'clear' the simulation
*/
void Simulation::ClearSimulation() {
	//Put everything to default state
	//Even better would be to end thread and launch again
	end = loadOK =  false;
	//tmpParticleLog.clear(); tmpParticleLog.shrink_to_fit(); //Will be reinitialized on LoadSimulation()
	//myTmpResults.clear(); //Will be reinitialized on LoadSimulation()
	totalDesorbed = 0;
	//structures.clear(); structures.shrink_to_fit(); //Will be reinitialized on LoadSimulation()
	//currentParticle = CurrentParticleStatus(); //Will be reinitialized on StartFromSource()
}


/**
* \brief Function to register a transparent pass (counter facets that logs passing particles)
* \param f subprocess facet
*/
void Simulation::RegisterTransparentPass(SubprocessFacet* f)
{
	double directionFactor = std::abs(Dot(currentParticle.direction, f->facetRef->sh.N));
	IncreaseFacetCounter(f, currentParticle.flightTime + myTmpFacetVars[f->globalId].colDistTranspPass / 100.0 / currentParticle.velocity, 1, 0, 0, 2.0 / (currentParticle.velocity*directionFactor), 2.0*(worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*currentParticle.velocity*directionFactor);

	myTmpFacetVars[f->globalId].hitted = true;
	if (/*f->texture &&*/ f->facetRef->sh.countTrans) {
		RecordHitOnTexture(f, currentParticle.flightTime + myTmpFacetVars[f->globalId].colDistTranspPass / 100.0 / currentParticle.velocity,
			true, 2.0, 2.0);
	}
	if (/*f->direction &&*/ f->facetRef->sh.countDirection) {
		RecordDirectionVector(f, currentParticle.flightTime + myTmpFacetVars[f->globalId].colDistTranspPass / 100.0 / currentParticle.velocity);
	}
	LogHit(f);
	ProfileFacet(f, currentParticle.flightTime + myTmpFacetVars[f->globalId].colDistTranspPass / 100.0 / currentParticle.velocity,
		true, 2.0, 2.0);
	if (f->facetRef->sh.anglemapParams.record) RecordAngleMap(f);
}

/**
* \brief Function that clears the previous simulation and loads parameters and memory structure
* \return true if loading was successful
*/
bool Simulation::LoadSimulation() {
	loadOK = false;
	SetLocalAndMasterState(PROCESS_STARTING, "Clearing previous simulation");
	ClearSimulation();
	SetLocalAndMasterState(PROCESS_STARTING, "Loading worker params");
	myOtfp = worker->ontheflyParams;
	SetLocalAndMasterState(PROCESS_STARTING, "Loading results memory structure");
	myTmpResults = worker->emptyResultTemplate;
	SetLocalAndMasterState(PROCESS_STARTING, "Loading log memory structure");
	ResizeTmpLog();
	ConstructFacetTmpVars();
	return loadOK = true;
}

/**
* \brief Copies state and parameters from master (worker) into local thread
*/
void Simulation::CopyStateFromMaster() {
	prState = PROCESS_READY;
	prParam = 0;

	if (LockMutex(worker->workerControl.mutex)) {
		prState = worker->workerControl.states[prIdx];
		prParam = worker->workerControl.cmdParam[prIdx];
		prParam2 = worker->workerControl.cmdParam2[prIdx];
		worker->workerControl.cmdParam[prIdx] = 0;
		worker->workerControl.cmdParam2[prIdx] = 0;

		ReleaseMutex(worker->workerControl.mutex);
	}
	else {
		printf("Subprocess couldn't connect to Molflow.\n");
		SetErrorSub("No connection to main program. Closing subprocess.");
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		end = true;
	}
}

/**
* \brief Starts the simulation by launching a particle from source
* \return true if particle hit a facet
*/
bool Simulation::StartSimulation() {
	//StartFromSource();
	//return currentParticle.lastHitFacet != NULL;
	return StartFromSource();
}

/**
* \brief Starts a MC simulation step with time measurement
* \return true if particle will not be used anymore in the system (leak, desorption limit etc.)
*/
bool Simulation::SimulationRun() {
	int nbStep = 1;
	bool goOn;

	if (stepPerSec == 0.0) nbStep = 250;
	else nbStep = (int)(stepPerSec + 0.5);
	auto start_time = std::chrono::high_resolution_clock::now();
	goOn = SimulationMCStep(nbStep);
	auto end_time = std::chrono::high_resolution_clock::now();
	double elapsedTimeMs = std::chrono::duration<double, std::milli>(end_time - start_time).count();
	stepPerSec = ((double)nbStep / elapsedTimeMs)*1000.0;
	return !goOn;
}