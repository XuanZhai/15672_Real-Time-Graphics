//
// Created by Xuan Zhai on 2024/2/11.
//

#ifndef XUANJAMESZHAI_A1_EVENTHELPER_H
#define XUANJAMESZHAI_A1_EVENTHELPER_H

#include <variant>
#include <string>
#include <vector>
#include <fstream>


/**
 * @brief A list of events that can do.
 */
enum EventType{
    AVAILABLE,
    PLAY,
    SAVE,
    MARK,
    NO_EVENT
};


/**
 * @brief A node refers to a specific event.
 */
class EventNode{
public:
    /* The time the event is processing. */
    float time = 0;
    /* The type of the event. */
    EventType eventType = EventType::NO_EVENT;
    /* The actual value of data in the event. */
    std::variant<std::string, std::pair<float,int>> data = "";

    static bool compare(const EventNode& a,float b) {
        return a.time <= b;
    }

    EventNode() = default;
};

class EventHelper {

public:
    /* A list contains all the events. */
    std::vector<EventNode> events;

    /* The start of the sliding window which has all the current events. */
    size_t startIndex = 0;
    /* The end of the sliding window. */
    size_t endIndex = 0;

    /* Read and parse the event file. */
    void ReadEventFile(const std::string& fileName);

    /* Given the current time step, set the sliding window to refers to the current events to act. */
    void GetMatchedNode(float time);

    /* Check if we reach to the end of the event lists. */
    bool EventAllFinished() const;
};


#endif //XUANJAMESZHAI_A1_EVENTHELPER_H
