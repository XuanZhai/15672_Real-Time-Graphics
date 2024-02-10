//
// Created by Xuan Zhai on 2024/2/2.
//

#include "S72Helper.h"

#include <memory>


std::unordered_map<std::string, VkPrimitiveTopology> topologyMap = {
        {"POINT_LIST", VK_PRIMITIVE_TOPOLOGY_POINT_LIST},
        {"LINE_LIST",VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
        {"LINE_STRIP",VK_PRIMITIVE_TOPOLOGY_LINE_STRIP},
        {"TRIANGLE_LIST",VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
        {"TRIANGLE_STRIP",VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP},
        {"TRIANGLE_FAN",VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN},
        {"LINE_LIST_WITH_ADJACENCY",VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY},
        {"LINE_STRIP_WITH_ADJACENCY",VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY},
        {"TRIANGLE_LIST_WITH_ADJACENCY",VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY},
        {"TRIANGLE_STRIP_WITH_ADJACENCY",VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY},
        {"PATCH_LIST", VK_PRIMITIVE_TOPOLOGY_PATCH_LIST}
};


std::unordered_map<std::string,VkFormat> formatMap = {
        {"R32G32B32_SFLOAT", VK_FORMAT_R32G32B32_SFLOAT},
        {"R8G8B8A8_UNORM",VK_FORMAT_R8G8B8_UNORM}
};


/* ================================================ Camera ========================================================== */


/**
 * @brief Default Constructor
 */
Camera::Camera() {
    cameraPos = XZM::vec3(12.493,-3.00024,3.50548);
    cameraDir = XZM::Normalize(XZM::vec3(0,0,0)-cameraPos);
    aspect = 1.7778f;
    v_fov = 0.287167f;
    near_z = 0.1f;
    far_z = 1000;
}


/**
 * @brief explicit constructor with a given node and a name.
 * @param node The node in the s72 structure.
 * @param newName The name of the camera.
 */
Camera::Camera(std::shared_ptr<ParserNode> node, const std::string& newName){
    data = node;
    name = newName;
    projMatrix = XZM::mat4(1);
    viewMatrix = XZM::mat4(1);
    aspect = 0;
    v_fov = 0;
    near_z = 0;
    far_z = 0;
}


/**
 * @brief Set Camera's value based on the inputs.
 * @param newAspect new Aspect ratio.
 * @param new_V_fov new vertical field of view.
 * @param newNear new near plane z distance.
 * @param newFar nea far place z distance.
 */
void Camera::SetCameraData(float newAspect, float new_V_fov, float newNear, float newFar){
    aspect = newAspect;
    v_fov = new_V_fov;
    near_z = newNear;
    far_z = newFar;
    float tan_fov = tanf(v_fov * 0.5f);

    /* Use the camera data to set the frustum. */
    frustum.near_right = aspect * near_z * tan_fov,
    frustum.near_top = near_z * tan_fov,
    frustum.near_plane = -near_z;
    frustum.far_plane = -far_z;
}


/**
 * @brief Compute and set the camera's view matrix based on its position and direction.
 */
void Camera::ComputeViewMatrix(){
    viewMatrix = XZM::LookAt(cameraPos,cameraPos + cameraDir, XZM::vec3(0,0,1));
}


/**
 * @brief Compute and set the camera's projection matrix based on its value.
 */
void Camera::ComputeProjectionMatrix(){
    projMatrix = XZM::Perspective(v_fov,aspect,near_z,far_z);
}


/**
 * @brief Reset the camera's matrices based on its transform matrix.
 * @param transMatrix The world transform matrix applied to the camera.
 */
void Camera::ProcessCamera(const XZM::mat4& transMatrix){
    cameraPos = XZM::ExtractTranslationFromMat(transMatrix);
    cameraDir = XZM::Normalize(XZM::GetLookAtDir(cameraPos,transMatrix));

    ComputeViewMatrix();
    ComputeProjectionMatrix();
}


/**
 * @brief Move the camera forward or backward.
 * @param isForward True if move forward, false move backward.
 */
void Camera::MoveCameraForwardBackward(bool isForward){

    if(cameraDir.IsEmpty() || !isMovable) return;

    if(isForward){
        cameraPos += cameraDir*0.5f;
    }
    else{
        cameraPos -= cameraDir*0.5f;
    }

    ComputeViewMatrix();
    ComputeProjectionMatrix();
}


/**
 * @brief Rotate the camera up or down.
 * @param isUp True if rotate upward, false if backward.
 */
void Camera::MoveCameraUpDown(bool isUp) {

    if(cameraDir.IsEmpty() || !isMovable) return;

    if(isUp){
        cameraDir = XZM::RotateVec3(cameraDir, XZM::vec3(0,1,0), 0.02f);
    }
    else{
        cameraDir = XZM::RotateVec3(cameraDir, XZM::vec3(0,1,0), -0.02f);
    }

    ComputeViewMatrix();
    ComputeProjectionMatrix();
}


/**
 * @brief Rotate the camera left or right.
 * @param isUp True if rotate right, false if left.
 */
void Camera::MoveCameraLeftRight(bool isRight) {
    if(cameraDir.IsEmpty() || !isMovable) return;

    if(isRight){
        cameraDir = XZM::RotateVec3(cameraDir, XZM::vec3(0,0,1), 0.02f);
    }
    else{
        cameraDir = XZM::RotateVec3(cameraDir, XZM::vec3(0,0,1), -0.02f);
    }

    ComputeViewMatrix();
    ComputeProjectionMatrix();
}


/**
 * @brief Reset the camera direction to let the camera look toward the world center.
 */
void Camera::ReFocusToCenter() {

    cameraDir = XZM::Normalize(XZM::vec3(0,0,0) - cameraPos);

    ComputeViewMatrix();
    ComputeProjectionMatrix();
}


/* ================================================= Mesh =========================================================== */


/**
 * @brief Create a mesh instance based on a node in the s72 structure.
 * @param node A Parser Node from the XZJ parser.
 */
Mesh::Mesh(std::shared_ptr<ParserNode>& node){
    this->data = node;

    stride = 0;
    count = 0;

    pOffset = 0;
    nOffset = 0;
    cOffset = 0;
    topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    pFormat = VK_FORMAT_R32G32B32_SFLOAT;
    nFormat = VK_FORMAT_R32G32B32_SFLOAT;
    cFormat = VK_FORMAT_R8G8B8A8_UNORM;
}


/**
 * @brief Process the parser node's data and set mesh's data.
 */
void Mesh::ProcessMesh(){
    if(data == nullptr){
        throw std::runtime_error("Set Mesh Error: mesh instance is empty.");
    }

    ParserNode::PNMap* pnMap = std::get_if<ParserNode::PNMap>(&data->data);
    if(pnMap == nullptr || std::get<std::string>((*pnMap)["type"]->data) != "MESH"){
        throw std::runtime_error("Set Mesh Error: mesh instance is wrong.");
    }

    std::shared_ptr<ParserNode> attributes = data->GetObjectValue("attributes");
    std::shared_ptr<ParserNode> new_count = data->GetObjectValue("count");
    std::shared_ptr<ParserNode> new_topology = data->GetObjectValue("topology");
    std::shared_ptr<ParserNode> new_name = data->GetObjectValue("name");

    std::shared_ptr<ParserNode> position = attributes->GetObjectValue("POSITION");
    std::shared_ptr<ParserNode> normal = attributes->GetObjectValue("NORMAL");
    std::shared_ptr<ParserNode> color = attributes->GetObjectValue("COLOR");

    std::shared_ptr<ParserNode> new_src = position->GetObjectValue("src");
    std::shared_ptr<ParserNode> new_stride = position->GetObjectValue("stride");

    std::shared_ptr<ParserNode> new_pOffset = position->GetObjectValue("offset");
    std::shared_ptr<ParserNode> new_pFormat = position->GetObjectValue("format");

    std::shared_ptr<ParserNode> new_nOffset = normal->GetObjectValue("offset");
    std::shared_ptr<ParserNode> new_nFormat = normal->GetObjectValue("format");

    std::shared_ptr<ParserNode> new_cOffset = color->GetObjectValue("offset");
    std::shared_ptr<ParserNode> new_cFormat = color->GetObjectValue("format");

    name = std::get<std::string>(new_name->data);
    count = (uint32_t) std::get<float>(new_count->data);

    stride = (uint32_t) std::get<float>(new_stride->data);
    pOffset = (uint32_t) std::get<float>(new_pOffset->data);
    nOffset = (uint32_t) std::get<float>(new_nOffset->data);
    cOffset = (uint32_t) std::get<float>(new_cOffset->data);

    SetSrc(std::get<std::string>(new_src->data));
    SetTopology(std::get<std::string>(new_topology->data));

    SetFormat(0,std::get<std::string>(new_pFormat->data));
    SetFormat(1,std::get<std::string>(new_nFormat->data));
    SetFormat(2,std::get<std::string>(new_cFormat->data));
}


/**
 * @brief Set the Mesh's data from a file based on a given path.
 * @param srcPath The path where the data file locate.
 */
void Mesh::SetSrc(const std::string& srcPath){
    // TODO: Replace models with the upper path of s72.
    std::ifstream input_file("Models/"+srcPath, std::ios::binary);

    if(!input_file){
        std::cout << name + " Cannot open b72 file " << std::endl;
    }
    /* Read the file into the src string. */
    std::stringstream buffer;
    buffer << input_file.rdbuf();

    /* Set the bounding box while reading the b72 file. */
    ReadBoundingBox(buffer);

    src = buffer.str();
}


/**
 * @brief Set the mesh's data format.
 * @param channel Can be position, normal, or color.
 * @param format The format in string.
 */
void Mesh::SetFormat(size_t channel, const std::string& format){
    /* Map the format from string to vkFormat, */
    if(!formatMap.count(format)){
        throw std::runtime_error("Set Mesh Error: Does not find a correspond format. ");
    }

    if(channel == 0) pFormat = formatMap[format];
    else if(channel == 1) nFormat = formatMap[format];
    else cFormat = formatMap[format];
}


/**
 * @brief Set the mesh's topology data.
 * @param new_topology The topology in string.
 */
void Mesh::SetTopology(const std::string& new_topology){
    /* Map the topology from string to vkTopology. */
    if(!topologyMap.count(new_topology)){
        throw std::runtime_error("Set Mesh Error: Does not find a correspond topology. ");
    }

    topology = topologyMap[new_topology];
}


/**
 * @brief Read the b72 file. Loop through each vertex position and construct the bounding box.
 * @param buffer The b72 file stored in a string stream buffer.
 */
void Mesh::ReadBoundingBox(std::stringstream& buffer){

    XZM::vec3 currPos,currNormal, currColor;

    /* The size of the position, normal, and color are determined by the offsets. */
    long long posSize = (nOffset - pOffset) / 3;
    long long normSize = (cOffset-nOffset) / 3;
    long long colorSize = (stride - cOffset) / 4;

    for(size_t i = 0; i < count; i++){
        buffer.read(reinterpret_cast<char*>(&currPos.data[0]), posSize);
        buffer.read(reinterpret_cast<char*>(&currPos.data[1]), posSize);
        buffer.read(reinterpret_cast<char*>(&currPos.data[2]), posSize);

        buffer.read(reinterpret_cast<char*>(&currNormal.data[0]), normSize);
        buffer.read(reinterpret_cast<char*>(&currNormal.data[1]), normSize);
        buffer.read(reinterpret_cast<char*>(&currNormal.data[2]), normSize);

        buffer.read(reinterpret_cast<char*>(&currColor.data[0]), colorSize);
        buffer.read(reinterpret_cast<char*>(&currColor.data[1]), colorSize);
        buffer.read(reinterpret_cast<char*>(&currColor.data[2]), colorSize);
        buffer.read(reinterpret_cast<char*>(&currColor.data[2]), colorSize);

        /* Update the min/max position in different axis. */
        boundingBox.b_min.data[0] = std::min(currPos.data[0], boundingBox.b_min.data[0]);
        boundingBox.b_min.data[1] = std::min(currPos.data[1], boundingBox.b_min.data[1]);
        boundingBox.b_min.data[2] = std::min(currPos.data[2], boundingBox.b_min.data[2]);
        boundingBox.b_max.data[0] = std::max(currPos.data[0], boundingBox.b_max.data[0]);
        boundingBox.b_max.data[1] = std::max(currPos.data[1], boundingBox.b_max.data[1]);
        boundingBox.b_max.data[2] = std::max(currPos.data[2], boundingBox.b_max.data[2]);
    }
}


void Driver::Initialization(const std::shared_ptr<ParserNode>& node){

    if(node == nullptr) return;

    nodeIndex = (int)std::get<float>(node->GetObjectValue("node")->data);
    channel = std::get<std::string>(node->GetObjectValue("channel")->data);
    ParserNode::PNVector timerNode = std::get<ParserNode::PNVector>(node->GetObjectValue("times")->data);
    ParserNode::PNVector valueNode = std::get<ParserNode::PNVector>(node->GetObjectValue("values")->data);

    for(size_t i = 0; i < timerNode.size(); i++){
        timers.emplace_back(std::get<float>(timerNode[i]->data));

        if(channel == "translation" || channel == "scale"){
            XZM::vec3 newNumber(std::get<float>(valueNode[3*i]->data), std::get<float>(valueNode[3*i+1]->data), std::get<float>(valueNode[3*i+2]->data));
            values.emplace_back(newNumber);
        }
        else{
            XZM::quat newNumber(std::get<float>(valueNode[4*i]->data), std::get<float>(valueNode[4*i+1]->data), std::get<float>(valueNode[4*i+2]->data), std::get<float>(valueNode[4*i+3]->data));
            values.emplace_back(newNumber);
        }
    }

}


std::variant<XZM::vec3,XZM::quat> Driver::GetCurrentData(float currTime){
    if(timers.empty()) return XZM::vec3();

    currTime = fmodf(currTime, timers.back());

    if(currTime <= timers.at(0)){
        return values.at(0);
    }

    auto low = std::lower_bound(timers.begin(),timers.end(), currTime);
    if(*low == timers.back()){
        return values.back();
    }

    auto high = std::next(low);
    float range = *high - *low;
    float t = 0;
    size_t lowIndex = low - timers.begin();
    size_t highIndex = high - timers.begin();

    if(range != 0){
        t = (currTime - *low)/range;
    }

    if(channel == "translation" || channel == "scale"){
        return XZM::Lerp(std::get<XZM::vec3>(values.at(lowIndex)), std::get<XZM::vec3>(values.at(highIndex)), t);
    }
    else{
        return XZM::Lerp(std::get<XZM::quat>(values.at(lowIndex)), std::get<XZM::quat>(values.at(highIndex)), t);
    }
}


std::string Driver::HasMatchNodeAndChannel(const std::shared_ptr<ParserNode>& node){
    int newIndex = (int)std::get<float>(node->GetObjectValue("ListIndex")->data);

    if(newIndex == nodeIndex){
        return channel;
    }
    return "";
}


/* =============================================== S72Helper ======================================================== */


/**
 * @brief Default constructor
 */
S72Helper::S72Helper(){
    /* Add the default user camera into the camera list. */
    std::shared_ptr<Camera> newCamera = std::make_shared<Camera>();
    newCamera->name = "User-Camera";
    /* The user camera is movable. */
    newCamera->isMovable = true;
    newCamera->ComputeViewMatrix();
    newCamera->ComputeProjectionMatrix();
    newCamera->SetCameraData(1.7778f,0.287167f,0.1f,1000);

    cameras.insert(std::make_pair(newCamera->name,newCamera));
}


/**
 * @brief Read and parse a s72 file.
 * @param filename The file name and its path.
 */
void S72Helper::ReadS72(const std::string &filename) {

    XZJParser parser;
    root = parser.Parse(filename);
    /* Reconstruct the parser data to form a tree structure. */
    ReconstructRoot();

    startTimePoint = std::chrono::high_resolution_clock::now();
}


/**
 * @brief Reconstruct the data structure to let the scene object to be the root.
 * Also reconstruct all the children and mesh relations.
 */
void S72Helper::ReconstructRoot() {

    std::shared_ptr<ParserNode> newRoot;
    float index = 0;

    /* Loop through the all the nodes to find the scene node. */
    for(const std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(root->data) ){

        /* Skip the first node which is the "s72-v1" */
        if(std::get_if<std::string>(&node->data) != nullptr){
            index++;
            continue;
        }

        std::shared_ptr<ParserNode> newIndex = std::make_shared<ParserNode>();
        newIndex->data = index;
        std::get<ParserNode::PNMap>(node->data).insert(make_pair("ListIndex", newIndex));

        /* If the object has a key which is the roots, we found the scene node */
        if(std::get<std::string>(node->GetObjectValue("type")->data) == "SCENE"){
            newRoot = node;
        }
        else if(std::get<std::string>(node->GetObjectValue("type")->data) == "DRIVER"){
            //size_t nodeIdx = (size_t)std::get<float>(node->GetObjectValue("node")->data);
            //std::shared_ptr<ParserNode> targetNode = std::get<ParserNode::PNVector>(root->data)[nodeIdx];
            //std::string nodeName = std::get<std::string>(targetNode->GetObjectValue("name")->data);
            //std::string name = std::get<std::string>(node->GetObjectValue("name")->data);

            std::shared_ptr<Driver> newDriver = std::make_shared<Driver>();
            newDriver->Initialization(node);

            movingNodes.emplace_back(newDriver);
        }
        index++;
    }

    /* Recursively reconstruct its children and reset the root node */
    ReconstructNode(newRoot, XZM::mat4());
    root = newRoot;

    /* Loop through all the meshes and construct their data. */
    for(auto& mesh : meshes){
        mesh.second->ProcessMesh();
    }
}


/**
 * @brief Reconstruct the child relation for a given node.
 * @param newNode The node we need to reconstruct.
 * @param newMat
 */
void S72Helper::ReconstructNode(std::shared_ptr<ParserNode> newNode, XZM::mat4 newMat) {

    /* If it is not an object, no need to reconstruct it. */
    if(std::get_if<ParserNode::PNMap>(&newNode->data) == nullptr){
        return;
    }

    ParserNode::PNMap newMap = std::get<ParserNode::PNMap>(newNode->data);
    std::string type = std::get<std::string>(newMap["type"]->data);

    /* If it is a node object, find its world transformation */
    if(type == "NODE"){
        XZM::vec3 translation = S72Helper::FindTranslation(*newNode);
        XZM::quat rotation = S72Helper::FindRotation(*newNode);
        XZM::vec3 scale = S72Helper::FindScale(*newNode);

        for(const auto& driver : movingNodes){
            std::string channel = driver->HasMatchNodeAndChannel(newNode);
            if(channel.empty()) continue;
            if(channel == "translation") translation = std::get<XZM::vec3>(driver->GetCurrentData(currTime));
            else if(channel == "rotation") rotation = std::get<XZM::quat>(driver->GetCurrentData(currTime));
            else if(channel == "scale") scale = std::get<XZM::vec3>(driver->GetCurrentData(currTime));
        }

        XZM::mat4 translationMatrix = XZM::Translation(translation);
        XZM::mat4 rotationMatrix = XZM::QuatToMat4(rotation);
        XZM::mat4 scaleMatrix = XZM::Scaling(scale);

        newMat = scaleMatrix * rotationMatrix * translationMatrix * newMat;
    }
    else if(type == "MESH"){
            std::string meshName = std::get<std::string>(newNode->GetObjectValue("name")->data);
            /* Insert into the mesh list, use a red-black tree so that it is unique. */
            if(!meshes.count(meshName)){
                meshes.insert(std::make_pair(meshName,std::make_shared<Mesh>(newNode)));
            }
            /* Add that instance to the mesh's instance list, also update the total instance count. */
            meshes[meshName]->instances.emplace_back(newMat);
            instanceCount++;
        }
    else if(type == "CAMERA"){
            std::string cameraName = std::get<std::string>(newNode->GetObjectValue("name")->data);
            ParserNode::PNMap perspective = std::get<ParserNode::PNMap>(newNode->GetObjectValue("perspective")->data);
            /* Create a camera instance and set its data. */
            std::shared_ptr<Camera> newCamera = std::make_shared<Camera>(newNode,cameraName);

            float aspect = std::get<float>(perspective["aspect"]->data);
            float v_fov = std::get<float>(perspective["vfov"]->data);
            float near = std::get<float>(perspective["near"]->data);
            float far = std::get<float>(perspective["far"]->data);
            newCamera->SetCameraData(aspect,v_fov,near,far);

            newCamera->ProcessCamera(newMat);
            cameras.insert(std::make_pair(cameraName,newCamera));
    }

    /* If it has a mesh key. Recursively visit its children. */
    if(newMap.count("mesh")){
        size_t idx = (size_t)std::get<float>(newMap["mesh"]->data);
        newMap["mesh"] = std::get<ParserNode::PNVector>(root->data)[idx];
        ReconstructNode(newMap["mesh"],newMat);
    }
    /* If it has a camera key. Recursively visit its children. */
    if(newMap.count("camera")){
        size_t idx = (size_t)std::get<float>(newMap["camera"]->data);
        newMap["camera"] = std::get<ParserNode::PNVector>(root->data)[idx];
        ReconstructNode(newMap["camera"],newMat);
    }
    /* If it has a roots key. Recursively visit its children. */
    if(newMap.count("roots")){
        /* Loop through nodes in the vector and replace it with the real reference. */
        for(std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(newMap["roots"]->data) ){
            auto idx = (size_t)std::get<float>(node ->data);
            node = std::get<ParserNode::PNVector>(root->data)[idx];
            ReconstructNode(node,newMat);
        }
    }
    /* If it has a children key. Recursively visit its children. */
    if(newMap.count("children")){
        /* Loop through nodes in the vector and replace it with the real reference. */
        for(std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(newMap["children"]->data) ){
            auto idx = (size_t)std::get<float>(node ->data);
            node = std::get<ParserNode::PNVector>(root->data)[idx];
            ReconstructNode(node,newMat);
        }
    }
    /* Update the node with the new object. */
    newNode->data = newMap;
}


/**
 * @brief Recursively loop through each object and update its transform matrix.
 */
void S72Helper::UpdateObjects(){
    /* Clear all the instances since we will add it again. */
    for(auto& mesh : meshes){
        mesh.second->instances.clear();
    }

    auto currentTimePoint = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTimePoint - startTimePoint).count();
    currTime = std::fmodf(time,120);

    UpdateObject(root,XZM::mat4());
}


/**
 * @brief Update an object's transform matrix and recursively visit its children.
 * @param newNode the newNode we are iterating.
 * @param newMat The new transform matrix.
 */
void S72Helper::UpdateObject(const std::shared_ptr<ParserNode>& newNode, XZM::mat4 newMat) {

    /* If it is not an object, no need to iterate it. */
    if(std::get_if<ParserNode::PNMap>(&newNode->data) == nullptr){
        return;
    }

    ParserNode::PNMap newMap = std::get<ParserNode::PNMap>(newNode->data);
    std::string type = std::get<std::string>(newMap["type"]->data);

    if(type == "NODE"){
        /* If it is a node object, find its world transform matrix. */
        XZM::vec3 translation = S72Helper::FindTranslation(*newNode);
        XZM::quat rotation = S72Helper::FindRotation(*newNode);
        XZM::vec3 scale = S72Helper::FindScale(*newNode);

        for(const auto& driver : movingNodes){
            std::string channel = driver->HasMatchNodeAndChannel(newNode);
            if(channel.empty()) continue;
            if(channel == "translation") translation = std::get<XZM::vec3>(driver->GetCurrentData(currTime));
            else if(channel == "rotation") {
                rotation = std::get<XZM::quat>(driver->GetCurrentData(currTime));
            }
            else if(channel == "scale") scale = std::get<XZM::vec3>(driver->GetCurrentData(currTime));
        }

        XZM::mat4 translationMatrix = XZM::Translation(translation);
        XZM::mat4 rotationMatrix = XZM::QuatToMat4(rotation);
        XZM::mat4 scaleMatrix = XZM::Scaling(scale);

        newMat = scaleMatrix * rotationMatrix * translationMatrix * newMat;
    }
    else if(type == "MESH"){
        /* Update the mesh instance with the new transform data. */
        std::string meshName = std::get<std::string>(newNode->GetObjectValue("name")->data);
        meshes[meshName]->instances.emplace_back(newMat);
    }
    else if(type == "CAMERA"){
        /* Update the camera with the new transform data. */
        std::string cameraName = std::get<std::string>(newNode->GetObjectValue("name")->data);
        cameras[cameraName]->ProcessCamera(newMat);
    }

    /* If it has a mesh key. Recursively visit its children. */
    if(newMap.count("mesh")){
        UpdateObject(newMap["mesh"],newMat);
    }
    /* If it has a camera key. Recursively visit its children. */
    if(newMap.count("camera")){
        UpdateObject(newMap["camera"],newMat);
    }
    /* If it has a roots key. Recursively visit its children. */
    if(newMap.count("roots")){
        /* Loop through nodes in the vector and update their value. */
        for(std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(newMap["roots"]->data) ){
            UpdateObject(node,newMat);
        }
    }
    /* If it has a children key. Recursively visit its children. */
    if(newMap.count("children")){
        /* Loop through nodes in the vector and update their value. */
        for(std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(newMap["children"]->data) ){
            UpdateObject(node,newMat);
        }
    }
}


/**
 * @brief Given a parser node which refers to a node object, find its translation info.
 * @param newNode The node we want to extract data from.
 * @return The local translation info.
 */
XZM::vec3 S72Helper::FindTranslation(const ParserNode& newNode) {

    XZM::vec3 trans(0,0,0);

    if(std::get_if<ParserNode::PNMap>(&newNode.data) == nullptr){
        return trans;
    }

    ParserNode::PNMap pnMap = std::get<ParserNode::PNMap>(newNode.data);
    if(pnMap.count("translation") == 0) return trans;

    ParserNode::PNVector pnVec = std::get<ParserNode::PNVector>(pnMap["translation"]->data);

    float x = std::get<float>(pnVec[0]->data);
    float y = std::get<float>(pnVec[1]->data);
    float z = std::get<float>(pnVec[2]->data);

    return {x,y,z};
}


/**
 * @brief Given a parser node which refers to a node object, find its rotation info.
 * @param newNode The node we want to extract data from.
 * @return The local rotation info.
 */
XZM::quat S72Helper::FindRotation(const ParserNode &newNode) {

    XZM::quat rotation(0,0,0,1);

    if(std::get_if<ParserNode::PNMap>(&newNode.data) == nullptr){
        return rotation;
    }

    ParserNode::PNMap pnMap = std::get<ParserNode::PNMap>(newNode.data);
    if(pnMap.count("rotation") == 0) return rotation;

    ParserNode::PNVector pnVec = std::get<ParserNode::PNVector>(pnMap["rotation"]->data);

    rotation.data[0] = std::get<float>(pnVec[0]->data);
    rotation.data[1] = std::get<float>(pnVec[1]->data);
    rotation.data[2] = std::get<float>(pnVec[2]->data);
    rotation.data[3] = std::get<float>(pnVec[3]->data);

    return rotation;
}


/**
 * @brief Given a parser node which refers to a node object, find its scale info.
 * @param newNode The node we want to extract data from.
 * @return The local scale info.
 */
XZM::vec3 S72Helper::FindScale(const ParserNode & newNode) {

    XZM::vec3 scale(1,1,1);

    if(std::get_if<ParserNode::PNMap>(&newNode.data) == nullptr){
        return scale;
    }

    ParserNode::PNMap pnMap = std::get<ParserNode::PNMap>(newNode.data);
    if(pnMap.count("scale") == 0) return scale;

    ParserNode::PNVector pnVec = std::get<ParserNode::PNVector>(pnMap["scale"]->data);

    float x = std::get<float>(pnVec[0]->data);
    float y = std::get<float>(pnVec[1]->data);
    float z = std::get<float>(pnVec[2]->data);

    return {x,y,z};
}
