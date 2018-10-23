#include "Simulation.h"
#include "IntersectAABB_shared.h"
#include "Worker.h"
#include "Geometry_shared.h"
#include <thread>
#include "Random.h"
#include "GLApp/MathTools.h"

SubProcessSuperStructure::~SubProcessSuperStructure()
{
	SAFE_DELETE(aabbTree);
}

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

size_t Simulation::GetMyState() {
	return prState;
}

void Simulation::SetLocalAndMasterState(size_t state, const std::string& status, bool changeState, bool changeStatus) {

	prState = state;
	if (LockMutex(worker->workerControl.mutex)) {
		if (changeState) worker->workerControl.states[prIdx] = state;
		if (changeStatus) worker->workerControl.statusStr[prIdx] = status;
		ReleaseMutex(worker->workerControl.mutex);
	}
}

void Simulation::SetErrorSub(const std::string& message) {
	
	SetLocalAndMasterState(PROCESS_ERROR, "Error: " + message + "\n");

}

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

void Simulation::SetReady() {

	if (loadOK)
		SetLocalAndMasterState(PROCESS_READY, GetMyStatusAsText());
	else
		SetLocalAndMasterState(PROCESS_READY, "(No geometry)");

}

void Simulation::SetStatusStringAtMaster(const std::string& status ) {

	if (LockMutex(worker->workerControl.mutex)) {
		worker->workerControl.statusStr[prIdx] = status;
		ReleaseMutex(worker->workerControl.mutex);
	}

}

void Simulation::ResizeTmpLog() {
	LockMutex(worker->logMutex);
	myLogTarget = std::max((myOtfp.enableLogging ? myOtfp.logLimit : 0) - worker->log.size(),(size_t)0) / myOtfp.nbProcess;
	ReleaseMutex(worker->logMutex);
	tmpParticleLog.clear();tmpParticleLog.shrink_to_fit(); tmpParticleLog.reserve(myLogTarget);
}


int Simulation::mainLoop(int index)
{
	bool eos = false;
	prIdx = index;

	//InitSimulation(); //Creates sHandle instance

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



void Simulation::RegisterTransparentPass(SubprocessFacet* f)
{
	double directionFactor = std::abs(Dot(currentParticle.direction, f->facetRef->sh.N));
	IncreaseFacetCounter(f, currentParticle.flightTime + f->colDist / 100.0 / currentParticle.velocity, 1, 0, 0, 2.0 / (currentParticle.velocity*directionFactor), 2.0*(worker->wp.useMaxwellDistribution ? 1.0 : 1.1781)*currentParticle.velocity*directionFactor);

	f->hitted = true;
	if (/*f->texture &&*/ f->facetRef->sh.countTrans) {
		RecordHitOnTexture(f, currentParticle.flightTime + f->colDist / 100.0 / currentParticle.velocity,
			true, 2.0, 2.0);
	}
	if (/*f->direction &&*/ f->facetRef->sh.countDirection) {
		RecordDirectionVector(f, currentParticle.flightTime + f->colDist / 100.0 / currentParticle.velocity);
	}
	LogHit(f);
	ProfileFacet(f, currentParticle.flightTime + f->colDist / 100.0 / currentParticle.velocity,
		true, 2.0, 2.0);
	if (f->facetRef->sh.anglemapParams.record) RecordAngleMap(f);
}

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
	return loadOK = true;
}

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

bool Simulation::StartSimulation() {
	StartFromSource();
	return currentParticle.lastHitFacet != NULL;
}

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