/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2014, Texas A&M University
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Texas A&M University nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Authors: Saurav Agarwal, Ali-akbar Agha-mohammadi */

#ifndef CONTROLLER_
#define CONTROLLER_

#include "SeparatedControllers/SeparatedControllerMethod.h"
#include "Filters/KalmanFilterMethod.h"
#include "MotionModels/MotionModelMethod.h"
#include "ObservationModels/ObservationModelMethod.h"
#include "SpaceInformation/SpaceInformation.h"
#include "ompl/base/Cost.h"
#include "boost/date_time/local_time/local_time.hpp"
#include <boost/thread.hpp>

/** \brief Base class for Controller. A controller's task is to use the filter to estimate the belief robot's state and
          generate control commands using the separated controller. For example by fusing an LQR and Kalman Filter
          we generate an LQG controller. */
template <class SeparatedControllerType, class FilterType>
class Controller
{

    public:
        typedef MotionModelMethod::SpaceType SpaceType;
        typedef MotionModelMethod::StateType StateType;
        typedef firm::SpaceInformation::SpaceInformationPtr SpaceInformationPtr;
        typedef MotionModelMethod::ControlType   ControlType;
        typedef ObservationModelMethod::ObservationType ObservationType;
        typedef MotionModelMethod::MotionModelPointer MotionModelPointer;
        typedef ObservationModelMethod::ObservationModelPointer ObservationModelPointer;

        /** \brief Constructor */
        Controller() {};

        /** \brief Constructor */
        Controller(const ompl::base::State *goal,
                 const std::vector<ompl::base::State*>& nominalXs,
                 const std::vector<ompl::control::Control*>& nominalUs,
                 const firm::SpaceInformation::SpaceInformationPtr si);

        /** \brief Execute the controller i.e. take the system from start to end state of edge. The execution cost is the sum of the trace of covariance at each step.
                   The construction mode flag tells the controller to check true state validity. This is useful to detect collision during edge construction (a collision during
                   a monte carlo sim affects the transition probability of the edge).
        */
        virtual bool Execute(const ompl::base::State *startState,
                   ompl::base::State* endState,
                   ompl::base::Cost &filteringCost,
                   int &stepsTaken,
                   int &timeToStop,
                   bool constructionMode=true);

        /** \brief Execute the controller for one step */
         virtual bool executeOneStep(const int k, const ompl::base::State *startState,
                   ompl::base::State* endState,
                   ompl::base::Cost &filteringCost,
                   bool constructionMode=true);

        /** \brief Execute the controller for given number of steps */
         virtual bool executeUpto(const int numSteps, const ompl::base::State *startState,
                   ompl::base::State* endState,
                   ompl::base::Cost &filteringCost,
                   int &stepsTaken,
                   bool constructionMode=true);

        /** \brief Stabilize the system to an existing FIRM node */
        virtual void  Stabilize(const ompl::base::State *startState,
                                              ompl::base::State* endState,
                                              ompl::base::Cost &stabilizationFilteringCost,
                                              int &stepsToStabilize,
                                              bool constructionMode=true);

        /** \brief Check whether the controller has satisfied its termination condition for e.g. reached target state*/
        virtual bool isTerminated(const ompl::base::State *state, const size_t t);

        /** \brief Evolve the controller over a single time step, i.e. apply control, predict, get observation, update */
        virtual void Evolve(const ompl::base::State *state, size_t t, ompl::base::State* nextState);

        /** \brief get the controllers goal state */
        ompl::base::State* getGoal() {return goal_; }

        /** \brief Set the space information of the planning problem */
        void setSpaceInformation(SpaceInformationPtr si)
        { 
          si_.reset();
          si_ = si; 
        }

        /** \brief Set the nodeReached angle.*/
        static void setNodeReachedAngle(double angle) {nodeReachedAngle_ = angle; }

        /** \brief Set the distance at which we assume the robot has reached a target node.*/
        static void setNodeReachedDistance(double d) {nodeReachedDistance_ = d; }

        /** \brief The max number of attempts to align with node. */
        static void setMaxTries(double maxtries) {maxTries_ = maxtries; }

        /** \brief Set the maximum trajectory deviation before which to replan. */
        static void setMaxTrajectoryDeviation(double dev) {nominalTrajDeviationThreshold_ = dev; }

        /** \brief Return the number of linear systems. */
        size_t Length() { return lss_.size(); }

    private:

        /** \brief The pointer to the space information. */
        SpaceInformationPtr si_; // Instead of the actuation system, in OMPL we have the spaceinformation

        /** \brief  The vector of linear systems. The linear systems basically represent the system state
                    at a point in the open loop trajectory.*/
        std::vector<LinearSystem> lss_;

        /** \brief  The separated controller used to generate the commands that are sent to the robot. */
        SeparatedControllerType separatedController_;

        /** \brief  The filter used to estimate the robot belief. */
    	  FilterType filter_;

        /** \brief  The target node to which the controller drives the robot.*/
        ompl::base::State *goal_;

        /** \brief Tracks the current number of time steps the robot has executed to align with goal node. */
    	  int tries_;

        /** \brief If the robot's heading is deviated from the target heading by less
            than the nodeReachedAngle_ then the robot is assumed to have alligned with the target heading. Used
            for node reachability checking. */
    	  static double nodeReachedAngle_;

        /** \brief The distance at which we assume the robot has reached a target node (i.e. b \in B). Reaching the exact node
            location is almost impractical for stochastic systems. We assume the robot has reached if it is within
            a certain region around the target. */
        static double nodeReachedDistance_;

        /** \brief  The max number of tries to align with target node. */
      	static double maxTries_;

        /** \brief  The maximum deviation from the nominal trajectory beyond which the robot must replan.*/
        static double nominalTrajDeviationThreshold_;

        /** \brief  The maximum time for which a controller can be executed. We need this bound as we cannot let a controller
                    execute indefinitely. This avoids situations when the robot has deviated or collided and the current
                    controller is no longer capable of driving the robot to the goal.*/
        double maxExecTime_;

        /** \brief  The debug mode, if true, controller is verbose.*/
      	bool debug_;

      	//unsigned int step_;

};

template <class SeparatedControllerType, class FilterType>
double Controller<SeparatedControllerType, FilterType>::nodeReachedAngle_ = -1;

template <class SeparatedControllerType, class FilterType>
double Controller<SeparatedControllerType, FilterType>::nodeReachedDistance_ = -1;

template <class SeparatedControllerType, class FilterType>
double Controller<SeparatedControllerType, FilterType>::maxTries_ = -1;

template <class SeparatedControllerType, class FilterType>
double Controller<SeparatedControllerType, FilterType>::nominalTrajDeviationThreshold_ = -1;

template <class SeparatedControllerType, class FilterType>
Controller<SeparatedControllerType, FilterType>::Controller(const ompl::base::State *goal,
            const std::vector<ompl::base::State*>& nominalXs,
            const std::vector<ompl::control::Control*>& nominalUs,
            const firm::SpaceInformation::SpaceInformationPtr si): si_(si)
{

  goal_ = si_->allocState();

  si_->copyState(goal_, goal);

  lss_.reserve(nominalXs.size());

  for(size_t i=0; i<nominalXs.size(); ++i)
  {

    LinearSystem ls(si_, nominalXs[i], nominalUs[i], si_->getMotionModel(), si_->getObservationModel());

    lss_.push_back(ls);
  }

  //copy construct separated controller
  SeparatedControllerType sepController(goal_, nominalXs, nominalUs, lss_,si_->getMotionModel());

  separatedController_ = sepController;

  FilterType filter(si);
  filter_ = filter;

  tries_ = 0;

  //nominalXs is scaled by <3> to allow a bit more steps for robot to execute edge.
  //Otherwise may not get good performance
  maxExecTime_ = ceil(nominalXs.size()*3);

  debug_ = false;

  //step_ = 0;

}


template <class SeparatedControllerType, class FilterType>
bool Controller<SeparatedControllerType, FilterType>::Execute(const ompl::base::State *startState,
                                                              ompl::base::State* endState,
                                                              ompl::base::Cost &filteringCost,
                                                              int &stepsTaken,
                                                              int &timeToStop,
                                                              bool constructionMode)
{
    using namespace std;

    unsigned int k = 0;

    //HOW TO SET INITAL VALUE OF COST
    //cost = 1 ,for time based only if time per execution is "1"
    //cost = 0.01 , for covariance based
    double cost = 0.001;

    //float totalCollisionCheckComputeTime = 0;
    //int totalNumCollisionChecks = 0;

    ompl::base::State *internalState = si_->allocState();

    si_->copyState(internalState, startState);

    ompl::base::State  *nominalX_K ;

    ompl::base::State *tempEndState = si_->allocState();

    si_->copyState(tempEndState, startState);

    while(!this->isTerminated(tempEndState, k))
    {

        this->Evolve(internalState, k, tempEndState) ;

        si_->copyState(internalState, tempEndState);

        /** Check if the controller is in construction mode,
        If true, that means we are doing monte carlo sims,
        Then need to check if If the propagated state is not valid, to stop controller on account of collision.
        */
        if(constructionMode)
        {
            //totalNumCollisionChecks++;

            // start profiling time to compute rollout
            //auto start_time = std::chrono::high_resolution_clock::now();

            bool isThisStateValid = si_->checkTrueStateValidity();

            //auto end_time = std::chrono::high_resolution_clock::now();

            //totalCollisionCheckComputeTime += std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

            if(!isThisStateValid)
            {
                si_->copyState(endState, internalState);

                return false;
            }
        }

        if(k<lss_.size())
          nominalX_K = lss_[k].getX();

        else nominalX_K = lss_[lss_.size()-1].getX();

        arma::colvec nomXVec = nominalX_K->as<StateType>()->getArmaData();
        arma::colvec endStateVec =  internalState->as<StateType>()->getArmaData();
        arma::colvec deviation = nomXVec.subvec(0,1) - endStateVec.subvec(0,1);

        if(abs(norm(deviation,2)) > nominalTrajDeviationThreshold_ || !si_->checkTrueStateValidity())
        {
            si_->copyState(endState, internalState);

            return false;
        }

        k++;

        //Increment cost by: 0.01 for time based, trace(Covariance) for FIRM
        arma::mat tempCovMat = internalState->as<StateType>()->getCovariance();
        cost += arma::trace(tempCovMat);

        if(!constructionMode)
        {
          boost::this_thread::sleep(boost::posix_time::milliseconds(20));
        }
    }

//    if(constructionMode)
//    {
//        arma::colvec diff = startState->as<StateType>()->getArmaData() - goal_->as<StateType>()->getArmaData();
//
//        std::ofstream outfile;
//        outfile.open("/home/sauravagarwal/Dropbox/SLAP_Rollout_FIRM/DATA-Sims/TRO-Sims-Latest/CollisionChecks.csv",std::ios::app);
//        outfile<<arma::norm(diff.subvec(0,1),2)<<","
//                 <<totalNumCollisionChecks<<","
//                 <<totalCollisionCheckComputeTime/(1000000)
//                 <<std::endl;
//        outfile.close();
//    }

    ompl::base::Cost stabilizationFilteringCost(0);

    int stepsToStabilize=0;

    ompl::base::State *stabilizedState = si_->allocState();

    //this->Stabilize(internalState, stabilizedState, stabilizationFilteringCost, stepsToStabilize, constructionMode) ;

    //si_->copyState(endState, stabilizedState);

    si_->copyState(endState, internalState);
    
    //filteringCost.v = cost + stabilizationFilteringCost.v;
    filteringCost = ompl::base::Cost(cost + stabilizationFilteringCost.value());

    stepsTaken = k+stepsToStabilize;

    timeToStop = k;

    si_->freeState(internalState);

    si_->freeState(tempEndState);

    si_->freeState(stabilizedState);

    return true ;
}


template <class SeparatedControllerType, class FilterType>
bool Controller<SeparatedControllerType, FilterType>::executeOneStep(const int k, const ompl::base::State *startState,
                                                              ompl::base::State* endState,
                                                              ompl::base::Cost &filteringCost,
                                                              bool constructionMode)
{
    using namespace std;

    //HOW TO SET INITAL VALUE OF COST
    //cost = 1 ,for time based only if time per execution is "1"
    //cost = 0.01 , for covariance based
    double cost = 0.001;

    ompl::base::State *internalState = si_->allocState();
    si_->copyState(internalState, startState);

    ompl::base::State  *nominalX_K = si_->allocState();

    this->Evolve(internalState, k, endState) ;

    si_->copyState(internalState, endState);

    // if the propagated state is not valid, return false (only in construction mode)
    if(constructionMode)
    {
        if(!si_->checkTrueStateValidity())
        {
            return false;
        }
    }

    if(k<lss_.size())
      nominalX_K = lss_[k].getX();

    else nominalX_K = lss_[lss_.size()-1].getX();

    arma::colvec nomXVec = nominalX_K->as<StateType>()->getArmaData();
    arma::colvec endStateVec = endState->as<StateType>()->getArmaData();
    arma::colvec deviation = nomXVec.subvec(0,1) - endStateVec.subvec(0,1);

    if(abs(norm(deviation,2)) > nominalTrajDeviationThreshold_ || !si_->checkTrueStateValidity())
    {
      return false;
    }

    //Increment cost by:
    //-> 0.01 for time based
    //-> trace(Covariance) for FIRM
    arma::mat tempCovMat = endState->as<StateType>()->getCovariance();
    cost += arma::trace(tempCovMat);

    if(!constructionMode) boost::this_thread::sleep(boost::posix_time::milliseconds(20));

    //filteringCost.v = cost;
    filteringCost = ompl::base::Cost(cost);

    si_->freeState(internalState);

    return true ;
}


template <class SeparatedControllerType, class FilterType>
bool Controller<SeparatedControllerType, FilterType>::executeUpto(const int numSteps, const ompl::base::State *startState,
                                                              ompl::base::State* endState,
                                                              ompl::base::Cost &filteringCost,
                                                              int &stepsTaken,
                                                              bool constructionMode)
{
    ompl::base::State *tempState = si_->allocState();

    si_->copyState(tempState, startState);

    ompl::base::State *tempEndState = si_->allocState();

    int k = 0;

    while(k < numSteps)
    {
        bool e = executeOneStep(k, tempState,tempEndState, filteringCost, constructionMode);

        k++;

        si_->copyState(tempState, tempEndState);

        si_->copyState(endState, tempEndState);

        if(!e)
        {
            si_->freeState(tempEndState);

            si_->freeState(tempState);

            return false;
        }

    }

    si_->freeState(tempEndState);

    si_->freeState(tempState);

    stepsTaken = k;

    return true;
}

template <class SeparatedControllerType, class FilterType>
void Controller<SeparatedControllerType, FilterType>::Evolve(const ompl::base::State *state, size_t t, ompl::base::State* nextState)
{
    ompl::control::Control* control = separatedController_.generateFeedbackControl(state, t);

    si_->applyControl(control);

    ObservationType zCorrected = si_->getObservation();

    LinearSystem current;
    LinearSystem next;

    current = next = LinearSystem(si_,goal_,
                                si_->getMotionModel()->getZeroControl(),
                                zCorrected,
                                si_->getMotionModel(),
                                si_->getObservationModel());

    if( (Length() > 0) && (t <= Length()-1) )
    {

        if( t == Length() - 1 )
        {
            current = lss_[t];
        }
        else
        {
            current = lss_[t];
            next = lss_[t+1];
        }
    }

    ompl::base::State *nextBelief = si_->allocState();

    filter_.Evolve(state, control, zCorrected, current, next, nextBelief);

    si_->copyState(nextState, nextBelief);

    si_->setBelief(nextBelief);

}


template <class SeparatedControllerType, class FilterType>
void Controller<SeparatedControllerType, FilterType>::Stabilize(const ompl::base::State *startState,
                                                                              ompl::base::State* endState,
                                                                              ompl::base::Cost &stabilizationFilteringCost,
                                                                              int &stepsToStabilize,
                                                                              bool constructionMode)
{
    int k = lss_.size()-1;

    int stepsTaken = 0;

    double cost = 0.0;

    ompl::base::State *tempState1 = si_->allocState();
    ompl::base::State *tempState2 = si_->allocState();

    si_->copyState(tempState1, startState);
    si_->copyState(tempState2, startState);

    while(!goal_->as<StateType>()->isReached(tempState1) && tries_ < maxTries_)
    {

        this->Evolve(tempState1, k, tempState2);

        stepsTaken++;

        arma::mat tempCovMat = tempState2->as<StateType>()->getCovariance();
        cost += arma::trace(tempCovMat); 

        si_->copyState(tempState1, tempState2) ;

        tries_++;

        if(!constructionMode)
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(20));
        }

    }

   //stabilizationFilteringCost.v = cost;
   stabilizationFilteringCost = ompl::base::Cost(cost);

   si_->copyState(endState, tempState2);
   si_->freeState(tempState1);
   si_->freeState(tempState2);
   tries_ = 0;
   stepsToStabilize = stepsTaken;

}

template <class SeparatedControllerType, class FilterType>
bool Controller<SeparatedControllerType, FilterType>::isTerminated(const ompl::base::State *state, const size_t t )
{

    using namespace arma;

    colvec diff = state->as<StateType>()->getArmaData() - goal_->as<StateType>()->getArmaData();

    double distance_to_goal = norm(diff.subvec(0,1),2);

    if( distance_to_goal > nodeReachedDistance_)
    {
        return false;
    }

    return true;

}

#endif
