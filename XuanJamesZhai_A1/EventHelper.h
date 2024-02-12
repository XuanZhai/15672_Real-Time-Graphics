//
// Created by Xuan Zhai on 2024/2/11.
//

#ifndef XUANJAMESZHAI_A1_EVENTHELPER_H
#define XUANJAMESZHAI_A1_EVENTHELPER_H

#include <variant>
#include <string>
#include <vector>
#include <fstream>

enum EventType{
    AVAILABLE,
    PLAY,
    SAVE,
    MARK,
    NO_EVENT
};

class EventNode{
public:
    float time = 0;
    EventType eventType = EventType::NO_EVENT;
    std::variant<std::string, std::pair<float,int>> data = "";

    static bool compare(const EventNode& a,float b) {
        return a.time <= b;
    }

    EventNode() = default;
};

class EventHelper {

public:
    std::vector<EventNode> events;
    size_t startIndex = 0;
    size_t endIndex = 0;

    void ReadEventFile(const std::string& fileName);

    void GetMatchedNode(float time);

    bool EventAllFinished() const;
};


#endif //XUANJAMESZHAI_A1_EVENTHELPER_H
