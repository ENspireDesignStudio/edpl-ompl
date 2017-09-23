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

/* Authors: Sung Kyun Kim, Ali-akbar Agha-mohammadi, Saurav Agarwal */

#include "Planner/FIRMCP.h"
#include "Visualization/Visualizer.h"
#include <boost/circular_buffer.hpp>
#include <tinyxml.h>

#define foreach BOOST_FOREACH
#define foreach_reverse BOOST_REVERSE_FOREACH

FIRMCP::FIRMCP(const firm::SpaceInformation::SpaceInformationPtr &si, bool debugMode)
    : FIRM(si, debugMode)
{
}

FIRMCP::~FIRMCP(void)
{
}

void FIRMCP::loadParametersFromFile(const std::string &pathToFile)
{
    // load parameters for FIRM
    FIRM::loadParametersFromFile(pathToFile);


    // load parameters for FIRMCP
    TiXmlDocument doc(pathToFile);
    bool loadOkay = doc.LoadFile();
    if( !loadOkay )
    {
        printf( "FIRMCP: Could not load setup file. Error='%s'. Exiting.\n", doc.ErrorDesc() );
        exit( 1 );
    }

    TiXmlNode* node = 0;
    TiXmlNode* child = 0;
    TiXmlElement* itemElement = 0;

    node = doc.FirstChild( "FIRMCP" );
    assert( node );


    child = node->FirstChild("numPOMCPParticles");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryIntAttribute("numPOMCPParticles", &numPOMCPParticles_);
    itemElement = 0;

    child = node->FirstChild("maxPOMCPDepth");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryIntAttribute("maxPOMCPDepth", &maxPOMCPDepth_);
    itemElement = 0;

    child = node->FirstChild("maxFIRMReachDepth");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryIntAttribute("maxFIRMReachDepth", &maxFIRMReachDepth_);
    itemElement = 0;

    child = node->FirstChild("cExplorationForSimulate");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryDoubleAttribute("cExplorationForSimulate", &cExplorationForSimulate_);
    itemElement = 0;

    child = node->FirstChild("cExploitationForRolloutOutOfReach");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryDoubleAttribute("cExploitationForRolloutOutOfReach", &cExploitationForRolloutOutOfReach_);
    itemElement = 0;

    child = node->FirstChild("cExploitationForRolloutWithinReach");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryDoubleAttribute("cExploitationForRolloutWithinReach", &cExploitationForRolloutWithinReach_);
    itemElement = 0;

    child = node->FirstChild("costToGoRegulatorOutOfReach");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryDoubleAttribute("costToGoRegulatorOutOfReach", &costToGoRegulatorOutOfReach_);
    itemElement = 0;

    child = node->FirstChild("costToGoRegulatorWithinReach");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryDoubleAttribute("costToGoRegulatorWithinReach", &costToGoRegulatorWithinReach_);
    itemElement = 0;

    child = node->FirstChild("nEpsForIsReached");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryDoubleAttribute("nEpsForIsReached", &nEpsForIsReached_);
    itemElement = 0;

    child = node->FirstChild("heurPosStepSize");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryDoubleAttribute("heurPosStepSize", &heurPosStepSize_);
    itemElement = 0;

    child = node->FirstChild("heurOriStepSize");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryDoubleAttribute("heurOriStepSize", &heurOriStepSize_);
    itemElement = 0;

    child = node->FirstChild("heurCovStepSize");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryDoubleAttribute("heurCovStepSize", &heurCovStepSize_);
    itemElement = 0;

    child = node->FirstChild("covConvergenceRate");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryDoubleAttribute("covConvergenceRate", &covConvergenceRate_);
    itemElement = 0;

    child = node->FirstChild("scaleStabNumSteps");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryIntAttribute("scaleStabNumSteps", &scaleStabNumSteps_);
    itemElement = 0;

    child = node->FirstChild("inflationForApproxStabCost");
    assert( child );
    itemElement = child->ToElement();
    assert( itemElement );
    itemElement->QueryIntAttribute("inflationForApproxStabCost", &inflationForApproxStabCost_);
    itemElement = 0;
}

void FIRMCP::executeFeedbackWithPOMCP(void)
{
//     EdgeControllerType edgeController;
//     NodeControllerType nodeController;

    bool edgeControllerStatus;
    bool nodeControllerStatus;


    const Vertex start = startM_[0];
    const Vertex goal = goalM_[0];
    Vertex currentVertex = start;
    Vertex tempVertex = currentVertex;
    Vertex targetNode;

    ompl::base::State *cstartState = siF_->allocState();
    ompl::base::State *cstartStatePrev = siF_->allocState();
    ompl::base::State *cendState = siF_->allocState();
//     ompl::base::State *goalState = siF_->cloneState(stateProperty_[goal]);
    ompl::base::State *goalState = stateProperty_[goal];
    ompl::base::State *tempTrueStateCopy = siF_->allocState();

    //===== SET A Custom Init Covariance=====================
    // using namespace arma;
    // mat tempCC(3,3);
    // tempCC<< 0.1 << 0.0 << 0.0 << endr
    //         << 0.0 << 0.1 << 0.0 << endr
    //         << 0.0 << 0.0 << 0.0000001<<endr;
    // stateProperty_[start]->as<StateType>()->setCovariance(tempCC);
    // Visualizer::updateCurrentBelief(stateProperty_[start]);
    //================================

    siF_->copyState(cstartState, stateProperty_[start]);
    siF_->copyState(cstartStatePrev, cstartState);
    siF_->setTrueState(stateProperty_[start]);
    siF_->setBelief(stateProperty_[start]);


    // Open a file for writing rollout computation time
    std::ofstream outfile;
    if(doSaveLogs_)
    {
        outfile.open(logFilePath_+"RolloutComputationTime.csv");
        outfile<<"RolloutNum, RadiusNN, NumNN, MCParticles, avgTimePerNeighbor, totalTimeSecs" <<std::endl;
    }

    Visualizer::setMode(Visualizer::VZRDrawingMode::RolloutMode);
    Visualizer::clearRobotPath();
    sendMostLikelyPathToViz(start, goal);

    Visualizer::doSaveVideo(doSaveVideo_);
    siF_->doVelocityLogging(true);
    nodeReachedHistory_.push_back(std::make_pair(currentTimeStep_, numberofNodesReached_) );


    //double averageTimeForRolloutComputation = 0;
    int numberOfRollouts = 0;

    // local variables for robust connection to a desirable (but far) FIRM nodes during rollout
    Edge e = feedback_.at(currentVertex);
    targetNode = boost::target(e, g_);

    // counter of the number of executions of the same edge controller in a row
    int kStepOfEdgeController = 0;
    Edge e_prev;  // do not set this to e from the beginning

    // 4) forcefully include the next FIRM node of the previously reached FIRM node in the candidate (nearest neighbor) list for rollout policy
    //Vertex nextFIRMVertex = boost::target(e, g_);

    // HACK WORKAROUNDS FOR INDEFINITE STABILIZATION DURING ROLLOUT: {1} CONNECTION TO FUTURE FIRM NODES
    // 5) forcefully include future feedback nodes of several previous target nodes in the candidate (nearest neighbor) list
    // initialize a container of FIRM nodes on feedback path
    boost::circular_buffer<Vertex> futureFIRMNodes(numberOfTargetsInHistory_ * numberOfFeedbackLookAhead_);  // it is like a size-limited queue or buffer


    OMPL_INFORM("FIRMCP: Running POMCP on top of FIRM");

    // While the robot state hasn't reached the goal state, keep running
    // HACK setting relaxedConstraint argument to false means isReached() condition is exceptionally relaxed for termination
    while(!goalState->as<FIRM::StateType>()->isReached(cstartState, true))
    //while(!goalState->as<FIRM::StateType>()->isReached(cstartState, false))
    {
        /**
          Instead of executing the entire controller, we need to execute N steps, then calculate the cost to go through the neighboring nodes.
          Whichever gives the lowest cost to go, is our new path. Do this at every N steps.
        */

        // [4-2] Rollout
        // TODO need to clean up code
        // NOTE commented this to do rollout even if the robot (almost) reached a FIRM node
        //else
        {
            Visualizer::doSaveVideo(false);
            siF_->doVelocityLogging(false);

            // start profiling time to compute rollout
            auto start_time = std::chrono::high_resolution_clock::now();


            // FIRMCP

            // add the current belief state to the graph
//             tempVertex = addStateToGraph(cstartState, false);

//     ompl::base::State* currentBelief = cstartStatePrev;  // latest start state that is already executed
    ompl::base::State* currentBelief = stateProperty_[tempVertex];  // latest start state that is already executed
    ompl::base::State* nextBelief = cstartState;         // current start state to be executed
    Vertex selectedChildQnode = targetNode;              // target node of last execution

    // ADD A QVNODE TO THE POMCP TREE
    // if the transioned state after simulated execution, T(h, a_j, o_k), is near to any of existing childQVnodes_[selectedChildQnode] (for the same action), T(h, a_j, o_l), on POMCP tree, merge them into one node!
    // REVIEW TODO how to update the existing belief state when another belief state is merged into this?
    // XXX currently, just keep the very first belief state without updating its belief state
    // but the first belief state won't represent the all belief states that are merged into it
    Vertex nextVertex;
    std::vector<Vertex> reachedChildQVnodes;
//     const std::vector<Vertex>& selectedChildQVnodes = currentBelief->as<FIRM::StateType>()->getChildQVnodes(selectedChildQnode);
    const Vertex selectedChildQVnode = currentBelief->as<FIRM::StateType>()->getChildQVnode(selectedChildQnode);

// 1)
//             // ORIGINAL
//             for (int j=0; j<selectedChildQVnodes.size(); j++)
//             {
//                 Vertex childQVnode = selectedChildQVnodes[j];
//
//                 // NOTE need to check for both directions since there is no from-to relationship between these selectedChildQVnodes
//                 if (stateProperty_[childQVnode]->as<FIRM::StateType>()->isReached(nextBelief))
//                 {
//                     //OMPL_WARN("Existing childQVnode!");
//                     nextVertex = childQVnode;
//                     reachedChildQVnodes.push_back(childQVnode);
//                 }
//                 else if (nextBelief->as<FIRM::StateType>()->isReached(stateProperty_[childQVnode]))
//                 {
//                     //OMPL_WARN("Existing childQVnode!");
//                     nextVertex = childQVnode;
//                     reachedChildQVnodes.push_back(childQVnode);
//                 }
//             }
//             // REVIEW what if a new childQVnode coincides more than one existing selectedChildQVnodes (for the same action)?
//             // pick the closest one?
//             // OR merge all selectedChildQVnodes[selectedChildQnode] into one node but with proper belief state merging scheme?!
//             if (reachedChildQVnodes.size()>1)
//             {
//                 OMPL_WARN("There are %d reachedChildQVnodes for this new childQVnode!", reachedChildQVnodes.size());
//                 //nextVertex = which childQVnode?
//                 //exit(0);    // XXX
//
//                 int random = rand() % reachedChildQVnodes.size();
//                 nextVertex = reachedChildQVnodes[random];  // to break the tie
//             }
//             else if (reachedChildQVnodes.size()==0)

// 2)
//         if (selectedChildQVnodes.size()!=0)
//         {
//             int random = rand() % selectedChildQVnodes.size();
//             nextVertex = selectedChildQVnodes[random];  // to break the tie
//
//             // update the matching belief state
//             siF_->copyState(stateProperty_[nextVertex], nextBelief);
//         }
//         else

// 3)
        if (selectedChildQVnode != ompl::magic::INVALID_VERTEX_ID)
        {
            nextVertex = selectedChildQVnode;

            // update the matching belief state
            // TODO belief state update from all particles, not just the latest one
            siF_->copyState(stateProperty_[nextVertex], nextBelief);
        }
        else


        {
            OMPL_INFORM("A new childQVnode after actual execution!");
            nextVertex = addQVnodeToPOMCPTree(siF_->cloneState(nextBelief));
            currentBelief->as<FIRM::StateType>()->addChildQVnode(selectedChildQnode, nextVertex);
        }


    // for debug
    OMPL_INFORM("FIRMCP: Moved from Vertex %u (%2.3f, %2.3f, %2.3f, %2.6f) to %u (%2.3f, %2.3f, %2.3f, %2.6f)", tempVertex, nextVertex, 
            stateProperty_[tempVertex]->as<FIRM::StateType>()->getX(),
            stateProperty_[tempVertex]->as<FIRM::StateType>()->getY(),
            stateProperty_[tempVertex]->as<FIRM::StateType>()->getYaw(),
            arma::trace(stateProperty_[tempVertex]->as<FIRM::StateType>()->getCovariance()),
            stateProperty_[nextVertex]->as<FIRM::StateType>()->getX(),
            stateProperty_[nextVertex]->as<FIRM::StateType>()->getY(),
            stateProperty_[nextVertex]->as<FIRM::StateType>()->getYaw(),
            arma::trace(stateProperty_[nextVertex]->as<FIRM::StateType>()->getCovariance()));
    // for debug
    //std::cout << tempVertex << " #[" << selectedChildQnode << "]# " << nextVertex;
    //OMPL_INFORM("FIRMCP: Moved %d #[%d]# %d", tempVertex, selectedChildQnode, nextVertex);


        // prune the old tree to free the memory
        // for debug
        //std::cout << "prunedNodes:";
//         if (reachedChildQVnodes.size()!=0)  // nextBelief matches at least one node on POMCP tree
//         {
//         if (selectedChildQVnode != ompl::magic::INVALID_VERTEX_ID)
        {
            const std::vector<Vertex>& childQnodes = currentBelief->as<FIRM::StateType>()->getChildQnodes();
            for (const auto& childQnode : childQnodes)
            {
//                 const std::vector<Vertex>& childQVnodes = currentBelief->as<FIRM::StateType>()->getChildQVnodes(childQnode);
//                 for (const auto& childQVnode : childQVnodes)
//                 {
//                     if (childQVnode != nextVertex)
//                     {
//                         prunePOMCPTreeFrom(childQVnode);
//                     }
//                 }

                const Vertex childQVnode = currentBelief->as<FIRM::StateType>()->getChildQVnode(childQnode);
                if (childQVnode != ompl::magic::INVALID_VERTEX_ID)
                {
                    if (childQVnode != nextVertex)
                    {
                        prunePOMCPTreeFrom(childQVnode);
                    }
                }
            }
//             prunePOMCPNode(tempVertex);
        }
//         else  // nextBelief does not match any nodes on POMCP tree
//         if (selectedChildQVnode == ompl::magic::INVALID_VERTEX_ID)
//         {
// //             prunePOMCPTreeFrom(tempVertex);
//             prunePOMCPNode(tempVertex);
//         }
        // for debug
        //std::cout << std::endl;


            // save the current true state
            siF_->getTrueState(tempTrueStateCopy);

    // if want/do not want to show monte carlo sim
    siF_->showRobotVisualization(ompl::magic::SHOW_MONTE_CARLO);


    tempVertex = nextVertex;

            // select the best next edge
            e = generatePOMCPPolicy(tempVertex, goal);

    // enable robot visualization again
    siF_->showRobotVisualization(true);

            // restore the current true state
            siF_->setTrueState(tempTrueStateCopy);


            // if the edge controller of the last execution is being used again now, apply the kStep'th open-loop control of the edge controller
            if (e == e_prev)
                kStepOfEdgeController++;
            else
                kStepOfEdgeController = 0;

            e_prev = e;




            // end profiling time to compute rollout
            auto end_time = std::chrono::high_resolution_clock::now();

            numberOfRollouts++;
            double timeToDoRollout = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            //int numNN = numNearestNeighbors_;
            //averageTimeForRolloutComputation += timeToDoRollout / numNN;    // rollout candidates are no longer limited to numNearestNeighbors_
            if(doSaveLogs_)
            {
                //outfile<<numberOfRollouts<<","<<NNRadius_<<","<<numNN<<","<<numMCParticles_<<","<<timeToDoRollout/(1000*numNN)<<","<<timeToDoRollout/1000<<std::endl;
                outfile<<numberOfRollouts<<","<<NNRadius_<<","<<numMCParticles_<<","<<timeToDoRollout/1000<<std::endl;
            }
            //std::cout << "Time to execute rollout : "<<timeToDoRollout << " milli seconds."<<std::endl;

            Visualizer::doSaveVideo(doSaveVideo_);
            siF_->doVelocityLogging(true);



        siF_->copyState(cstartStatePrev, cstartState);
        targetNode = boost::target(e, g_);

//         double succProb = evaluateSuccessProbability(e, tempVertex, goal);
//         successProbabilityHistory_.push_back(std::make_pair(currentTimeStep_, succProb ) );

        OMPL_INFORM("FIRMCP: Moving from Vertex %u (%2.3f, %2.3f, %2.3f, %2.6f) to [%u] (%2.3f, %2.3f, %2.3f, %2.6f)", tempVertex, targetNode, 
                stateProperty_[tempVertex]->as<FIRM::StateType>()->getX(),
                stateProperty_[tempVertex]->as<FIRM::StateType>()->getY(),
                stateProperty_[tempVertex]->as<FIRM::StateType>()->getYaw(),
                arma::trace(stateProperty_[tempVertex]->as<FIRM::StateType>()->getCovariance()),
                stateProperty_[targetNode]->as<FIRM::StateType>()->getX(),
                stateProperty_[targetNode]->as<FIRM::StateType>()->getY(),
                stateProperty_[targetNode]->as<FIRM::StateType>()->getYaw(),
                arma::trace(stateProperty_[targetNode]->as<FIRM::StateType>()->getCovariance()));

        // HACK WORKAROUNDS FOR INDEFINITE STABILIZATION DURING ROLLOUT: {1} CONNECTION TO FUTURE FIRM NODES
        // 5) forcefully include future feedback nodes of several previous target nodes in the candidate (nearest neighbor) list
        // update the future feedback node at every iteration
//         if(connectToFutureNodes_)
//         {
//             Vertex futureVertex = targetNode;
//             for(int i=0; i<numberOfFeedbackLookAhead_; i++)
//             {
//                 futureFIRMNodes.push_back(futureVertex);
//
//                 if(feedback_.find(futureVertex) != feedback_.end())    // there exists a valid feedback edge for this vertex
//                 {
//                     futureVertex = boost::target(feedback_.at(futureVertex), g_);
//                 }
//                 else    // there is no feedback edge coming from this vertex
//                 {
//                     // fill the rest of the buffer with the last valid vertex; redundant vertices will be ignored for rollout check
//                     for(int j=i+1; j<numberOfFeedbackLookAhead_; j++)
//                     {
//                         futureFIRMNodes.push_back(futureVertex);
//                     }
//                     break;
//                 }
//             }
//             // for debug
//             // if(ompl::magic::PRINT_FUTURE_NODES)
//             // {
//             //     std::cout << "futureFIRMNodesDuplicate: { ";
//             //     foreach(futureVertex, futureFIRMNodes)
//             //         std::cout << futureVertex << " ";
//             //     std::cout << "}" << std::endl;
//             // }
//         }



            // NOTE commented to prevent redundant call of nearest neighbor search just for visualization! moved to FIRM::addStateToGraph()
            //showRolloutConnections(tempVertex);

            // for rollout edge visualization
            //boost::this_thread::sleep(boost::posix_time::milliseconds(50));  // doesn't seem to be necessary

            // clear the rollout candidate connection drawings and show the selected edge
            Visualizer::clearRolloutConnections();
            Visualizer::setChosenRolloutConnection(stateProperty_[tempVertex], stateProperty_[targetNode]);

        } // [4] Rollout




        ompl::base::Cost costCov;
        int stepsExecuted = 0;
        int stepsToStop = 0;

        // NOTE NodeController will be invoked after executing EdgeController for the given rolloutSteps_ steps

        // [1] EdgeController
        EdgeControllerType& edgeController = edgeControllers_.at(e);
        edgeController.setSpaceInformation(policyExecutionSI_);
        if(!edgeController.isTerminated(cstartState, 0))  // check if cstartState is near to the target FIRM node (by x,y position); this is the termination condition B) for EdgeController::Execute()
        {
            // NOTE do not execute edge controller to prevent jiggling motion around the target node

            //edgeControllerStatus = edgeController.executeUpto(rolloutSteps_, cstartState, cendState, costCov, stepsExecuted, false);
            edgeControllerStatus = edgeController.executeFromUpto(kStepOfEdgeController, rolloutSteps_, cstartState, cendState, costCov, stepsExecuted, false);

            // NOTE how to penalize uncertainty (covariance) and path length (time steps) in the cost
            //*1) cost = wc * sum(trace(cov_k))  + wt * K  (for k=1,...,K)
            // 2) cost = wc * trace(cov_f)       + wt * K
            // 3) cost = wc * mean(trace(cov_k)) + wt * K
            // 4) cost = wc * sum(trace(cov_k))

            currentTimeStep_ += stepsExecuted;

            executionCostCov_ += costCov.value() - ompl::magic::EDGE_COST_BIAS;    // 1,2,3,4) costCov is actual execution cost but only for covariance penalty (even without weight multiplication)

            executionCost_ = informationCostWeight_*executionCostCov_ + timeCostWeight_*currentTimeStep_;    // 1)
            //executionCost_ = informationCostWeight_*executionCostCov_/(currentTimeStep_==0 ? 1e-10 : currentTimeStep_) + timeCostWeight_*currentTimeStep_;    // 3)
            //executionCost_ = informationCostWeight_*executionCostCov_;    // 4)

            costHistory_.push_back(std::make_tuple(currentTimeStep_, executionCostCov_, executionCost_));


            // this is a secondary (redundant) collision check for the true state
            siF_->getTrueState(tempTrueStateCopy);
            if(!siF_->isValid(tempTrueStateCopy))
            {
                OMPL_INFORM("Robot Collided :(");
                return;
            }

            // update cstartState for next iteration
            siF_->copyState(cstartState, cendState);

        } // [1] EdgeController

        // [2] NodeController
        else
        {
//             // HACK WORKAROUNDS FOR INDEFINITE STABILIZATION DURING ROLLOUT: {2} ACCUMULATING STATIONARY PENALTY
//             if(applyStationaryPenalty_)
//             {
//                 // incrementally penalize a node that is being selected as a target due to the not-yet-converged current covariance even after the robot reached that node's position and orientation
//                 // NOTE this is to myopically improve the suboptimal policy based on approximate value function (with inaccurate edge cost induced from isReached() relaxation)
//                 // it will help to break (almost) indefinite stabilization process during rollout, especially when land marks are not very close
//                 if(stateProperty_[targetNode]->as<FIRM::StateType>()->isReachedPose(cstartState))
//                 {
//                     if(targetNode != goal)
//                     {
//                         // increase the stationary penalty
//                         if(stationaryPenalties_.find(targetNode) == stationaryPenalties_.end())
//                         {
//                             stationaryPenalties_[targetNode] = statCostIncrement_;
//                             // for log
//                             numberOfStationaryPenalizedNodes_++;
//                         }
//                         else
//                         {
//                             stationaryPenalties_[targetNode] += statCostIncrement_;
//                         }
//                         // for log
//                         sumOfStationaryPenalties_ += statCostIncrement_;
// //                         stationaryPenaltyHistory_.push_back(std::make_tuple(currentTimeStep_, numberOfStationaryPenalizedNodes_, sumOfStationaryPenalties_));
//
//                         // for debug
//                         if(ompl::magic::PRINT_STATIONARY_PENALTY)
//                             std::cout << "stationaryPenalty[" << targetNode << "]: " << stationaryPenalties_[targetNode] << std::endl;
//                     }
//                 }
//             }
            // NOTE tried applying the stationary penalty if isReached(), instead of isTerminated(), is satisfied, but the resultant policy was more suboptimal
            //if(stateProperty_[targetNode]->as<FIRM::StateType>()->isReached(cstartState))
            //{
            //}

            // call StabilizeUpto() at every rollout iteration
            {
                NodeControllerType& nodeController = nodeControllers_.at(targetNode);
                nodeController.setSpaceInformation(policyExecutionSI_);
                nodeControllerStatus = nodeController.StabilizeUpto(rolloutSteps_, cstartState, cendState, costCov, stepsExecuted, false);


                // NOTE how to penalize uncertainty (covariance) and path length (time steps) in the cost
                //*1) cost = wc * sum(trace(cov_k))  + wt * K  (for k=1,...,K)
                // 2) cost = wc * trace(cov_f)       + wt * K
                // 3) cost = wc * mean(trace(cov_k)) + wt * K
                // 4) cost = wc * sum(trace(cov_k))

                currentTimeStep_ += stepsExecuted;

                executionCostCov_ += costCov.value() - ompl::magic::EDGE_COST_BIAS;    // 1,2,3,4) costCov is actual execution cost but only for covariance penalty (even without weight multiplication)

                executionCost_ = informationCostWeight_*executionCostCov_ + timeCostWeight_*currentTimeStep_;    // 1)
                //executionCost_ = informationCostWeight_*executionCostCov_/(currentTimeStep_==0 ? 1e-10 : currentTimeStep_) + timeCostWeight_*currentTimeStep_;    // 3)
                //executionCost_ = informationCostWeight_*executionCostCov_;    // 4)

                costHistory_.push_back(std::make_tuple(currentTimeStep_, executionCostCov_, executionCost_));


                // this is a secondary (redundant) collision check for the true state
                siF_->getTrueState(tempTrueStateCopy);
                if(!siF_->isValid(tempTrueStateCopy))
                {
                    OMPL_INFORM("Robot Collided :(");
                    return;
                }

                // update the cstartState for next iteration
                siF_->copyState(cstartState, cendState);

            }
        } // [2] NodeController


        // XXX HACK CHECK REVIEW
//         // [3] Free the memory for states and controls for this temporary node/edge created from previous iteration
//         if(tempVertex != start)
//         {
//             foreach(Edge edge, boost::out_edges(tempVertex, g_))
//             {
//                 edgeControllers_[edge].freeSeparatedController();
//                 edgeControllers_[edge].freeLinearSystems();
//                 edgeControllers_.erase(edge);
//             }
//
//             // NOTE there is no node controller generated for this temporary node during rollout execution
//
//
//             // remove the temporary node/edges after executing one rollout iteration
//             // NOTE this is important to keep tempVertex to be the same over each iteration
//             boost::clear_vertex(tempVertex, g_);    // remove all edges from or to tempVertex
//             boost::remove_vertex(tempVertex, g_);   // remove tempVertex
//             //stateProperty_.erase(tempVertex);
//             nn_->remove(tempVertex);
//         }


        // [4-1] Rollout
        // check if cendState after execution has reached targetNode, just for logging
        if(stateProperty_[targetNode]->as<FIRM::StateType>()->isReached(cendState))
        {
            OMPL_INFORM("FIRMCP: Reached FIRM Node: %u", targetNode);
            numberofNodesReached_++;
            nodeReachedHistory_.push_back(std::make_pair(currentTimeStep_, numberofNodesReached_) );

            // NOTE commented this to do rollout even if the robot (almost) reached a FIRM node
            // tempVertex = boost::target(e, g_);
            // // if the reached node is not the goal
            // if(tempVertex != goal)
            // {
            //     e = feedback_.at(tempVertex);    // NOTE this is not guaranteed to be the best
            //     // for debug
            //     nextFIRMVertex = boost::target(e, g_);
            // }
        }

        // for debug
//         siF_->getTrueState(tempTrueStateCopy);
//         OMPL_INFORM("tempTrueStateCopy: (%2.3f, %2.3f, %2.3f, %2.6f)",
//                 tempTrueStateCopy->as<FIRM::StateType>()->getX(),
//                 tempTrueStateCopy->as<FIRM::StateType>()->getY(),
//                 tempTrueStateCopy->as<FIRM::StateType>()->getYaw(),
//                 arma::trace(tempTrueStateCopy->as<FIRM::StateType>()->getCovariance()));


    } // while()

    // XXX HACK CHECK REVIEW
//     // [3'] Free the memory for states and controls for this temporary node/edge created from previous iteration
//     if(tempVertex != start)
//     {
//         foreach(Edge edge, boost::out_edges(tempVertex, g_))
//         {
//             edgeControllers_[edge].freeSeparatedController();
//             edgeControllers_[edge].freeLinearSystems();
//             edgeControllers_.erase(edge);
//         }
//
//         // NOTE there is no node controller generated for this temporary node during rollout execution
//
//
//         // remove the temporary node/edges after executing one rollout iteration
//         // NOTE this is important to keep tempVertex to be the same over each iteration
//         boost::clear_vertex(tempVertex, g_);    // remove all edges from or to tempVertex
//         boost::remove_vertex(tempVertex, g_);   // remove tempVertex
//         //stateProperty_.erase(tempVertex);
//         nn_->remove(tempVertex);
//     }

    nodeReachedHistory_.push_back(std::make_pair(currentTimeStep_, numberofNodesReached_) );

    //OMPL_INFORM("FIRM: Number of nodes reached with Rollout: %u", numberofNodesReached_);
    //averageTimeForRolloutComputation = averageTimeForRolloutComputation / (1000*numberOfRollouts);
    //std::cout<<"Nearest Neighbor Radius: "<<NNRadius_<<", Monte Carlo Particles: "<<numMCParticles_<<", Avg Time/neighbor (seconds): "<<averageTimeForRolloutComputation<<std::endl;    


    // for analysis

    // this data is also saved in run-(TIMESTAMP)/FIRMCPCostHistory.csv
    std::cout << std::endl;
    std::cout << "Execution time steps: " << currentTimeStep_ << std::endl;
    std::cout << "Execution covariance cost: " << executionCostCov_ << std::endl;
    std::cout << "Execution cost: " << executionCost_ << "  ( = " << informationCostWeight_ << "*" << executionCostCov_ << " + " << timeCostWeight_ << "*" << currentTimeStep_ << " )" << std::endl;     // 1)
    //std::cout << "Execution cost: " << executionCost_ << "  ( = " << informationCostWeight_ << "*" << executionCostCov_ << "/" << currentTimeStep_ << " + " << timeCostWeight_ << "*" << currentTimeStep_ << " )" << std::endl;     // 3)
    //std::cout << "Execution cost: " << executionCost_ << "  ( = " << informationCostWeight_ << "*" << executionCostCov_ << " )" << std::endl;     // 4)
    std::cout << std::endl;

    // this data is also saved in run-(TIMESTAMP)/FIRMCPCostHistory.csv
    std::cout << "Number of nodes with stationary penalty: " << numberOfStationaryPenalizedNodes_ << std::endl;
    std::cout << "Sum of stationary penalties: " << sumOfStationaryPenalties_ << std::endl;


    if(doSaveLogs_)
    {
        outfile.close();
        writeTimeSeriesDataToFile("FIRMCPCostHistory.csv", "costToGo");
        writeTimeSeriesDataToFile("FIRMCPSuccessProbabilityHistory.csv", "successProbability");
        writeTimeSeriesDataToFile("FIRMCPNodesReachedHistory.csv","nodesReached");
        writeTimeSeriesDataToFile("FIRMCPStationaryPenaltyHistory.csv","stationaryPenalty");
        std::vector<std::pair<double, double>> velLog;
        siF_->getVelocityLog(velLog);
        for(int i=0; i < velLog.size(); i++)
        {
            velocityHistory_.push_back(std::make_pair(i, sqrt( pow(velLog[i].first,2) + pow(velLog[i].second,2) ))); // omni
            //velocityHistory_.push_back(std::make_pair(i, velLog[i].first)); // unicycle
        }
        writeTimeSeriesDataToFile("FIRMCPVelocityHistory.csv", "velocity");
    }
    Visualizer::doSaveVideo(true);
    sleep(0.33);

    // free the memory
    siF_->freeState(cstartState);
    siF_->freeState(cstartStatePrev);
    siF_->freeState(cendState);
    siF_->freeState(tempTrueStateCopy);
}

FIRM::Edge FIRMCP::generatePOMCPPolicy(const FIRM::Vertex currentVertex, const FIRM::Vertex goal)
{
    // declare local variables
    ompl::base::State* tempTrueStateCopy = siF_->allocState();
    ompl::base::State* sampState = siF_->allocState();


    // save the current true state
    siF_->getTrueState(tempTrueStateCopy);
    Visualizer::setMode(Visualizer::VZRDrawingMode::FIRMCPMode);


    // Update()

    // NOTE this needs to move to executeFeedbackWithPOMCP() after actual execution by edge/node controllers
    // add the executed (a,o) pair to history


    // check if the end state matches a node in the tree


    // prune the old tree and set the end state as a root


    // --------------


    // for N particles
    for (unsigned int i=0; i<numPOMCPParticles_; i++)
    {
        // sample a particle from the root (current) belief state
        // NOTE random sampling of a true state from the current belief state for Monte Carlo simulation
        double nsigma = 3.0;   // HACK to increase the chance of detecting collision with a few number of particles
        if(!stateProperty_[currentVertex]->as<FIRM::StateType>()->sampleTrueStateFromBelief(sampState, nsigma))
        {
            OMPL_WARN("Could not sample a true state from the current belief state!");
            continue;
        }
        siF_->setTrueState(sampState);  // true state is only used for collision check by checkTrueStateValidity()
        // for debug
        std::cout << currentVertex;

        // run Monte Carlo simulation for one particle and update cost-to-go and number of visits
        int currentDepth = 0;
        int collisionDepth = (int)ompl::magic::DEFAULT_INF_COST_TO_GO;  // initialize as if collision never happens
        Edge selectedEdgeDummy;  // just for syntax

        double totalCostToGo = pomcpSimulate(currentVertex, currentDepth, selectedEdgeDummy, collisionDepth);
        
//     // NOTE penalize collision with discount
//     // obstacleCostToGo_ would affect up to ~5 steps from the depth of collision and decay beyond it, so that its parent branch can have a chance to be selected
//     int depthDiff = collisionDepth - currentDepth;
//     depthDiff = (depthDiff < 0) ? 0 : depthDiff;
//     double discountedPenalty = obstacleCostToGo_ * 1.0/(1 + std::exp(2*(depthDiff-3)));
// //     totalCostToGo += computeDiscountedCollisionPenalty(executionCost, currentDepth, collisionDepth);
// //     totalCostToGo += discountedPenalty;

//     double totalCostToGoWithDiscountedPenalty = totalCostToGo + discountedPenalty;


// for debug
//         std::cout << "totalCostToGo: " << totalCostToGo << std::endl;
//         std::cout << "totalCostToGoWithDiscountedPenalty: " << totalCostToGoWithDiscountedPenalty << std::endl;
//         std::cout << "particleCostToGo: " << totalCostToGoWithDiscountedPenalty << "(=" << totalCostToGo << "+" << discountedPenalty << ")" << std::endl;
        std::cout << "thisQVmincosttogo: " << totalCostToGo << std::endl;
    }

    // select the best action
            const std::vector<Vertex>& childQnodes = stateProperty_[currentVertex]->as<FIRM::StateType>()->getChildQnodes();
//             const std::vector<Vertex>& childQnodes = stateProperty_[currentVertex]->as<FIRM::StateType>()->getChildQnodes();
            double minQcosttogo = infiniteCostToGo_;
            std::vector<Vertex> minQcosttogoNodes;
            Vertex childQnode, selectedChildQnode;
            double childQcosttogo;
            // for debug
            std::cout << "childQcosttogoes: ";
            for (int j=0; j<childQnodes.size(); j++)
            {
                childQnode = childQnodes[j];
                childQcosttogo = stateProperty_[currentVertex]->as<FIRM::StateType>()->getChildQcosttogo(childQnode);
                // for debug
//                 double childQvalue = stateProperty_[currentVertex]->as<FIRM::StateType>()->getChildQvalue(childQnode);
//                 double childQpenalty = stateProperty_[currentVertex]->as<FIRM::StateType>()->getChildQpenalty(childQnode);
//                 std::cout << "[" << childQnode << "]" << childQcosttogo << "(=" << childQvalue << "+" << childQpenalty << ") ";
                std::cout << "[" << childQnode << "]" << childQcosttogo << " ";

                if (minQcosttogo >= childQcosttogo)
                {
                    if (minQcosttogo > childQcosttogo)
                    {
                        minQcosttogoNodes.clear();
                    }
                    minQcosttogo = childQcosttogo;
                    minQcosttogoNodes.push_back(childQnode);
                }
            }
            if (minQcosttogoNodes.size()==1)
            {
                selectedChildQnode = minQcosttogoNodes[0];
            }
            else
            {
                assert(minQcosttogoNodes.size()!=0);
                //OMPL_WARN("More than one childQnodes are with the minQcosttogo!");
                int random = rand() % minQcosttogoNodes.size();
                selectedChildQnode = minQcosttogoNodes[random];  // to break the tie
            }
            // for debug
            std::cout << std::endl;
            std::cout << "minQcosttogo: " << "[" << selectedChildQnode << "]" << minQcosttogo << std::endl;
            std::cout << "executionCost: " << executionCost_ << std::endl;
            std::cout << "expTotalCost: " << minQcosttogo + executionCost_ << std::endl;

            // for debug
//             OMPL_INFORM("FIRMCP-Execute ##### selectedChlidQnode %u (%2.3f, %2.3f, %2.3f, %2.6f)", selectedChildQnode, 
//                     stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getX(),
//                     stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getY(),
//                     stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getYaw(),
//                     arma::trace(stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getCovariance()));

            Edge selectedEdge = boost::edge(currentVertex, selectedChildQnode, g_).first;


    // restore the current true state
    siF_->setTrueState(tempTrueStateCopy);
    Visualizer::setMode(Visualizer::VZRDrawingMode::RolloutMode);


    // free the memory
    siF_->freeState(tempTrueStateCopy);
    siF_->freeState(sampState);

    return selectedEdge;
}

double FIRMCP::pomcpSimulate(const Vertex currentVertex, const int currentDepth, const Edge& selectedEdgePrev, int& collisionDepth)
{
    // declare local variables
    ompl::base::State* currentBelief = stateProperty_[currentVertex];
    Edge selectedEdge;
    Vertex selectedChildQnode;
    double delayedCostToGo;
    double executionCost;
    bool isNewNodeExpanded = false;


    // CREATE A NEW NODE IF NOT COINCIDES ANY OF EXISTING POMCP TREE NODES
//     if ((currentBelief->as<FIRM::StateType>()->getChildQnodes()).size()==0)
//     if ((currentBelief->as<FIRM::StateType>()->getChildQvalues()).size()==0)  // NOTE childQvalues are added only if this node was expanded before in expandQnodesOnPOMCPTreeWithApproxCostToGo()
    if (!currentBelief->as<FIRM::StateType>()->getChildQexpanded())  // NOTE childQvalues are added only if this node was expanded before in expandQnodesOnPOMCPTreeWithApproxCostToGo()
    {
        isNewNodeExpanded = true;  // so, initialize N(ha) and V(ha) of this node for all actions

        // if (currentDepth > maxPOMCPDepth_), pomcpRollout() will handle it

        delayedCostToGo = pomcpRollout(currentVertex, currentDepth, selectedEdgePrev, collisionDepth, isNewNodeExpanded);   // pass isNewNodeExpanded only for the first pomcpRollout()
        executionCost = 0.0;

        // total cost-to-go from this node
        double discountFactor = 1.0;
        double totalCostToGo = executionCost + discountFactor*delayedCostToGo;

        
        // no need to do this here
//     // NOTE penalize collision with discount
//     // obstacleCostToGo_ would affect up to ~5 steps from the depth of collision and decay beyond it, so that its parent branch can have a chance to be selected
//     int depthDiff = collisionDepth - currentDepth;
//     depthDiff = (depthDiff < 0) ? 0 : depthDiff;
//     double discountedPenalty = obstacleCostToGo_ * 1.0/(1 + std::exp(2*(depthDiff-3)));
// //     totalCostToGo += computeDiscountedCollisionPenalty(executionCost, currentDepth, collisionDepth);
// //     totalCostToGo += discountedPenalty;
//     double totalCostToGoWithDiscountedPenalty = totalCostToGo + discountedPenalty;

        return totalCostToGo;
    }
    else
    {

        if (currentDepth >= maxPOMCPDepth_)
        {
            Vertex targetVertex = boost::target(selectedEdgePrev, g_);   // latest target before reaching the finite horizon

            if (currentDepth >= maxFIRMReachDepth_)
            {
                OMPL_WARN("Could not reach to the target node within %d iterations", maxFIRMReachDepth_);
                //return obstacleCostToGo_;

// //                 // approximately penalize the stabilization cost in the case that it immediately stops recursive simulation
// //                 double executionCost = currentBelief->as<FIRM::StateType>()->getTraceCovariance();
// //                 executionCost *= maxFIRMReachDepth_ - currentDepth;
//                 collisionDepth = currentDepth;  // mark that collision happened at this depth from the root
// //                 return executionCost;
//
//                 // compute approximate edge cost and cost-to-go
//                 double approxEdgeCost = computeApproxEdgeCost(currentVertex, targetVertex);
//                 double approxCostToGo = costToGo_[targetVertex] + approxEdgeCost;
//
//                 return approxCostToGo;


                double totalCostToGo = obstacleCostToGo_;

                // UPDATE THE NUMBER OF VISITS AND COST-TO-GO
                currentBelief->as<FIRM::StateType>()->addThisQVvisit();                    // N(h) += 1
                currentBelief->as<FIRM::StateType>()->setThisQVmincosttogo(totalCostToGo);

                return totalCostToGo;
            }

            // TODO CONTINUE TO MOVE TOWARD THE LATEST TARGET FIRM NODE AND RETURN COST-TO-GO
            if (stateProperty_[targetVertex]->as<FIRM::StateType>()->isReached(currentBelief))
            {
                // clear the rollout candidate connection drawings and show the selected edge
                Visualizer::clearRolloutConnections();
                //Visualizer::setChosenRolloutConnection(stateProperty_[tempVertex], stateProperty_[targetNode]);

                // for debug
                std::cout << std::endl;

                // compute approximate edge cost and cost-to-go
                double approxEdgeCost = computeApproxEdgeCost(currentVertex, targetVertex);
//                 double approxCostToGo = costToGo_[targetVertex] + approxEdgeCost;
                double approxCostToGo = getCostToGoWithApproxStabCost(targetVertex) + approxEdgeCost;

                // UPDATE THE NUMBER OF VISITS AND COST-TO-GO
                currentBelief->as<FIRM::StateType>()->addThisQVvisit();                    // N(h) += 1
                currentBelief->as<FIRM::StateType>()->setThisQVmincosttogo(approxCostToGo);

                return approxCostToGo;
//                 return costToGo_[targetVertex];
            }

            selectedEdge = selectedEdgePrev;    // selectedEdgePrev should be one of function arguments
            selectedChildQnode = targetVertex;

            // check if previously selected action is still valid for this node
            const std::vector<Vertex>& childQnodes = currentBelief->as<FIRM::StateType>()->getChildQnodes();
            if (std::find(childQnodes.begin(), childQnodes.end(), selectedChildQnode) == childQnodes.end())
            {
                OMPL_WARN("selectedChildQnode action for %d node to reach a FIRM node %d during pomcpSimulate() is not available for this current node!", currentVertex, selectedChildQnode);

                double totalCostToGo = obstacleCostToGo_;

                // UPDATE THE NUMBER OF VISITS AND COST-TO-GO
                currentBelief->as<FIRM::StateType>()->addThisQVvisit();                    // N(h) += 1
                currentBelief->as<FIRM::StateType>()->setThisQVmincosttogo(totalCostToGo);

                return totalCostToGo;
            }

        } // if (currentDepth >= maxPOMCPDepth_)
        else
        {


            // create a new node with default invalid N(ha) and V(ha)
            // need to register as a Vertex
            // need to add this to nearest neighbor database
            // find nearest neighbor child FIRM nodes, not any of POMCP tree nodes
            // then, save this child node id's in the Vertex attibute
            // also save estimated edge cost (from ditance) in the Vertex attibute
            // need edge/node controllers to its neighbors
            // but without edge cost computation



            // SELECT AN ACTION USING GREEDY UCB POLICY
            const std::vector<Vertex>& childQnodes = currentBelief->as<FIRM::StateType>()->getChildQnodes();
            double minQcosttogo = infiniteCostToGo_;
            std::vector<Vertex> minQcosttogoNodes;
            Vertex childQnode;
            double childQcosttogo;
            for (int j=0; j<childQnodes.size(); j++)
            {
                childQnode = childQnodes[j];
                childQcosttogo = currentBelief->as<FIRM::StateType>()->getChildQcosttogo(childQnode);

                // for debug
//                 std::cout << "childQcosttogo = " << childQcosttogo;

                // apply exploration bonus!
                double thisQVvisit = currentBelief->as<FIRM::StateType>()->getThisQVvisit();            // N(h)
                double childQvisit = currentBelief->as<FIRM::StateType>()->getChildQvisit(childQnode);  // N(ha)
                // NOTE we minimize, not maximize, the cost-to-go
//                 childQcosttogo -= cExplorationForSimulate_ * std::sqrt( std::log(thisQVvisit+1) / (childQvisit+1) );
                childQcosttogo -= cExplorationForSimulate_ * std::sqrt( std::log(thisQVvisit+1.0) / (childQvisit+1e-10) );
                // for debug
//                 std::cout << " - " << cExplorationForSimulate_ << " * " << std::sqrt( std::log(thisQVvisit+1.0) / (childQvisit+1.0) ) << " = " << childQcosttogo << std::endl;
//                 std::cout << " - " << cExplorationForSimulate_ << " * " << std::sqrt( std::log(thisQVvisit+1.0) / (childQvisit+1e-10) ) << " = " << childQcosttogo << std::endl;
                // NOTE commented this line to allow childQcosttogo with exploration bonus can be a negative value, only for action selection in this function
                //childQcosttogo = (childQcosttogo > 0.0) ? childQcosttogo : 0.0;

                if (minQcosttogo >= childQcosttogo)
                {
                    if (minQcosttogo > childQcosttogo)
                    {
                        minQcosttogoNodes.clear();
                    }
                    minQcosttogo = childQcosttogo;
                    minQcosttogoNodes.push_back(childQnode);
                }
            }
            if (minQcosttogoNodes.size()==1)
            {
                selectedChildQnode = minQcosttogoNodes[0];
            }
            else
            {
                assert(minQcosttogoNodes.size()!=0);
                int random = rand() % minQcosttogoNodes.size();
                selectedChildQnode = minQcosttogoNodes[random];  // to break the tie
            }
            // for debug
//             OMPL_INFORM("FIRMCP-Simulate >>>> selectedChlidQnode %u (%2.3f, %2.3f, %2.3f, %2.6f)", selectedChildQnode, 
//                     stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getX(),
//                     stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getY(),
//                     stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getYaw(),
//                     arma::trace(stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getCovariance()));

            selectedEdge = boost::edge(currentVertex, selectedChildQnode, g_).first;

        } // else if (currentDepth < maxPOMCPDepth_)


        // SIMULATE ACTION EXECUTION
        // TODO if the currently reached FIRM node is selected again (stabilization), execute several times more than the other case (transition), to reduce the depth of the tree toward the stabilization

        ompl::base::State* nextBelief = siF_->allocState();

        // the terminating edge controller was used once when currentDepth==maxPOMCPDepth_
        int kStep = std::max(0, currentDepth - maxPOMCPDepth_ + 1);
//         if (!executeSimulationFromUpto(kStep, rolloutSteps_, currentBelief, selectedEdge, nextBelief, executionCost))
        bool executionStatus = executeSimulationFromUpto(kStep, rolloutSteps_, currentBelief, selectedEdge, nextBelief, executionCost);
        if (!executionStatus)
        {
            OMPL_ERROR("Failed to executeSimulationFromUpto()!");
            //executionCost = obstacleCostToGo_;  // commented; obstacleCostToGo_ penalty will be considered by collisionDepth

// //             // approximately penalize the stabilization cost in the case that it immediately stops recursive simulation
// //             executionCost *= maxFIRMReachDepth_ - currentDepth;
//             collisionDepth = currentDepth;  // mark that collision happened at this depth from the root
//
//             // compute approximate edge cost
//             Vertex targetVertex = boost::target(selectedEdge, g_);   // latest target before reaching the finite horizon
//             double approxEdgeCost = computeApproxEdgeCost(currentVertex, targetVertex);
//             executionCost += approxEdgeCost;

            executionCost = obstacleCostToGo_;
        }

        // XXXXX for debug
//         OMPL_INFORM("FIRMCP: Moved to: (%2.3f, %2.3f, %2.3f, %2.6f)",
//                 nextBelief->as<FIRM::StateType>()->getX(),
//                 nextBelief->as<FIRM::StateType>()->getY(),
//                 nextBelief->as<FIRM::StateType>()->getYaw(),
//                 arma::trace(nextBelief->as<FIRM::StateType>()->getCovariance()));

        // XXX
        Visualizer::clearRolloutConnections();

        // ADD A QVNODE TO THE POMCP TREE
        // if the transioned state after simulated execution, T(h, a_j, o_k), is near to any of existing childQVnodes_[selectedChildQnode] (for the same action), T(h, a_j, o_l), on POMCP tree, merge them into one node!
        // REVIEW TODO how to update the existing belief state when another belief state is merged into this?
        // XXX currently, just keep the very first belief state without updating its belief state
        // but the first belief state won't represent the all belief states that are merged into it
        Vertex nextVertex;
        std::vector<Vertex> reachedChildQVnodes;
//         const std::vector<Vertex>& selectedChildQVnodes = currentBelief->as<FIRM::StateType>()->getChildQVnodes(selectedChildQnode);
        const Vertex selectedChildQVnode = currentBelief->as<FIRM::StateType>()->getChildQVnode(selectedChildQnode);

// 1)
//             // ORIGINAL
//             for (int j=0; j<selectedChildQVnodes.size(); j++)
//             {
//                 Vertex childQVnode = selectedChildQVnodes[j];
//
//                 // NOTE need to check for both directions since there is no from-to relationship between these selectedChildQVnodes
//                 if (stateProperty_[childQVnode]->as<FIRM::StateType>()->isReached(nextBelief))
//                 {
//                     //OMPL_WARN("Existing childQVnode!");
//                     nextVertex = childQVnode;
//                     reachedChildQVnodes.push_back(childQVnode);
//                 }
//                 else if (nextBelief->as<FIRM::StateType>()->isReached(stateProperty_[childQVnode]))
//                 {
//                     //OMPL_WARN("Existing childQVnode!");
//                     nextVertex = childQVnode;
//                     reachedChildQVnodes.push_back(childQVnode);
//                 }
//             }
//             // REVIEW what if a new childQVnode coincides more than one existing selectedChildQVnodes (for the same action)?
//             // pick the closest one?
//             // OR merge all selectedChildQVnodes[selectedChildQnode] into one node but with proper belief state merging scheme?!
//             if (reachedChildQVnodes.size()>1)
//             {
//                 //OMPL_WARN("There are %d reachedChildQVnodes for this new childQVnode!", reachedChildQVnodes.size());
//                 //nextVertex = which childQVnode?
//                 //exit(0);    // XXX
//
//                 int random = rand() % reachedChildQVnodes.size();
//                 nextVertex = reachedChildQVnodes[random];  // to break the tie
//             }
//             else if (reachedChildQVnodes.size()==0)

// 2)
//         if (selectedChildQVnodes.size()!=0)
//         {
//             // HACK for pomcpSimulate(), assume that T(hao) are always merged into one!
//             int random = rand() % selectedChildQVnodes.size();
//             nextVertex = selectedChildQVnodes[random];  // to break the tie
//
//             // update the matching belief state
//             siF_->copyState(stateProperty_[nextVertex], nextBelief);
//         }
//         else

// 3)
        if (selectedChildQVnode != ompl::magic::INVALID_VERTEX_ID)
        {
            nextVertex = selectedChildQVnode;

            // update the matching belief state
            // TODO belief state update from all particles, not just the latest one
            siF_->copyState(stateProperty_[nextVertex], nextBelief);
        }
        else


        {
            //OMPL_INFORM("A new childQVnode!");
            nextVertex = addQVnodeToPOMCPTree(siF_->cloneState(nextBelief));
            currentBelief->as<FIRM::StateType>()->addChildQVnode(selectedChildQnode, nextVertex);
        }

        // for debug
        if (currentDepth < maxPOMCPDepth_)
            std::cout << "-[" << selectedChildQnode << "]-" << nextVertex;
        else
            std::cout << ".[" << selectedChildQnode << "]." << nextVertex;


        // recursively call pomcpSimulate()
//         double delayedCostToGo = 0.0;
        double selectedChildQVmincosttogo = 0.0;
        if (executionStatus)  // if not collided during latest execution
        {
//             delayedCostToGo = pomcpSimulate(nextVertex, currentDepth+1, selectedEdge, collisionDepth);
            selectedChildQVmincosttogo = pomcpSimulate(nextVertex, currentDepth+1, selectedEdge, collisionDepth);
        }

        // free the memory
        siF_->freeState(nextBelief);



//         // total cost-to-go from this node
//         double discountFactor = 1.0;
//         double totalCostToGo = executionCost + discountFactor*delayedCostToGo;  // V(ha): childQvalues[selectedQnode]


//     // NOTE penalize collision with discount
//     // obstacleCostToGo_ would affect up to ~5 steps from the depth of collision and decay beyond it, so that its parent branch can have a chance to be selected
//     int depthDiff = collisionDepth - currentDepth;
//     depthDiff = (depthDiff < 0) ? 0 : depthDiff;
//     double discountedPenalty = obstacleCostToGo_ * 1.0/(1 + std::exp(2*(depthDiff-3)));  // C(ha): childQpenalties_[selectedQnode]
// //     totalCostToGo += computeDiscountedCollisionPenalty(executionCost, currentDepth, collisionDepth);
// //     totalCostToGo += discountedPenalty;
// //     double totalCostToGoWithDiscountedPenalty = totalCostToGo + discountedPenalty;



        // UPDATE THE NUMBER OF VISITS AND MISSES
        currentBelief->as<FIRM::StateType>()->addThisQVvisit();                    // N(h) += 1
        currentBelief->as<FIRM::StateType>()->addChildQvisit(selectedChildQnode);  // N(ha) += 1
        if (!executionStatus)  // if not collided during latest execution
        {
            currentBelief->as<FIRM::StateType>()->addChildQmiss(selectedChildQnode);  // M(ha) += 1
        }


        // UPDATE THE COST-TO-GO
        // REVIEW CHECK how about other heuristics to update the value, like simply taking the min value?
        // min value may lead to collision during execution!
        double selectedChildQvisit = currentBelief->as<FIRM::StateType>()->getChildQvisit(selectedChildQnode);  // N(ha)
        double selectedChildQmiss = currentBelief->as<FIRM::StateType>()->getChildQmiss(selectedChildQnode);    // M(ha)
//         double selectedChildQvalue, selectedChildQvalueUpdated;
//         double selectedChildQpenalty, selectedChildQpenaltyUpdated;
//         double selectedChildQcosttogoUpdated;
        double selectedChildQcosttogo = currentBelief->as<FIRM::StateType>()->getChildQcosttogo(selectedChildQnode);    // Q(ha)
        double selectedChildQcosttogoUpdated;
        double thisQVmincosttogo = currentBelief->as<FIRM::StateType>()->getThisQVmincosttogo();         // J(h)
        double thisQVmincosttogoUpdated;

//         selectedChildQvalue = currentBelief->as<FIRM::StateType>()->getChildQvalue(selectedChildQnode);      // V(ha)
//         selectedChildQpenalty = currentBelief->as<FIRM::StateType>()->getChildQpenalty(selectedChildQnode);  // C(ha)

        // a) V(ha) = V(ha) + (totalCostToGo - V(ha)) / N(ha); All Moves As First (AMAF) heuristic
        // collision happening very far from the current node affects this without any discount; execution gets stuck
//         if (isNewNodeExpanded)   // XXXXX
//             selectedChildQvalue = 0.0;  // to discard initial V(ha) set to be approxCostToGo from expandQnodesOnPOMCPTreeWithApproxCostToGo() for the first POMCP-Rollout node
//         selectedChildQvalueUpdated = selectedChildQvalue + (totalCostToGo - selectedChildQvalue) / selectedChildQvisit;  // V(ha) = V(ha) + (totalCostToGo - V(ha)) / N(ha)

        // b) V(ha) = V(ha) + (totalCostToGoWithDiscountedPenalty - V(ha)) / N(ha); All Moves As First (AMAF) heuristic
//         if (isNewNodeExpanded)   // XXXXX
//             selectedChildQvalue = 0.0;  // to discard initial V(ha) set to be approxCostToGo from expandQnodesOnPOMCPTreeWithApproxCostToGo() for the first POMCP-Rollout node
//         selectedChildQvalueUpdated = selectedChildQvalue + (totalCostToGoWithDiscountedPenalty - selectedChildQvalue) / selectedChildQvisit;  // V(ha) = V(ha) + (totalCostToGoWithDiscountedPenalty - V(ha)) / N(ha)

        // c) V(ha) = min(V(ha), totalCostToGo); Greedily taking the mininum
        // this naively ignores the chance of collision if there is at least a particle that didn't experience collision
//         if (isNewNodeExpanded)   // XXXXX
//             selectedChildQvalueUpdated = totalCostToGo;  // V(ha) = totalCostToGo for the first new node
//         else
//             selectedChildQvalueUpdated = std::min(selectedChildQvalue, totalCostToGo);  // V(ha) = min(V(ha), totalCostToGo)

        // d) V(ha) = min(V(ha), totalCostToGoWithDiscountedPenalty); Greedily taking the mininum
//         // this naively ignores the chance of collision if there is at least a particle that didn't experience collision
//         if (isNewNodeExpanded)   // XXXXX
//             selectedChildQvalueUpdated = totalCostToGoWithDiscountedPenalty;  // V(ha) = totalCostToGoWithDiscountedPenalty for the first new node
//         else
//             selectedChildQvalueUpdated = std::min(selectedChildQvalue, totalCostToGoWithDiscountedPenalty);  // V(ha) = min(V(ha), totalCostToGoWithDiscountedPenalty)

        // e) V(ha) = min(V(ha), totalCostToGo), C(ha) = max(C(ha), discountedPenalty), J(ha) = V(ha) + C(ha); Greedily taking the mininum
//         // this naively ignores the chance of collision if there is at least a particle that didn't experience collision
//         if (isNewNodeExpanded)   // XXXXX
//             selectedChildQvalueUpdated = totalCostToGo;  // V(ha) = totalCostToGo for the first new node
//         else
//             selectedChildQvalueUpdated = std::min(selectedChildQvalue, totalCostToGo);              // V(ha) = min(V(ha), totalCostToGo)
//         selectedChildQpenaltyUpdated = std::max(selectedChildQpenalty, discountedPenalty);          // C(ha) = max(C(ha), discountedPenalty)
//         selectedChildQcosttogoUpdated = selectedChildQvalueUpdated + selectedChildQpenaltyUpdated;  // J(ha) = V(ha) + C(ha)


        //*f) Q(ha) = c(a) + p * J(hao) + (1 - p) * J_obs
        //          = Q(ha) + (Q_k(ha) - Q(ha)) / N(ha), where
		//    Q_k(ha) = c(a) + ( J(hao) or J_obs )
        //    J(h) = min_a(Q(ha)) = min(J(h), Q(ha)), but
		//    J(h) != min(J(h), Q(ha)), since Q(ha) changes (may increase) over time

        // total cost-to-go from this node
//         double transitionProbability = 1.0 - selectedChildQmiss / selectedChildQvisit;
//         selectedChildQcosttogoUpdated = executionCost + transitionProbability*selectedChildQVmincosttogo + (1-transitionProbability)*obstacleCostToGo_;  // V(ha): childQvalues[selectedQnode]
		//    Q_k(ha) = c(a) + ( J(hao) or J_obs )
        selectedChildQcosttogoUpdated = executionCost;
        if (executionStatus)
            selectedChildQcosttogoUpdated += selectedChildQVmincosttogo;
        else
            selectedChildQcosttogoUpdated += obstacleCostToGo_;

        //    Q(ha) = Q(ha) + (Q_k(ha) - Q(ha)) / N(ha)
        if (isNewNodeExpanded)   // XXXXX
            selectedChildQcosttogo = 0.0;  // to discard initial V(ha) set to be approxCostToGo from expandQnodesOnPOMCPTreeWithApproxCostToGo() for the first POMCP-Rollout node
        selectedChildQcosttogoUpdated = selectedChildQcosttogo + (selectedChildQcosttogoUpdated - selectedChildQcosttogo) / selectedChildQvisit;  // Q(ha) = Q(ha) + (Q_k(ha) - Q(ha)) / N(ha)
//         thisQVmincosttogoUpdated = std::min(thisQVmincosttogo, selectedChildQcosttogoUpdated);  // J(h) = min_a(Q(ha)) = min(J(h), Q(ha))
        if (selectedChildQcosttogoUpdated < thisQVmincosttogo)
        {
            thisQVmincosttogoUpdated = selectedChildQcosttogoUpdated;
        }
        else
        {
            // check if the current J(h)=min_a'(Q(ha')) = Q(ha) before update (which may be less than the true Q(ha))
            thisQVmincosttogoUpdated = selectedChildQcosttogoUpdated;
            const std::vector<Vertex>& childQnodes = currentBelief->as<FIRM::StateType>()->getChildQnodes();
            for (int j=0; j<childQnodes.size(); j++)
            {
                const Vertex childQnode = childQnodes[j];
                const double childQcosttogo = stateProperty_[currentVertex]->as<FIRM::StateType>()->getChildQcosttogo(childQnode);

                if (thisQVmincosttogoUpdated > childQcosttogo)
                {
                    thisQVmincosttogoUpdated = childQcosttogo;
                }
            }
        }


        // a,b,c,d)
//         currentBelief->as<FIRM::StateType>()->setChildQvalue(selectedChildQnode, selectedChildQvalueUpdated);
        // e)
//         currentBelief->as<FIRM::StateType>()->setChildQvalue(selectedChildQnode, selectedChildQvalueUpdated);
//         currentBelief->as<FIRM::StateType>()->setChildQpenalty(selectedChildQnode, selectedChildQpenaltyUpdated);
//         currentBelief->as<FIRM::StateType>()->setChildQcosttogo(selectedChildQnode, selectedChildQcosttogoUpdated);
        //*f)
        currentBelief->as<FIRM::StateType>()->setChildQcosttogo(selectedChildQnode, selectedChildQcosttogoUpdated);
        currentBelief->as<FIRM::StateType>()->setThisQVmincosttogo(thisQVmincosttogoUpdated);
        // for debug
//         std::cout << "selectedChildQcosttogoUpdated[" << selectedChildQnode << "]: " << selectedChildQcosttogoUpdated << std::endl;


        // return total cost-to-go
        // NOTE recursive return value is V(ha), not J(ha), to separate C(ha) from accumulation!
//         return totalCostToGo;

        return thisQVmincosttogoUpdated;

    } // else if ((currentBelief->as<FIRM::StateType>()->getChildQnodes()).size()!=0)
}

double FIRMCP::pomcpRollout(const Vertex currentVertex, const int currentDepth, const Edge& selectedEdgePrev, int& collisionDepth, const bool isNewNodeExpanded)
{
    // declare local variables
    ompl::base::State* currentBelief = stateProperty_[currentVertex];  // current belief after applying previous control toward the latest target
    Edge selectedEdge;
    Vertex selectedChildQnode;


    if (currentDepth >= maxPOMCPDepth_)
    {

        Vertex targetVertex = boost::target(selectedEdgePrev, g_);   // latest target before reaching the finite horizon

        if (currentDepth >= maxFIRMReachDepth_)
        {
            OMPL_WARN("Could not reach to the target node within %d iterations", maxFIRMReachDepth_);
            //return obstacleCostToGo_;

// //             // approximately penalize the stabilization cost in the case that it immediately stops recursive simulation
// //             double executionCost = currentBelief->as<FIRM::StateType>()->getTraceCovariance();
// //             executionCost *= maxFIRMReachDepth_ - currentDepth;
//             collisionDepth = currentDepth;  // mark that collision happened at this depth from the root
// //             return executionCost;
//
//             // compute approximate edge cost and cost-to-go
//             double approxEdgeCost = computeApproxEdgeCost(currentVertex, targetVertex);
//             double approxCostToGo = costToGo_[targetVertex] + approxEdgeCost;
//
//             return approxCostToGo;


            double totalCostToGo = obstacleCostToGo_;

            // UPDATE THE NUMBER OF VISITS AND COST-TO-GO
            currentBelief->as<FIRM::StateType>()->addThisQVvisit();                    // N(h) += 1
            currentBelief->as<FIRM::StateType>()->setThisQVmincosttogo(totalCostToGo);

            return totalCostToGo;
        }

        // CREATE A NEW NODE IF NOT COINCIDES ANY OF EXISTING POMCP TREE NODES
//         if ((currentBelief->as<FIRM::StateType>()->getChildQnodes()).size()==0)
        if (!currentBelief->as<FIRM::StateType>()->getChildQexpanded())  // NOTE childQvalues are added only if this node was expanded before in expandQnodesOnPOMCPTreeWithApproxCostToGo()
        {
            // this will compute approximate edge cost, cost-to-go, and heuristic action weight
            // NOTE call this function just once per POMCP tree node
            if (!expandQnodesOnPOMCPTreeWithApproxCostToGo(currentVertex, isNewNodeExpanded))
            {
                OMPL_WARN("Failed to expandQnodesOnPOMCPTreeWithApproxCostToGo()!");
                //return obstacleCostToGo_;

// //                 // approximately penalize the stabilization cost in the case that it immediately stops recursive simulation
// //                 double executionCost = currentBelief->as<FIRM::StateType>()->getTraceCovariance();
// //                 executionCost *= maxFIRMReachDepth_ - currentDepth;
//                 collisionDepth = currentDepth;  // mark that collision happened at this depth from the root
// //                 return executionCost;
//
//                 // compute approximate edge cost and cost-to-go
//                 double approxEdgeCost = computeApproxEdgeCost(currentVertex, targetVertex);
//                 double approxCostToGo = costToGo_[targetVertex] + approxEdgeCost;
//
//                 return approxCostToGo;


                double totalCostToGo = obstacleCostToGo_;

                // UPDATE THE NUMBER OF VISITS AND COST-TO-GO
                currentBelief->as<FIRM::StateType>()->addThisQVvisit();                    // N(h) += 1
                currentBelief->as<FIRM::StateType>()->setThisQVmincosttogo(totalCostToGo);

                return totalCostToGo;
            }
        }


        // TODO CONTINUE TO MOVE TOWARD THE LATEST TARGET FIRM NODE AND RETURN COST-TO-GO
        if (stateProperty_[targetVertex]->as<FIRM::StateType>()->isReached(currentBelief))
        {
            // clear the rollout candidate connection drawings and show the selected edge
            Visualizer::clearRolloutConnections();
            //Visualizer::setChosenRolloutConnection(stateProperty_[tempVertex], stateProperty_[targetNode]);

            // for debug
            std::cout << std::endl;

            // compute approximate edge cost and cost-to-go
            double approxEdgeCost = computeApproxEdgeCost(currentVertex, targetVertex);
//             double approxCostToGo = costToGo_[targetVertex] + approxEdgeCost;
            double approxCostToGo = getCostToGoWithApproxStabCost(targetVertex) + approxEdgeCost;

            // UPDATE THE NUMBER OF VISITS AND COST-TO-GO
            currentBelief->as<FIRM::StateType>()->addThisQVvisit();                    // N(h) += 1
            currentBelief->as<FIRM::StateType>()->setThisQVmincosttogo(approxCostToGo);

            return approxCostToGo;
//             return costToGo_[targetVertex];
        }

        selectedEdge = selectedEdgePrev;
        selectedChildQnode = targetVertex;

        // check if previously selected action is still valid for this node
        const std::vector<Vertex>& childQnodes = currentBelief->as<FIRM::StateType>()->getChildQnodes();
        if (std::find(childQnodes.begin(), childQnodes.end(), selectedChildQnode) == childQnodes.end())
        {
            OMPL_WARN("selectedChildQnode action for %d node to reach a FIRM node %d during pomcpRollout() is not available for this current node!", currentVertex, selectedChildQnode);

            double totalCostToGo = obstacleCostToGo_;

            // UPDATE THE NUMBER OF VISITS AND COST-TO-GO
            currentBelief->as<FIRM::StateType>()->addThisQVvisit();                    // N(h) += 1
            currentBelief->as<FIRM::StateType>()->setThisQVmincosttogo(totalCostToGo);

            return totalCostToGo;
        }

    } // if (currentDepth >= maxPOMCPDepth_)
    else
    {

        // create a new node with default invalid N(ha) and V(ha)
            // need to register as a Vertex
            // need to add this to nearest neighbor database
            // find nearest neighbor child FIRM nodes, not any of POMCP tree nodes
                // then, save this child node id's in the Vertex attibute
                // also save estimated edge cost (from ditance) in the Vertex attibute
            // need edge/node controllers to its neighbors
            // but without edge cost computation


        // CREATE A NEW NODE IF NOT COINCIDES ANY OF EXISTING POMCP TREE NODES
//         if ((currentBelief->as<FIRM::StateType>()->getChildQnodes()).size()==0)
        if (!currentBelief->as<FIRM::StateType>()->getChildQexpanded())  // NOTE childQvalues are added only if this node was expanded before in expandQnodesOnPOMCPTreeWithApproxCostToGo()
        {
            // this will compute approximate edge cost, cost-to-go, and heuristic action weight
            // NOTE call this function just once per POMCP tree node
            if (!expandQnodesOnPOMCPTreeWithApproxCostToGo(currentVertex, isNewNodeExpanded))
            {
                OMPL_WARN("Failed to expandQnodesOnPOMCPTreeWithApproxCostToGo()!");
                //return obstacleCostToGo_;

// //                 // approximately penalize the stabilization cost in the case that it immediately stops recursive simulation
// //                 double executionCost = currentBelief->as<FIRM::StateType>()->getTraceCovariance();
// //                 executionCost *= maxFIRMReachDepth_ - currentDepth;
//                 collisionDepth = currentDepth;  // mark that collision happened at this depth from the root
// //                 return executionCost;
//
//                 std::cout << "segfault here?" << std::endl;
//         Vertex targetVertex = boost::target(selectedEdgePrev, g_);   // latest target before reaching the finite horizon
//
//                 // compute approximate edge cost and cost-to-go
//                 double approxEdgeCost = computeApproxEdgeCost(currentVertex, targetVertex);
//                 double approxCostToGo = costToGo_[targetVertex] + approxEdgeCost;
//
//                 return approxCostToGo;


                double totalCostToGo = obstacleCostToGo_;

                // UPDATE THE NUMBER OF VISITS AND COST-TO-GO
                currentBelief->as<FIRM::StateType>()->addThisQVvisit();                    // N(h) += 1
                currentBelief->as<FIRM::StateType>()->setThisQVmincosttogo(totalCostToGo);

                return totalCostToGo;
            }
        }


        // SELECT AN ACTION FROM ROLLOUT POLICY

    //     // find nearest neighbors to enumerate possible action
    //         // need to add the current state to the nearest neighbor database
    //             // adding every state to NN database during Rollout?
    //             // need to remove afterward? maybe yes only for NN database, but not for FIRM graph
    //             // or utilize graph connection once generated? yes
    //         // then just call 
    //         // neighbors = connectionStrategy_(m, NNRadius_);
    //
    //     // for each neighbor
    //         // compute the distance
    //         // compute the approximate edge cost
    //         // compute the approximate cost-to-go
    //         // compute the regularized weight from approximate cost-to-go


        // IMPORTANCE SAMPLING TO PICK ONE NEIGHBOR AS A TARGET

        // get the connected neighbor list
        const std::vector<Vertex>& childQnodes = currentBelief->as<FIRM::StateType>()->getChildQnodes();

        // allot a section in the weight bar according to each weight
        std::vector<double> weightSections;  // upper limit of accumulated weight for each action
        double costtogo, weight, weightSum = 0.0;

        // check for isReachedWithinNEps for any of nearest neighbor FIRM nodes
        bool isReachedWithinNEps = false;
        for (const auto& childQnode : childQnodes)
        {
            isReachedWithinNEps = stateProperty_[childQnode]->as<FIRM::StateType>()->isReachedWithinNEps(currentBelief, nEpsForIsReached_);
            break;
        }

        for (const auto& childQnode : childQnodes)
        {
                // FIXME in this case, childQweight is not needed since it is the same with the initialized childQvalue
                // and it makes more sense that pomcpRollout() uses UPDATED childQcosttogo for action selection once childQnode is expanded!
//             weight = currentBelief->as<FIRM::StateType>()->getChildQweight(childQnode);
//             weightSum += weight;



            // NOTE POMCP-Rollout is to explore the unknown area of the search space!
            // in our problem, the main unknown area is between the approximate stabilization of isReached() and the true stabilization
            // if isReachedWithinNEps() is satisfied, increase the randomness of POMCP-Rollout policy to explore this region,
            // otherwise, decrease the randomness to be more exploitative like FIRM-Rollout

            costtogo = currentBelief->as<FIRM::StateType>()->getChildQcosttogo(childQnode);

            //if (stateProperty_[targetVertex]->as<FIRM::StateType>()->isReachedWithinNEps(currentBelief))
            if (isReachedWithinNEps)
            {
                // explorative
//                 weight = 1.0 / std::pow(costtogo + costToGoRegulatorWithinReach_, cExploitationForRolloutWithinReach_);
                weight = 1.0 / (std::pow(costtogo, cExploitationForRolloutWithinReach_) + costToGoRegulatorWithinReach_);

                // for debug
                //std::cout << "isReachedWithinNEps: true" << std::endl;
            }
            else
            {
                // exploitative
//                 weight = 1.0 / std::pow(costtogo + costToGoRegulatorOutOfReach_, cExploitationForRolloutOutOfReach_);
                weight = 1.0 / (std::pow(costtogo, cExploitationForRolloutOutOfReach_) + costToGoRegulatorOutOfReach_);

                // for debug
                //std::cout << "isReachedWithinNEps: false" << std::endl;
            }


            weightSum += weight;
            weightSections.push_back(weightSum);
        }
        for (auto& weight : weightSections)
        {
            weight /= weightSum;  // normalize the accumulated weight
        }
        // for debug
//         std::cout << "-----------------------------" << std::endl;
//         std::cout << "childQnode: {";
//         for (const auto& childQnode : childQnodes)
//             std::cout << childQnode << " ";
//         std::cout << "}" << std::endl;
//         std::cout << "weight: {";
//         for (const auto& weight : weightSections)
//             std::cout << weight << " ";
//         std::cout << "}" << std::endl;

        // randomly pick an accumulate weight threshold
        arma::colvec weightPickedVec = arma::randu<arma::colvec>(1,1);  // range: [0, 1]
        double weightPicked = weightPickedVec[0];

        // enumerate to find the matching weightSection, assuming that childQnodes.size() is not so big
        int jSelected;
        for (int j=0; j<weightSections.size(); j++)
        {
            if (weightPicked < weightSections[j])
            {
                jSelected = j;
                break;
            }
        }


        // SELECT THE ACTION
        selectedChildQnode = childQnodes[jSelected];
        // for debug
//         OMPL_INFORM("FIRMCP-Rollout ~~~~~ selectedChlidQnode %u (%2.3f, %2.3f, %2.3f, %2.6f)", selectedChildQnode, 
//                 stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getX(),
//                 stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getY(),
//                 stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getYaw(),
//                 arma::trace(stateProperty_[selectedChildQnode]->as<FIRM::StateType>()->getCovariance()));

        selectedEdge = boost::edge(currentVertex, selectedChildQnode, g_).first;

    } // else if (currentDepth < maxPOMCPDepth_)


    // SIMULATE ACTION EXECUTION
        // TODO if the currently reached FIRM node is selected again (stabilization), execute several times more than the other case (transition), to reduce the depth of the tree toward the stabilization

    ompl::base::State* nextBelief = siF_->allocState();

    double executionCost;

    // the terminating edge controller was used once when currentDepth==maxPOMCPDepth_
    int kStep = std::max(0, currentDepth - maxPOMCPDepth_ + 1);
//     if (!executeSimulationFromUpto(kStep, rolloutSteps_, currentBelief, selectedEdge, nextBelief, executionCost))
    bool executionStatus = executeSimulationFromUpto(kStep, rolloutSteps_, currentBelief, selectedEdge, nextBelief, executionCost);
    if (!executionStatus)
    {
        OMPL_ERROR("Failed to executeSimulationFromUpto()!");
        //executionCost = obstacleCostToGo_;  // commented; obstacleCostToGo_ penalty will be considered by collisionDepth

// //         // approximately penalize the stabilization cost in the case that it immediately stops recursive simulation
// //         executionCost *= maxFIRMReachDepth_ - currentDepth;
//         collisionDepth = currentDepth;  // mark that collision happened at this depth from the root
//
//         // compute approximate edge cost
//         Vertex targetVertex = boost::target(selectedEdge, g_);   // latest target before reaching the finite horizon
//         double approxEdgeCost = computeApproxEdgeCost(currentVertex, targetVertex);
//         executionCost += approxEdgeCost;

        executionCost = obstacleCostToGo_;
    }

    // for debug
    // OMPL_INFORM("FIRMCP: Moved to: (%2.3f, %2.3f, %2.3f, %2.6f)",
    //         nextBelief->as<FIRM::StateType>()->getX(),
    //         nextBelief->as<FIRM::StateType>()->getY(),
    //         nextBelief->as<FIRM::StateType>()->getYaw(),
    //         arma::trace(nextBelief->as<FIRM::StateType>()->getCovariance()));

    // XXX
    Visualizer::clearRolloutConnections();


    // ADD A QVNODE TO THE POMCP TREE
    // if the transioned state after simulated execution, T(h, a_j, o_k), is near to any of existing childQVnodes_[selectedChildQnode] (for the same action), T(h, a_j, o_l), on POMCP tree, merge them into one node!
    // REVIEW TODO how to update the existing belief state when another belief state is merged into this?
    // XXX currently, just keep the very first belief state without updating its belief state
    // but the first belief state won't represent the all belief states that are merged into it
    Vertex nextVertex;
    std::vector<Vertex> reachedChildQVnodes;
//     const std::vector<Vertex>& selectedChildQVnodes = currentBelief->as<FIRM::StateType>()->getChildQVnodes(selectedChildQnode);
    const Vertex    selectedChildQVnode = currentBelief->as<FIRM::StateType>()->getChildQVnode(selectedChildQnode);

// 1)
            // ORIGINAL
//             for (int j=0; j<selectedChildQVnodes.size(); j++)
//             {
//                 Vertex childQVnode = selectedChildQVnodes[j];
//
//                 // NOTE need to check for both directions since there is no from-to relationship between these selectedChildQVnodes
//                 if (stateProperty_[childQVnode]->as<FIRM::StateType>()->isReached(nextBelief))
//                 {
//                     //OMPL_WARN("Existing childQVnode!");
//                     nextVertex = childQVnode;
//                     reachedChildQVnodes.push_back(childQVnode);
//                 }
//                 else if (nextBelief->as<FIRM::StateType>()->isReached(stateProperty_[childQVnode]))
//                 {
//                     //OMPL_WARN("Existing childQVnode!");
//                     nextVertex = childQVnode;
//                     reachedChildQVnodes.push_back(childQVnode);
//                 }
//             }
//             // REVIEW what if a new childQVnode coincides more than one existing selectedChildQVnodes (for the same action)?
//             // pick the closest one?
//             // OR merge all selectedChildQVnodes[selectedChildQnode] into one node but with proper belief state merging scheme?!
//             if (reachedChildQVnodes.size()>1)
//             {
//                 //OMPL_WARN("There are %d reachedChildQVnodes for this new childQVnode!", reachedChildQVnodes.size());
//                 //nextVertex = which childQVnode?
//                 //exit(0);    // XXX
//
//                 int random = rand() % reachedChildQVnodes.size();
//                 nextVertex = reachedChildQVnodes[random];  // to break the tie
//             }
//             else if (reachedChildQVnodes.size()==0)

// 2)
//         if (selectedChildQVnodes.size()!=0)
//         {
//             // HACK for pomcpSimulate(), assume that T(hao) are always merged into one!
//             int random = rand() % selectedChildQVnodes.size();
//             nextVertex = selectedChildQVnodes[random];  // to break the tie
//
//             // update the matching belief state
//             siF_->copyState(stateProperty_[nextVertex], nextBelief);
//         }
//         else

// 3)
        if (selectedChildQVnode != ompl::magic::INVALID_VERTEX_ID)
        {
            nextVertex = selectedChildQVnode;

            // update the matching belief state
            // TODO belief state update from all particles, not just the latest one
            siF_->copyState(stateProperty_[nextVertex], nextBelief);
        }
        else


    {
        //OMPL_INFORM("A new childQVnode!");
        nextVertex = addQVnodeToPOMCPTree(siF_->cloneState(nextBelief));
        currentBelief->as<FIRM::StateType>()->addChildQVnode(selectedChildQnode, nextVertex);
    }

    // for debug
    if (currentDepth < maxPOMCPDepth_)
        std::cout << "~(" << selectedChildQnode << ")~" << nextVertex;
    else
        std::cout << ".(" << selectedChildQnode << ")." << nextVertex;


    // recursively call pomcpRollout()
//     double delayedCostToGo = 0.0;
    double selectedChildQVmincosttogo = 0.0;
    if (executionStatus)  // if not collided during latest execution
    {
//         delayedCostToGo = pomcpRollout(nextVertex, currentDepth+1, selectedEdge, collisionDepth);
        selectedChildQVmincosttogo = pomcpRollout(nextVertex, currentDepth+1, selectedEdge, collisionDepth);
    }

    // free the memory
    siF_->freeState(nextBelief);



//     // total cost-to-go from this node
//     double discountFactor = 1.0;
//     double totalCostToGo = executionCost + discountFactor*delayedCostToGo;  // V(ha): childQvalues[selectedQnode]

//     // NOTE penalize collision with discount
//     // obstacleCostToGo_ would affect up to ~5 steps from the depth of collision and decay beyond it, so that its parent branch can have a chance to be selected
//     int depthDiff = collisionDepth - currentDepth;
//     depthDiff = (depthDiff < 0) ? 0 : depthDiff;
//     double discountedPenalty = obstacleCostToGo_ * 1.0/(1 + std::exp(2*(depthDiff-3)));  // C(ha): childQpenalties_[selectedQnode]
// //     totalCostToGo += computeDiscountedCollisionPenalty(executionCost, currentDepth, collisionDepth);
// //     totalCostToGo += discountedPenalty;
// //     double totalCostToGoWithDiscountedPenalty = totalCostToGo + discountedPenalty;


    // REVIEW do BACKUP() for all nodes during Rollout or not?
//     if (isNewNodeExpanded)
//     {
        // UPDATE THE NUMBER OF VISITS AND MISSES
        currentBelief->as<FIRM::StateType>()->addThisQVvisit();                    // N(h) += 1
        currentBelief->as<FIRM::StateType>()->addChildQvisit(selectedChildQnode);  // N(ha) += 1
        if (!executionStatus)  // if collided during latest execution
        {
            currentBelief->as<FIRM::StateType>()->addChildQmiss(selectedChildQnode);  // M(ha) += 1
        }


        // UPDATE THE COST-TO-GO
        // REVIEW CHECK how about other heuristics to update the value, like simply taking the min value?
        // min value may lead to collision during execution!
        double selectedChildQvisit = currentBelief->as<FIRM::StateType>()->getChildQvisit(selectedChildQnode);  // N(ha)
        double selectedChildQmiss = currentBelief->as<FIRM::StateType>()->getChildQmiss(selectedChildQnode);    // M(ha)
//         double selectedChildQvalue, selectedChildQvalueUpdated;
//         double selectedChildQpenalty, selectedChildQpenaltyUpdated;
//         double selectedChildQcosttogoUpdated;
        double selectedChildQcosttogo = currentBelief->as<FIRM::StateType>()->getChildQcosttogo(selectedChildQnode);    // Q(ha)
        double selectedChildQcosttogoUpdated;
        double thisQVmincosttogo = currentBelief->as<FIRM::StateType>()->getThisQVmincosttogo();      // J(h)
        double thisQVmincosttogoUpdated;

//         selectedChildQvalue = currentBelief->as<FIRM::StateType>()->getChildQvalue(selectedChildQnode);      // V(ha)
//         selectedChildQpenalty = currentBelief->as<FIRM::StateType>()->getChildQpenalty(selectedChildQnode);  // C(ha)

        // a) V(ha) = V(ha) + (totalCostToGo - V(ha)) / N(ha); All Moves As First (AMAF) heuristic
        // collision happening very far from the current node affects this without any discount; execution gets stuck
//         if (isNewNodeExpanded)   // XXXXX
//             selectedChildQvalue = 0.0;  // to discard initial V(ha) set to be approxCostToGo from expandQnodesOnPOMCPTreeWithApproxCostToGo() for the first POMCP-Rollout node
//         selectedChildQvalueUpdated = selectedChildQvalue + (totalCostToGo - selectedChildQvalue) / selectedChildQvisit;  // V(ha) = V(ha) + (totalCostToGo - V(ha)) / N(ha)

        // b) V(ha) = V(ha) + (totalCostToGoWithDiscountedPenalty - V(ha)) / N(ha); All Moves As First (AMAF) heuristic
        // collision happening very far from the current node affects this without any discount; execution gets stuck
//         if (isNewNodeExpanded)   // XXXXX
//             selectedChildQvalue = 0.0;  // to discard initial V(ha) set to be approxCostToGo from expandQnodesOnPOMCPTreeWithApproxCostToGo() for the first POMCP-Rollout node
//         selectedChildQvalueUpdated = selectedChildQvalue + (totalCostToGoWithDiscountedPenalty - selectedChildQvalue) / selectedChildQvisit;  // V(ha) = V(ha) + (totalCostToGoWithDiscountedPenalty - V(ha)) / N(ha)

        // c) V(ha) = min(V(ha), totalCostToGo); Greedily taking the mininum
        // this naively ignores the chance of collision if there is at least a particle that didn't experience collision
//         if (isNewNodeExpanded)   // XXXXX
//             selectedChildQvalueUpdated = totalCostToGo;  // V(ha) = totalCostToGo for the first new node
//         else
//             selectedChildQvalueUpdated = std::min(selectedChildQvalue, totalCostToGo);  // V(ha) = min(V(ha), totalCostToGo)

        // d) V(ha) = min(V(ha), totalCostToGoWithDiscountedPenalty); Greedily taking the mininum
//         // this naively ignores the chance of collision if there is at least a particle that didn't experience collision
//         if (isNewNodeExpanded)   // XXXXX
//             selectedChildQvalueUpdated = totalCostToGoWithDiscountedPenalty;  // V(ha) = totalCostToGoWithDiscountedPenalty for the first new node
//         else
//             selectedChildQvalueUpdated = std::min(selectedChildQvalue, totalCostToGoWithDiscountedPenalty);  // V(ha) = min(V(ha), totalCostToGoWithDiscountedPenalty)

        // e) V(ha) = min(V(ha), totalCostToGo), C(ha) = max(C(ha), discountedPenalty), J(ha) = V(ha) + C(ha); Greedily taking the mininum
//         // this naively ignores the chance of collision if there is at least a particle that didn't experience collision
//         if (isNewNodeExpanded)   // XXXXX
//             selectedChildQvalueUpdated = totalCostToGo;  // V(ha) = totalCostToGo for the first new node
//         else
//             selectedChildQvalueUpdated = std::min(selectedChildQvalue, totalCostToGo);              // V(ha) = min(V(ha), totalCostToGo)
//         selectedChildQpenaltyUpdated = std::max(selectedChildQpenalty, discountedPenalty);          // C(ha) = max(C(ha), discountedPenalty)
//         selectedChildQcosttogoUpdated = selectedChildQvalueUpdated + selectedChildQpenaltyUpdated;  // J(ha) = V(ha) + C(ha)


        //*f) Q(ha) = c(a) + p * J(hao) + (1 - p) * J_obs
        //          = Q(ha) + (Q_k(ha) - Q(ha)) / N(ha), where
		//    Q_k(ha) = c(a) + ( J(hao) or J_obs )
        //    J(h) = min_a(Q(ha)) = min(J(h), Q(ha)), but
		//    J(h) != min(J(h), Q(ha)), since Q(ha) changes (may increase) over time

        // total cost-to-go from this node
//         double transitionProbability = 1.0 - selectedChildQmiss / selectedChildQvisit;
//         selectedChildQcosttogoUpdated = executionCost + transitionProbability*selectedChildQVmincosttogo + (1-transitionProbability)*obstacleCostToGo_;  // V(ha): childQvalues[selectedQnode]
		//    Q_k(ha) = c(a) + ( J(hao) or J_obs )
        selectedChildQcosttogoUpdated = executionCost;
        if (executionStatus)
            selectedChildQcosttogoUpdated += selectedChildQVmincosttogo;
        else
            selectedChildQcosttogoUpdated += obstacleCostToGo_;

        //    Q(ha) = Q(ha) + (Q_k(ha) - Q(ha)) / N(ha)
        if (isNewNodeExpanded)   // XXXXX
            selectedChildQcosttogo = 0.0;  // to discard initial V(ha) set to be approxCostToGo from expandQnodesOnPOMCPTreeWithApproxCostToGo() for the first POMCP-Rollout node
        selectedChildQcosttogoUpdated = selectedChildQcosttogo + (selectedChildQcosttogoUpdated - selectedChildQcosttogo) / selectedChildQvisit;  // Q(ha) = Q(ha) + (Q_k(ha) - Q(ha)) / N(ha)
//         thisQVmincosttogoUpdated = std::min(thisQVmincosttogo, selectedChildQcosttogoUpdated);  // J(h) = min_a(Q(ha)) = min(J(h), Q(ha))
        if (selectedChildQcosttogoUpdated < thisQVmincosttogo)
        {
            thisQVmincosttogoUpdated = selectedChildQcosttogoUpdated;
        }
        else
        {
            // check if the current J(h)=min_a'(Q(ha')) = Q(ha) before update (which may be less than the true Q(ha))
            thisQVmincosttogoUpdated = selectedChildQcosttogoUpdated;
            const std::vector<Vertex>& childQnodes = currentBelief->as<FIRM::StateType>()->getChildQnodes();
            for (int j=0; j<childQnodes.size(); j++)
            {
                const Vertex childQnode = childQnodes[j];
                const double childQcosttogo = stateProperty_[currentVertex]->as<FIRM::StateType>()->getChildQcosttogo(childQnode);

                if (thisQVmincosttogoUpdated > childQcosttogo)
                {
                    thisQVmincosttogoUpdated = childQcosttogo;
                }
            }
        }


        // a,b,c,d)
//         currentBelief->as<FIRM::StateType>()->setChildQvalue(selectedChildQnode, selectedChildQvalueUpdated);
        //*e)
//         currentBelief->as<FIRM::StateType>()->setChildQvalue(selectedChildQnode, selectedChildQvalueUpdated);
//         currentBelief->as<FIRM::StateType>()->setChildQpenalty(selectedChildQnode, selectedChildQpenaltyUpdated);
//         currentBelief->as<FIRM::StateType>()->setChildQcosttogo(selectedChildQnode, selectedChildQcosttogoUpdated);
        //*f)
        currentBelief->as<FIRM::StateType>()->setChildQcosttogo(selectedChildQnode, selectedChildQcosttogoUpdated);
        currentBelief->as<FIRM::StateType>()->setThisQVmincosttogo(thisQVmincosttogoUpdated);
        // for debug
//         std::cout << "selectedChildQcosttogoUpdated[" << selectedChildQnode << "]: " << selectedChildQcosttogoUpdated << std::endl;

//     } // if (isNewNodeExpanded)
    

    // return total cost-to-go
    // NOTE recursive return value is V(ha), not J(ha), to separate C(ha) from accumulation!
//     return totalCostToGo;

    return thisQVmincosttogoUpdated;
}

FIRM::Vertex FIRMCP::addQVnodeToPOMCPTree(ompl::base::State *state)
{
    // just for compatibility with FIRM::addStateToGraph()
    bool addReverseEdge = false;

    boost::mutex::scoped_lock _(graphMutex_);

    // add the given belief state to graph as FIRM node
    Vertex m;
    m = boost::add_vertex(g_);

    stateProperty_[m] = state;
//     totalConnectionAttemptsProperty_[m] = 1;
//     successfulConnectionAttemptsProperty_[m] = 0;

    return m;
}

bool FIRMCP::expandQnodesOnPOMCPTreeWithApproxCostToGo(const Vertex m, const bool isNewNodeExpanded)
{
    // just for compatibility with FIRM::addStateToGraph()
    bool addReverseEdge = false;

    const Vertex start = startM_[0];

    // add this vertex to the database for nearest neighbor search
    if (m != start)
    {
        nn_->add(m);
    }

    // which milestones will we attempt to connect to?
    // NOTE use longer nearest neighbor to avoid oscillating behavior of rollout policy by allowing connection to farther FIRM nodes
    std::vector<Vertex> neighbors;
    std::vector<Vertex> kneighbors;
    if(addReverseEdge)  // graph construction phase
    {
        neighbors = connectionStrategy_(m, NNRadius_);
        //neighbors = kConnectionStrategy_(m, numNearestNeighbors_);    // NOTE this makes sense only if all the points are sampled first and then connected to each other, but not in the case point sampling and connection are done simultaneously
    }
    else  // rollout phase
    {
        // NOTE robust connection to a desirable (but far) FIRM nodes during rollout

        // 1) allow longer edge length for connection to FIRM nodes during rollout
//         neighbors = connectionStrategy_(m, 1.5*NNRadius_);
//         neighbors = connectionStrategy_(m, 1.2*NNRadius_);
        neighbors = connectionStrategy_(m, 1.0*NNRadius_);

        // 2) forcefully include the current state's k-nearest neighbors of FIRM nodes in the candidate list; this may be considered as a change of the graph connection property
//         kneighbors = kConnectionStrategy_(m, numNearestNeighbors_);
//         // remove duplicate entities from kneighbors
//         std::vector<Vertex>::iterator diff;
//         diff = std::set_difference(kneighbors.begin(), kneighbors.end(), neighbors.begin(), neighbors.end(), kneighbors.begin());
//         kneighbors.erase(diff, kneighbors.end());   // {kneighbors} = {kneighbors} - {neighbors}
//         // concantenate kneighbors to neighbors
//         neighbors.insert(neighbors.end(), kneighbors.begin(), kneighbors.end());

        // 3) instead of bounded nearest neighbors, use k-nearest neighbors during rollout; this may be considered as a change of the graph connection property
//         neighbors = kConnectionStrategy_(m, numNearestNeighbors_);

        // 4) forcefully include the next FIRM node of the previously reached FIRM node in the candidate (nearest neighbor) list
        // implemented in FIRM::executeFeedbackWithRollout() to access nextFIRMVertex information
//         neighbors = connectionStrategy_(m, NNRadius_);

        // 5) forcefully include future feedback nodes of several previous target nodes in the candidate (nearest neighbor) list
        // implemented in FIRM::executeFeedbackWithRollout() to access nextFIRMVertex information
//         neighbors = connectionStrategy_(m, NNRadius_);
    }

    // do not print this during rollout
    if(addReverseEdge)
        OMPL_INFORM("Adding a state: %u nearest neighbors from %u nodes in the graph", neighbors.size(), boost::num_vertices(g_));

    // remove this vertex to the database to exclude POMCP tree nodes from edge connection to FIRM nodes
    if (m != start)
    {
        nn_->remove(m);
    }

    // check for valid neighbors
    if (!addReverseEdge)
    {
        if (neighbors.size()==1)
        {
            if (m==neighbors[0])
            {
                OMPL_ERROR("No neighbor other than itself was found for vertex %d", m);
                exit(0);    // XXX for debug
                return false;
            }
        }
    }


    FIRMWeight approxEdgeCost;
    double approxCostToGo, weight;
    foreach (Vertex n, neighbors)
    {
        if ( m!=n )
        {
//             totalConnectionAttemptsProperty_[m]++;
//             totalConnectionAttemptsProperty_[n]++;

            // XXX CHECK is it better to checkMotion() beforehand, or learn by running Monte Carlo simulation?
            if (siF_->checkMotion(stateProperty_[m], stateProperty_[n]))     // NOTE skip this process since it calls (computationally expensive) isValid() function for all interpolated states along the edge
            {

                bool forwardEdgeAdded=false;

                // NOTE in execution mode, i.e., addReverseEdge is false, compute edge cost from the center belief state, not from a sampled border belief state
                approxEdgeCost = addEdgeToPOMCPTreeWithApproxCost(m, n, forwardEdgeAdded);

                if(forwardEdgeAdded)
                {
//                     successfulConnectionAttemptsProperty_[m]++;

                    // compute the approximate cost-to-go
//                     approxCostToGo = approxEdgeCost.getCost() + costToGo_[n];
                    approxCostToGo = approxEdgeCost.getCost() + getCostToGoWithApproxStabCost(n);

                    // compute weight for this action with regularization
                    // with higher costToGoRegulator_, weight will be closer to uniform distribution over actions
//                     weight = 1.0 / (approxCostToGo + costToGoRegulator_);


                    // FIXME in this case, childQweight is not needed since it is the same with the initialized childQvalue
                    // and it makes more sense that pomcpRollout() uses UPDATED childQcosttogo for action selection once childQnode is expanded!
//                     // HACK XXX XXX extremely exploitative
//                     // NOTE POMCP-Rollout policy should be more exploitative, so that pure-rollout branch can be more similar to the optimal branch and thus, simulate() also just follow the incrementally constructed rollout branch
//                     //weight = 1.0 / (std::pow(approxCostToGo, cExploitationForRollout_) + costToGoRegulator_);
//                     weight = approxCostToGo;  // save the raw approxCostToGo data, and the above computation is done in pomcpRollout()  // FIXME in this case, childQweight is not needed since it is the same with the initialized childQvalue
//
//                     // for debug
//                     //std::cout << "weight[" << n << "]: " << weight << " (=1.0/(" << approxCostToGo << "+" << costToGoRegulator_ << "))" << std::endl;


                    // save childQnode and weight for next POMCP-Rollout
                    stateProperty_[m]->as<FIRM::StateType>()->addChildQnode(n);
                    //stateProperty_[m]->as<FIRM::StateType>()->setChildQweight(n, weight);  // deprecated


                    // save approximate cost-to-go as an initial value for next POMCP-Simulate
                    // save for all nodes, and check for expanded by getChildQexpanded()
//                     if (isNewNodeExpanded)
                    {
                        // CHECK if approxCostToGo are actually a good initial value or not!
                        // this should depend on the tuning parameters... for the current setting, it over-estimates the executedCostToGo by 10 %
//                         stateProperty_[m]->as<FIRM::StateType>()->setChildQvalue(n, approxCostToGo);
                        stateProperty_[m]->as<FIRM::StateType>()->setChildQcosttogo(n, approxCostToGo);
                        // for debug
//                         std::cout << "approxCostToGo[" << m << "]: " << approxCostToGo << std::endl;
                    }
                }
            }
        }
    }

    // for rollout edge visualization
    if(!addReverseEdge)
    {
        foreach(Vertex n, neighbors)
        {
            if(boost::edge(m, n, g_).second)
            {
                Visualizer::addRolloutConnection(stateProperty_[m], stateProperty_[n]);
            }
        }
        //boost::this_thread::sleep(boost::posix_time::milliseconds(50));  // moved to FIRM::executeFeedbackWithRollout() for more accurate time computatiof
    }

    policyGenerator_->addFIRMNodeToObservationGraph(stateProperty_[m]);

    // mark that this node's childQnodes are expanded now
    stateProperty_[m]->as<FIRM::StateType>()->setChildQexpanded();

    return true;
}

FIRMWeight FIRMCP::addEdgeToPOMCPTreeWithApproxCost(const FIRM::Vertex a, const FIRM::Vertex b, bool &edgeAdded)
{
    // just for compatibility with FIRM::addEdgeToGraph()
    bool addReverseEdge = false;

    EdgeControllerType edgeController;

    // for debug
    if(ompl::magic::PRINT_MC_PARTICLES)
        std::cout << "=================================================" << std::endl;

    // HACK WORKAROUNDS FOR INDEFINITE STABILIZATION DURING ROLLOUT: {3} EDGE COST WITH A BORDER BELIEF STATE
    // apply the edge cost computation option
    bool constructionMode;
    if(borderBeliefSampling_)
    {
        // use the border belief state to compute the edge cost when constructing a graph
        constructionMode = addReverseEdge;
    }
    else
    {
        // use the center belief state to compute the edge cost even when constructing a graph
        constructionMode = false;
    }

    // generate an edge controller and compute edge cost
    //const FIRMWeight weight = generateEdgeControllerWithCost(a, b, edgeController, constructionMode);      // deprecated: EdgeController only
    //const FIRMWeight weight = generateEdgeNodeControllerWithCost(a, b, edgeController, constructionMode);  // EdgeController and NodeController concatenated
    const FIRMWeight weight = generateEdgeNodeControllerWithApproxCost(a, b, edgeController);    // EdgeController and NodeController concatenated; heuristically compute edge cost for POMCP-Rollout without Monte Carlo simulation

    assert(edgeController.getGoal() && "The generated controller has no goal");

    const unsigned int id = maxEdgeID_++;

    const Graph::edge_property_type properties(weight, id);

    // create an edge with the edge weight property
    std::pair<Edge, bool> newEdge = boost::add_edge(a, b, properties, g_);

    edgeControllers_[newEdge.first] = edgeController;

    edgeAdded = true;

    return weight;
}

FIRMWeight FIRMCP::generateEdgeNodeControllerWithApproxCost(const FIRM::Vertex a, const FIRM::Vertex b, EdgeControllerType &edgeController)
{
    ompl::base::State* startNodeState = siF_->cloneState(stateProperty_[a]);
//     ompl::base::State* targetNodeState = siF_->cloneState(stateProperty_[b]);
    ompl::base::State* targetNodeState = stateProperty_[b];

     // Generate the edge controller for given start and end state
    generateEdgeController(startNodeState,targetNodeState,edgeController);


    // compute approximate edge cost
    double approxEdgeCost = computeApproxEdgeCost(a, b);

    ompl::base::Cost edgeCost(approxEdgeCost);
    double transitionProbability = 1.0;    // just naively set this to 1.0

    FIRMWeight weight(edgeCost.value(), transitionProbability);

    // free the memory
    siF_->freeState(startNodeState);
//     siF_->freeState(targetNodeState);

    return weight;
}

double FIRMCP::computeApproxEdgeCost(const FIRM::Vertex a, const FIRM::Vertex b)
{
    double approxTransCost = computeApproxTransitionCost(a, b);
    double approxStabCost = computeApproxStabilizationCost(a, b);

    double approxEdgeCost = approxTransCost + approxStabCost;
    // for debug
    if(ompl::magic::PRINT_EDGE_COST)
        std::cout << "approxEdgeCost[" << a << "->" << b << "]: " << approxEdgeCost << " = " << approxTransCost << " + " << approxStabCost << std::endl;

    return approxEdgeCost;
}

double FIRMCP::computeApproxTransitionCost(const FIRM::Vertex a, const FIRM::Vertex b)
{
    // declare local variables
    ompl::base::State* startNodeState = stateProperty_[a];
    ompl::base::State* targetNodeState = stateProperty_[b];

    // compute the distance between two states
    double posDistance = startNodeState->as<FIRM::StateType>()->getPosDistanceTo(targetNodeState);
    double oriDistance = startNodeState->as<FIRM::StateType>()->getOriDistanceTo(targetNodeState);

    double startTraceCov = startNodeState->as<FIRM::StateType>()->getTraceCovariance();
    // XXX do not consider covariance for edge cost (transition until isReached() is satisfied)
//     double targetTraceCov = targetNodeState->as<FIRM::StateType>()->getTraceCovariance();
//     double covDistance = startTraceCov - targetTraceCov;
//     covDistance = (covDistance > 0.0) ? covDistance : 0.0;

    // compensate the distance considering isReached() tolerance
    // OPTIONAL) enabling these may lead to under-estimation of the actual cost, which then can result in jiggling execution
    posDistance = ((posDistance - StateType::reachDistPos_) > 0.0) ? (posDistance - StateType::reachDistPos_) : 0.0;
    oriDistance = ((oriDistance - StateType::reachDistOri_) > 0.0) ? (oriDistance - StateType::reachDistOri_) : 0.0;
//     covDistance = ((covDistance - StateType::reachDistCov_) > 0.0) ? (covDistance - StateType::reachDistCov_) : 0.0;

    // compute an approximate edge cost (heuristically!)
    double numPosConvergence = posDistance / heurPosStepSize_;
    double numOriConvergence = oriDistance / heurOriStepSize_;
//     double numCovConvergence = covDistance / heurCovStepSize_;   // heurCovStepSize_ is no longer being used!
//     double maxNumConvergence = std::max(std::max(numPosConvergence, numOriConvergence), numCovConvergence); // XXX
    double maxNumConvergence = std::max(numPosConvergence, numOriConvergence);
    double stepsToStop = maxNumConvergence;
    double filteringCost = startTraceCov * covConvergenceRate_ * (1 - std::pow(covConvergenceRate_, stepsToStop)) / (1 - covConvergenceRate_);  // sum_{k=1}^{stepsToStop}(covConvergenceRate^k * startTraceCov)

    // NOTE how to penalize uncertainty (covariance) and path length (time steps) in the cost
    //*1) cost = wc * sum(trace(cov_k))  + wt * K  (for k=1,...,K)
    // 2) cost = wc * trace(cov_f)       + wt * K
    // 3) cost = wc * mean(trace(cov_k)) + wt * K
    // 4) cost = wc * sum(trace(cov_k))

    double approxEdgeCost = informationCostWeight_*filteringCost + timeCostWeight_*stepsToStop;   // 1,2,3)
    //double approxEdgeCost = informationCostWeight_*filteringCost.value();   // 4)

    // free the memory
//     siF_->freeState(startNodeState);

    return approxEdgeCost;
}

double FIRMCP::computeApproxStabilizationCost(const FIRM::Vertex a, const FIRM::Vertex b)
{
    // declare local variables
    ompl::base::State* startNodeState = stateProperty_[a];
    ompl::base::State* targetNodeState = stateProperty_[b];

    // compute the covariance ratio between two states
    // NOTE more rigorously, startTraceCov should be after maxNumConvergence step of computeApproxTransitionCost()
    double startTraceCov = startNodeState->as<FIRM::StateType>()->getTraceCovariance();
    double targetTraceCov = targetNodeState->as<FIRM::StateType>()->getTraceCovariance();
    double covRatio = targetTraceCov / startTraceCov;
//     // for debug
//     std::cout << "covRatio: " << covRatio << std::endl;
    covRatio = (covRatio < 1.0) ? covRatio : 1.0;

    // compute an approximate stabilization cost (heuristically!)
    double numCovConvergence = std::log(covRatio) / std::log(covConvergenceRate_);
//     // for debug
//     std::cout << "numCovConvergence: " << numCovConvergence << std::endl;

    double maxNumConvergence = numCovConvergence;
    double stepsToStop = maxNumConvergence;
    double filteringCost = startTraceCov * covConvergenceRate_ * (1 - std::pow(covConvergenceRate_, stepsToStop)) / (1 - covConvergenceRate_);  // sum_{k=1}^{stepsToStop}(covConvergenceRate^k * startTraceCov)

    // NOTE how to penalize uncertainty (covariance) and path length (time steps) in the cost
    //*1) cost = wc * sum(trace(cov_k))  + wt * K  (for k=1,...,K)
    // 2) cost = wc * trace(cov_f)       + wt * K
    // 3) cost = wc * mean(trace(cov_k)) + wt * K
    // 4) cost = wc * sum(trace(cov_k))

    double approxStabCost = informationCostWeight_*filteringCost + timeCostWeight_*stepsToStop;   // 1,2,3)
    //double approxStabCost = informationCostWeight_*filteringCost.value();   // 4)

    // for debug
    if(ompl::magic::PRINT_EDGE_COST)
        std::cout << "approxStabCost[" << a << "->" << b << "] " << approxStabCost << std::endl;


    // free the memory
//     siF_->freeState(startNodeState);

    return approxStabCost;
}

double FIRMCP::getCostToGoWithApproxStabCost(const Vertex vertex)
{
    if (costToGoWithApproxStabCost_.find(vertex) == costToGoWithApproxStabCost_.end())
    {
        if (!updateCostToGoWithApproxStabCost(vertex))
        {
            //OMPL_ERROR("Failed to updateCostToGoWithApproxStabCost()!");
            return infiniteCostToGo_;
        }
    }

    return costToGoWithApproxStabCost_.at(vertex);
}

bool FIRMCP::updateCostToGoWithApproxStabCost(const Vertex current)
{
    const Vertex goal = goalM_[0];

    if (current == goal)
    {
        costToGoWithApproxStabCost_[current] = costToGo_.at(current);  // no need to update
        return true;
    }

    // Check if feedback from current to goal is valid or not
    if(!isFeedbackPolicyValid(current, goal))
    {
        //OMPL_WARN("No feedback path is found for the given node %d!", current);
        costToGoWithApproxStabCost_[current] = infiniteCostToGo_;  // fill infinite value here to avoid repetitive call for this node
        return false;
    }
    // check if feedback_ path exists for this node
    if (feedback_.find(current) == feedback_.end())
    {
        //OMPL_WARN("No feedback path is found for the given node %d!", current);
        costToGoWithApproxStabCost_[current] = infiniteCostToGo_;  // fill infinite value here to avoid repetitive call for this node
        return false;
    }
    if (costToGo_[current] >= infiniteCostToGo_)
    {
        //OMPL_WARN("costToGo_ is already larger than or equal to infiniteCostToGo_!");
        costToGoWithApproxStabCost_[current] = infiniteCostToGo_;  // fill infinite value here to avoid repetitive call for this node
        return false;
    }

    Edge edge = feedback_.at(current);
    Vertex next = boost::target(edge, g_);
    if (next == goal)
    {
        costToGoWithApproxStabCost_[current] = costToGo_.at(current);  // no need to update
        return true;
    }

    // recursively update the cost-to-go with approximate stabilization cost
    // 1) rigorously, approximate stabilization cost should start from a covariance which is larger than that of next by epsilon
//     costToGoWithApproxStabCost_[current] = costToGo_.at(current) + (getCostToGoWithApproxStabCost(next) - costToGo_.at(next)) + computeApproxStabilizationCost(next+epsilon, next);
    // 2) more rigorously, approximate stabilization cost should consider history from the start (computed by forward simulation like FIRM-Offline)
//     costToGoWithApproxStabCost_[current] = costToGo_.at(current) + (getCostToGoWithApproxStabCost(next) - costToGo_.at(next)) + computeApproxStabilizationCost(forward_simulated_next_reached, next_stationary_cov);
    // 3) approximately, mutliply an inflation factor to compensate under-estimation of actual (history dependent) stabilization cost approximately computed with stationary covariances (which is usually critical in the paths near the start point with high covariance)
    costToGoWithApproxStabCost_[current] = costToGo_.at(current) + (getCostToGoWithApproxStabCost(next) - costToGo_.at(next)) + inflationForApproxStabCost_*computeApproxStabilizationCost(current, next);

    return true;
}

bool FIRMCP::executeSimulationFromUpto(const int kStep, const int numSteps, const ompl::base::State *startState, const Edge& selectedEdge, ompl::base::State* endState, double& executionCost)
{
//     EdgeControllerType edgeController;
//     NodeControllerType nodeController;

    bool edgeControllerStatus;
    bool nodeControllerStatus;

    ompl::base::Cost costCov;
    int stepsExecuted = 0;
    int stepsToStop = 0;

    int currentTimeStep = 0;
    double executionCostCov = 0.0;
    executionCost = 0.0;

    const Vertex start = startM_[0];
    const Vertex goal = goalM_[0];
//     Vertex currentVertex = start;
//     Vertex tempVertex = currentVertex;
//     Vertex targetNode;

    ompl::base::State *cstartState = si_->allocState();
    ompl::base::State *cendState = si_->allocState();
//     ompl::base::State *goalState = si_->cloneState(stateProperty_[goal]);
    ompl::base::State *goalState = stateProperty_[goal];
    ompl::base::State *tempTrueStateCopy = si_->allocState();

    siF_->copyState(cstartState, startState);


    // [1] EdgeController
    Vertex targetNode = boost::target(selectedEdge, g_);
    EdgeControllerType& edgeController = edgeControllers_.at(selectedEdge);
    edgeController.setSpaceInformation(policyExecutionSI_);
    if(!edgeController.isTerminated(cstartState, 0))  // check if cstartState is near to the target FIRM node (by x,y position); this is the termination condition B) for EdgeController::Execute()
    {
        // NOTE do not execute edge controller to prevent jiggling motion around the target node

        edgeControllerStatus = edgeController.executeFromUpto(kStep, numSteps, cstartState, cendState, costCov, stepsExecuted, false);


        // NOTE how to penalize uncertainty (covariance) and path length (time steps) in the cost
        //*1) cost = wc * sum(trace(cov_k))  + wt * K  (for k=1,...,K)
        // 2) cost = wc * trace(cov_f)       + wt * K
        // 3) cost = wc * mean(trace(cov_k)) + wt * K
        // 4) cost = wc * sum(trace(cov_k))

        currentTimeStep += stepsExecuted;

        executionCostCov += costCov.value() - ompl::magic::EDGE_COST_BIAS;    // 1,2,3,4) costCov is actual execution cost but only for covariance penalty (even without weight multiplication)

        executionCost = informationCostWeight_*executionCostCov + timeCostWeight_*currentTimeStep;    // 1)
        //executionCost = informationCostWeight_*executionCostCov/(currentTimeStep==0 ? 1e-10 : currentTimeStep) + timeCostWeight_*currentTimeStep;    // 3)
        //executionCost = informationCostWeight_*executionCostCov;    // 4)

        costHistory_.push_back(std::make_tuple(currentTimeStep, executionCostCov, executionCost));


        // if edgeControllerStatus is false (usually due to collision or too much of deviation)
        if(!edgeControllerStatus)
        {
            OMPL_INFORM("Edge controller failed :(");

            // return the simulated result state
            siF_->copyState(endState, cendState);

            return false;
        }

        // this is a secondary (redundant) collision check for the true state
        siF_->getTrueState(tempTrueStateCopy);
        if(!siF_->isValid(tempTrueStateCopy))
        {
            OMPL_INFORM("Robot Collided :(");

            // return the simulated result state
            siF_->copyState(endState, cendState);

            return false;
        }

    } // [1] EdgeController

    // [2] NodeController
    else
    {
//         // HACK WORKAROUNDS FOR INDEFINITE STABILIZATION DURING ROLLOUT: {2} ACCUMULATING STATIONARY PENALTY
//         if(applyStationaryPenalty_)
//         {
//             // incrementally penalize a node that is being selected as a target due to the not-yet-converged current covariance even after the robot reached that node's position and orientation
//             // NOTE this is to myopically improve the suboptimal policy based on approximate value function (with inaccurate edge cost induced from isReached() relaxation)
//             // it will help to break (almost) indefinite stabilization process during rollout, especially when land marks are not very close
//             if(stateProperty_[targetNode]->as<FIRM::StateType>()->isReachedPose(cstartState))
//             {
//                 if(targetNode != goal)
//                 {
//                     // increase the stationary penalty
//                     if(stationaryPenalties_.find(targetNode) == stationaryPenalties_.end())
//                     {
//                         stationaryPenalties_[targetNode] = statCostIncrement_;
//                         // for log
//                         numberOfStationaryPenalizedNodes_++;
//                     }
//                     else
//                     {
//                         stationaryPenalties_[targetNode] += statCostIncrement_;
//                     }
//                     // for log
//                     sumOfStationaryPenalties_ += statCostIncrement_;
//                     //stationaryPenaltyHistory_.push_back(std::make_tuple(currentTimeStep_, numberOfStationaryPenalizedNodes_, sumOfStationaryPenalties_));
//
//                     // for debug
//                     if(ompl::magic::PRINT_STATIONARY_PENALTY)
//                         std::cout << "stationaryPenalty[" << targetNode << "]: " << stationaryPenalties_[targetNode] << std::endl;
//                 }
//             }
//         }
        // NOTE tried applying the stationary penalty if isReached(), instead of isTerminated(), is satisfied, but the resultant policy was more suboptimal
        //if(stateProperty_[targetNode]->as<FIRM::StateType>()->isReached(cstartState))
        //{
        //}


        // call StabilizeUpto() at every rollout iteration
        {
            NodeControllerType& nodeController = nodeControllers_.at(targetNode);

            nodeController.setSpaceInformation(policyExecutionSI_);

            //nodeControllerStatus = nodeController.StabilizeUpto(numSteps, cstartState, cendState, costCov, stepsExecuted, false);
            // NOTE to reduce the number of mostly identical POMCP tree nodes during stabilization, inflate the number of execution steps
            nodeControllerStatus = nodeController.StabilizeUpto(scaleStabNumSteps_*numSteps, cstartState, cendState, costCov, stepsExecuted, false);


            // NOTE how to penalize uncertainty (covariance) and path length (time steps) in the cost
            //*1) cost = wc * sum(trace(cov_k))  + wt * K  (for k=1,...,K)
            // 2) cost = wc * trace(cov_f)       + wt * K
            // 3) cost = wc * mean(trace(cov_k)) + wt * K
            // 4) cost = wc * sum(trace(cov_k))

            currentTimeStep += stepsExecuted;

            executionCostCov += costCov.value() - ompl::magic::EDGE_COST_BIAS;    // 1,2,3,4) costCov is actual execution cost but only for covariance penalty (even without weight multiplication)

            executionCost = informationCostWeight_*executionCostCov + timeCostWeight_*currentTimeStep;    // 1)
            //executionCost = informationCostWeight_*executionCostCov/(currentTimeStep==0 ? 1e-10 : currentTimeStep) + timeCostWeight_*currentTimeStep;    // 3)
            //executionCost = informationCostWeight_*executionCostCov;    // 4)

            costHistory_.push_back(std::make_tuple(currentTimeStep, executionCostCov, executionCost));


            // if nodeControllerStatus is false (usually due to too many iterations, more than maxTries_)
            if(!nodeControllerStatus)
            {
                OMPL_INFORM("Node controller failed :(");

                // return the simulated result state
                siF_->copyState(endState, cendState);

                return false;
            }

            // this is a secondary (redundant) collision check for the true state
            siF_->getTrueState(tempTrueStateCopy);
            if(!siF_->isValid(tempTrueStateCopy))
            {
                OMPL_INFORM("Robot Collided :(");

                // return the simulated result state
                siF_->copyState(endState, cendState);

                return false;
            }

        }
    } // [2] NodeController


//     // [3] Free the memory for states and controls for this temporary node/edge created from previous iteration
//     if(tempVertex != start)
//     {
//         foreach(Edge edge, boost::out_edges(tempVertex, g_))
//         {
//             edgeControllers_[edge].freeSeparatedController();
//             edgeControllers_[edge].freeLinearSystems();
//             edgeControllers_.erase(edge);
//         }
//
//         // NOTE there is no node controller generated for this temporary node during rollout execution
//
//
//         // remove the temporary node/edges after executing one rollout iteration
//         // NOTE this is important to keep tempVertex to be the same over each iteration
//         boost::clear_vertex(tempVertex, g_);    // remove all edges from or to tempVertex
//         boost::remove_vertex(tempVertex, g_);   // remove tempVertex
//         //stateProperty_.erase(tempVertex);
//         nn_->remove(tempVertex);
//     }

    // return the simulated result state
    siF_->copyState(endState, cendState);

    // free the memory
    si_->freeState(cstartState);
    si_->freeState(cendState);
    si_->freeState(tempTrueStateCopy);

    return true;
}

void FIRMCP::prunePOMCPTreeFrom(const Vertex rootVertex)
{
    ompl::base::State* rootState = stateProperty_[rootVertex];

    // recursively call prunePOMCPTreeFrom() to destruct the descendent nodes starting from the leaves
    if (rootState->as<FIRM::StateType>()->getChildQexpanded())
    {
        const std::vector<Vertex>& childQnodes = rootState->as<FIRM::StateType>()->getChildQnodes();
        for (const auto& childQnode : childQnodes)
        {
//             const std::vector<Vertex>& childQVnodes = rootState->as<FIRM::StateType>()->getChildQVnodes(childQnode);
//             for (const auto& childQVnode : childQVnodes)
//             {
//                 prunePOMCPTreeFrom(childQVnode);
//             }

            const Vertex childQVnode = rootState->as<FIRM::StateType>()->getChildQVnode(childQnode);
            if (childQVnode != ompl::magic::INVALID_VERTEX_ID)
            {
                prunePOMCPTreeFrom(childQVnode);
            }
        }
    }

    // prune this node on POMCP tree
    prunePOMCPNode(rootVertex);
}

void FIRMCP::prunePOMCPNode(const Vertex rootVertex)
{
    //const Vertex start = startM_[0];
    if (rootVertex != startM_[0])
    {
        // free the memory of controller
        // NOTE there is no node controller generated for POMCP tree nodes during rollout execution
        foreach(Edge edge, boost::out_edges(rootVertex, g_))
        {
            edgeControllers_[edge].freeSeparatedController();
//             edgeControllers_[edge].freeLinearSystems();
            edgeControllers_.erase(edge);
        }

        // free the memory of state
        siF_->freeState(stateProperty_[rootVertex]);

        // remove the node/edges from POMCP tree
        boost::clear_vertex(rootVertex, g_);     // remove all edges from or to rootVertex
        //boost::remove_vertex(rootVertex, g_);  // remove rootVertex  // NOTE commented this to avoid confusion from vertex IDs
        //stateProperty_.erase(rootVertex);

        // for debug
        //std::cout << " " << rootVertex;
    }
}

// for FIRM-Rollout
FIRM::Edge FIRMCP::generateRolloutPolicy(const FIRM::Vertex currentVertex, const FIRM::Vertex goal)
{
    /**
        For the given node, find the out edges and see the total cost of taking that edge
        The cost of taking the edge is cost to go from the target of the edge + the cost of the edge itself
    */
    double minCost = std::numeric_limits<double>::max();
    Edge edgeToTake;

    // for debug
    FIRM::Vertex minCostVertCurrent, minCostVertNext, minCostVertNextNext;

    // Iterate over the out edges
    foreach(Edge e, boost::out_edges(currentVertex, g_))
    {

        // Get the target node of the edge
        Vertex targetNode = boost::target(e, g_);  

        // The FIRM edge to take from the target node
        //Edge nextFIRMEdge = feedback_.at(targetNode);

        // The node to which next firm edge goes
        //Vertex targetOfNextFIRMEdge = boost::target(nextFIRMEdge, g_);

        // for debug
        if(ompl::magic::PRINT_FEEDBACK_PATH)
            std::cout << "PATH[" << currentVertex;

        // Check if feedback from target to goal is valid or not
        if(!isFeedbackPolicyValid(targetNode, goal))
        {
//             //OMPL_INFORM("Rollout: Invalid path detected from Vertex %u to %u", targetNode, targetOfNextFIRMEdge);
//
//             updateEdgeCollisionCost(targetNode, goal);
//
//             // resolve Dijkstra/DP
//             solveDijkstraSearch(goal);
//             solveDynamicProgram(goal, false);
//
//             //targetOfNextFIRMEdge = boost::target(feedback_.at(targetNode), g_);
//             //OMPL_INFORM("Rollout: Updated path, next firm edge moving from Vertex %u to %u", targetNode, targetOfNextFIRMEdge);

            updateCostToGoWithApproxStabCost(targetNode);
        }

        // The cost to go from the target node
//         double nextNodeCostToGo = costToGo_[targetNode];
        double nextNodeCostToGo = getCostToGoWithApproxStabCost(targetNode);

        // Find the weight of the edge
        FIRMWeight edgeWeight =  boost::get(boost::edge_weight, g_, e);

        // The transition prob of the edge
        double transitionProbability = edgeWeight.getSuccessProbability();        
        
        // compute distance from goal to target
        //arma::colvec targetToGoalVec = stateProperty_[goal]->as<FIRM::StateType>()->getArmaData() - stateProperty_[targetNode]->as<FIRM::StateType>()->getArmaData();
        //double distToGoalFromTarget = arma::norm(targetToGoalVec.subvec(0,1),2); 

        // get the stationary penalty
        // NOTE this is to myopically improve the suboptimal policy based on approximate value function (with inaccurate edge cost induced from isReached() relaxation)
        double stationaryPenalty = 0.0;
        if(stationaryPenalties_.find(targetNode) != stationaryPenalties_.end())
            stationaryPenalty = stationaryPenalties_.at(targetNode);

        // the cost of taking the edge
        //double edgeCostToGo = transitionProbability*nextNodeCostToGo + (1-transitionProbability)*obstacleCostToGo_ + edgeWeight.getCost();
        // NOTE this is to myopically improve the suboptimal policy based on approximate value function (with inaccurate edge cost induced from isReached() relaxation)
        double edgeCostToGo = transitionProbability*nextNodeCostToGo + (1-transitionProbability)*obstacleCostToGo_ + edgeWeight.getCost() + stationaryPenalty;  // HACK only for rollout policy search; actual execution cost will not consider stationaryPenalty


        // for debug
        if(ompl::magic::PRINT_COST_TO_GO)
//             std::cout << "COST[" << currentVertex << "->" << targetNode << "->G] " << edgeCostToGo << " = " << transitionProbability << "*" << nextNodeCostToGo << " + " << "(1-" << transitionProbability << ")*" << obstacleCostToGo_ << " + " << edgeWeight.getCost() << std::endl;
            std::cout << "COST[" << currentVertex << "->" << targetNode << "->G] " << edgeCostToGo << " = " << transitionProbability << "*" << nextNodeCostToGo << " + " << "(1-" << transitionProbability << ")*" << obstacleCostToGo_ << " + " << edgeWeight.getCost() << " + " << stationaryPenalty << std::endl;


        if(edgeCostToGo < minCost)
        {
            minCost  = edgeCostToGo;
            edgeToTake = e;

            // for debug
            minCostVertCurrent = currentVertex;
            minCostVertNext = targetNode;
            //minCostVertNextNext = targetOfNextFIRMEdge;
        }
    }

    // for debug
    if(ompl::magic::PRINT_COST_TO_GO)
        std::cout << "minC[" << minCostVertCurrent << "->" << minCostVertNext << "->G] " << minCost << std::endl;

    return edgeToTake;
}
