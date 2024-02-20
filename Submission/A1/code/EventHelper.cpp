//
// Created by Xuan Zhai on 2024/2/11.
//

#include "EventHelper.h"


/**
 * @brief Read and parse an event file.
 * @param fileName The event file's path and name.
 */
void EventHelper::ReadEventFile(const std::string& fileName){
    std::ifstream input = std::ifstream(fileName, std::ios::binary);

    if(!input){
        throw std::runtime_error("Parse Event Error: Unable to open the input file.");
    }

    float time;
    std::string type;
    std::string data;
    float data1;
    int data2;

    while(!input.eof()){
        EventNode newNode;
        input >> time >> type;
        newNode.time = time;

        if(type == "AVAILABLE"){
            newNode.eventType = EventType::AVAILABLE;
        }
        else if(type == "PLAY"){
            newNode.eventType = EventType::PLAY;
            input >> data1 >> data2;
            newNode.data = std::make_pair(data1,data2);
        }
        else if(type == "SAVE"){
            newNode.eventType = EventType::SAVE;
            input >> data;
            newNode.data = data;
        }
        else if(type == "MARK"){
            newNode.eventType = EventType::MARK;
            input.ignore();
            std::getline(input,data);
            newNode.data = data;
        }
        else{
            throw std::runtime_error("Parse Event Error: Unable to find the event type.");
        }
        events.emplace_back(newNode);
    }
}


/**
 * @brief Given the current time step, set the sliding window to refers to the current events to act.
 * @param time The current time step.
 */
void EventHelper::GetMatchedNode(float time){

    auto iter = std::lower_bound(events.begin(),events.end(),time, EventNode::compare);

    endIndex = iter-events.begin();
}


/**
 * @brief Check if all the events are done.
 * @return True if all are done.
 */
bool EventHelper::EventAllFinished() const {
    return startIndex == events.size();
}
