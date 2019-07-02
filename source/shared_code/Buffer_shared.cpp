#include "Buffer_shared.h"
#include "GLApp/MathTools.h"
#include "Worker.h"
#include "MolflowGeometry.h"
#include "Facet_shared.h"

/**
* \brief Assign operator
* \param src reference to source object
* \return address of this
*/
GlobalSimuState& GlobalSimuState::operator=(const GlobalSimuState & src) {
	//Copy all but mutex
	facetStates = src.facetStates;
	globalHistograms = src.globalHistograms;
	globalHits = src.globalHits;
	initialized = src.initialized;
	return *this;
}

/**
* \brief Clears simulation state
*/
void GlobalSimuState::clear() {
	LockMutex(mutex);
	globalHits = GlobalHitBuffer();
	globalHistograms.clear();
	facetStates.clear();
	ReleaseMutex(mutex);
}

/**
* \brief Constructs the 'dpHit' structure to hold all results, zero-init
* \param w Worker handle
*/
void GlobalSimuState::Resize(Worker& w) { //Constructs the 'dpHit' structure to hold all results, zero-init
	LockMutex(mutex);
	size_t nbF = w.GetGeometry()->GetNbFacet();
	std::vector<FacetState>(nbF).swap(facetStates);
	for (size_t i = 0; i < nbF; i++) {
		Facet* f = w.GetMolflowGeometry()->GetFacet(i);
		FacetMomentSnapshot facetMomentTemplate;
		facetMomentTemplate.histogram.Resize(f->sh.facetHistogramParams);
		facetMomentTemplate.direction = std::vector<DirectionCell>(f->sh.countDirection ? f->sh.texWidth*f->sh.texHeight : 0);
		facetMomentTemplate.profile = std::vector<ProfileSlice>(f->sh.isProfile ? PROFILE_SIZE : 0);
		facetMomentTemplate.texture = std::vector<TextureCell>(f->sh.isTextured ? f->sh.texWidth*f->sh.texHeight : 0);
		//No init for hits 
		facetStates[i].momentResults = std::vector<FacetMomentSnapshot>(1 + w.moments.size(), facetMomentTemplate);
		if (f->sh.anglemapParams.record) facetStates[i].recordedAngleMapPdf = std::vector<size_t>(f->sh.anglemapParams.GetMapSize());
	}
	//Global histogram
	FacetHistogramBuffer globalHistTemplate; globalHistTemplate.Resize(w.wp.globalHistogramParams);
	globalHistograms = std::vector<FacetHistogramBuffer>(1 + w.moments.size(), globalHistTemplate);
	initialized = true;
	ReleaseMutex(mutex);
}

/**
* \brief zero-init for all structures
*/
void GlobalSimuState::Reset() {
	LockMutex(mutex);
	for (auto& h : globalHistograms) {
		ZEROVECTOR(h.distanceHistogram);
		ZEROVECTOR(h.nbHitsHistogram);
		ZEROVECTOR(h.timeHistogram);
	}
	memset(&globalHits, 0, sizeof(globalHits)); //Plain old data
	for (auto& state : facetStates) {
		ZEROVECTOR(state.recordedAngleMapPdf);
		for (auto& m : state.momentResults) {
			ZEROVECTOR(m.histogram.distanceHistogram);
			ZEROVECTOR(m.histogram.nbHitsHistogram);
			ZEROVECTOR(m.histogram.timeHistogram);
			std::vector<DirectionCell>(m.direction.size()).swap(m.direction);
			std::vector<TextureCell>(m.texture.size()).swap(m.texture);
			std::vector<ProfileSlice>(m.profile.size()).swap(m.profile);
			memset(&(m.hits), 0, sizeof(m.hits));
		}
	}
	ReleaseMutex(mutex);
}

/**
* \brief += operator, with simple += of underlying structures
* \param rhs reference object on the right hand
* \return address of this (lhs)
*/
FacetHitBuffer& FacetHitBuffer::operator+=(const FacetHitBuffer& rhs) {
	this->nbDesorbed+=rhs.nbDesorbed;
	 this->nbMCHit+=rhs.nbMCHit;
	 this->nbHitEquiv+=rhs.nbHitEquiv;
	 this->nbAbsEquiv+=rhs.nbAbsEquiv;
	 this->sum_1_per_ort_velocity+=rhs.sum_1_per_ort_velocity;
	 this->sum_1_per_velocity+=rhs.sum_1_per_velocity;
	 this->sum_v_ort+=rhs.sum_v_ort;
	 return *this;
}

/**
* \brief Resize histograms according to sizes in params
* \param params contains data about sizes
*/
void FacetHistogramBuffer::Resize(const HistogramParams& params) {
	nbHitsHistogram = std::vector<double>(params.recordBounce ? params.GetBounceHistogramSize() : 0);
	distanceHistogram = std::vector<double>(params.recordDistance ? params.GetDistanceHistogramSize() : 0);
	timeHistogram = std::vector<double>(params.recordTime ? params.GetTimeHistogramSize() : 0);
}

/**
* \brief += operator, with simple += of underlying structures
* \param rhs reference object on the right hand
* \return address of this (lhs)
*/
FacetHistogramBuffer& FacetHistogramBuffer::operator+=(const FacetHistogramBuffer & rhs) {
	this->nbHitsHistogram += rhs.nbHitsHistogram;
	this->distanceHistogram += rhs.distanceHistogram;
	this->timeHistogram += rhs.timeHistogram;
	return *this;
}

/**
* \brief += operator, with simple += of underlying structures
* \param rhs reference object on the right hand
* \return address of this (lhs)
*/
FacetMomentSnapshot& FacetMomentSnapshot::operator+=(const FacetMomentSnapshot & rhs) {
	this->hits += rhs.hits;
	this->profile += rhs.profile;
	this->texture += rhs.texture;
	this->direction += rhs.direction;
	this->histogram += rhs.histogram;
	return *this;
}

/**
* \brief + operator, simply calls implemented +=
* \param rhs reference object on the right hand
* \return address of this (lhs)
*/
FacetMomentSnapshot& FacetMomentSnapshot::operator+(const FacetMomentSnapshot & rhs) {
	*this += rhs;
	return *this;
}

/**
* \brief += operator, with simple += of underlying structures
* \param rhs reference object on the right hand
* \return address of this (lhs)
*/
FacetState& FacetState::operator+=(const FacetState & rhs) {
	this->recordedAngleMapPdf += rhs.recordedAngleMapPdf;
	this->momentResults += rhs.momentResults;
	return *this;
}
