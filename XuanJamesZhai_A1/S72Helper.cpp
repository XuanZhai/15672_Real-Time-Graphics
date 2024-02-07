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




Camera::Camera(const std::shared_ptr<ParserNode>& node, const std::string& newName){
    data = node;
    name = newName;
    projMatrix = XZM::mat4(1);
    viewMatrix = XZM::mat4(1);
}


void Camera::ComputeViewMatrix(){
    if(data == nullptr){
        throw std::runtime_error("Compute Camera Error: data is empty");
    }

    ParserNode::PNMap pnMap = std::get<ParserNode::PNMap>(data->data);
    //XZM::vec3 cameraPos =  XZM::GetTranslationFromMat(*std::get_if<XZM::mat4>(&(pnMap["wTransform"]->data)));
    //XZM::vec3 cameraPos = XZM::vec3(-3.857870,7.722800,-2.557720);

    //XZM::vec3 cameraPos = XZM::vec3(15,15,15);

    //XZM::quat* cameraDir = std::get_if<XZM::quat>(&(pnMap["wRotation"]->data));

    //if(cameraPos == nullptr || cameraDir == nullptr){
    //    throw std::runtime_error("Compute Camera Error: wTranslation/wRotation is empty");
    //}

    viewMatrix = XZM::LookAt(cameraPos,targetPos, XZM::vec3(0,0,1));

    //XZM::mat4 translate = XZM::Translate(XZM::mat4(1.0f), *cameraPos);
    //XZM::mat4 rotate = XZM::QuatToMat4(*cameraDir);

    //viewMatrix = XZM::Inverse(translate * rotate);
}


void Camera::ComputeProjectionMatrix(){
    if(data == nullptr){
        throw std::runtime_error("Compute Camera Error: data is empty");
    }

    ParserNode::PNMap pnMap = std::get<ParserNode::PNMap>(data->data);
    ParserNode::PNMap perspective = std::get<ParserNode::PNMap>(pnMap["perspective"]->data);

    float aspect = std::get<float>(perspective["aspect"]->data);
    float vfov = std::get<float>(perspective["vfov"]->data);
    float near = std::get<float>(perspective["near"]->data);
    float far = std::get<float>(perspective["far"]->data);

    projMatrix = XZM::Perspective(vfov,aspect,near,far);
}


void Camera::ProcessCamera(const XZM::mat4& transMatrix){
    cameraPos = XZM::ExtractTranslationFromMat(transMatrix);

    XZM::vec3 lookatDir = XZM::GetLookAtDir(cameraPos,transMatrix);
    targetPos = cameraPos + lookatDir;
    //targetPos = XZM::vec3();
    ComputeViewMatrix();
    ComputeProjectionMatrix();
}



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
    SetSrc(std::get<std::string>(new_src->data));
    SetTopology(std::get<std::string>(new_topology->data));
    count = (uint32_t) std::get<float>(new_count->data);
    stride = (uint32_t) std::get<float>(new_stride->data);

    SetFormat(0,std::get<std::string>(new_pFormat->data));
    SetFormat(1,std::get<std::string>(new_nFormat->data));
    SetFormat(2,std::get<std::string>(new_cFormat->data));

    pOffset = (uint32_t) std::get<float>(new_pOffset->data);
    nOffset = (uint32_t) std::get<float>(new_nOffset->data);
    cOffset = (uint32_t) std::get<float>(new_cOffset->data);
}


void Mesh::SetSrc(const std::string& srcPath){
    // TODO: Replace models with the upper path of s72.
    std::ifstream inputfile("Models/"+srcPath, std::ios::binary);

    if(!inputfile){
        std::cout << name + " Cannot open b72 file " << std::endl;
    }

    std::stringstream buffer;
    buffer << inputfile.rdbuf();
    src = buffer.str();
}


void Mesh::SetFormat(size_t channel, const std::string& format){

    if(channel == 0) pFormat = formatMap[format];
    else if(channel == 1) nFormat = formatMap[format];
    else cFormat = formatMap[format];
}


void Mesh::SetTopology(const std::string& new_topology){
    topology = topologyMap[new_topology];
}


void S72Helper::ReadS72(const std::string &filename) {

    XZJParser parser;
    root = parser.Parse(filename);

    ReconstructRoot();

    //UpdateNodes(root,XZM::vec3(0,0,0), XZM::quat(0.0,0.0,0.0,1.0), XZM::vec3(1,1,1));

}


/**
 * @brief Reconstruct the data structure to let the scene object to be the root.
 * Also reconstruct all the children and mesh relations.
 */
void S72Helper::ReconstructRoot() {

    std::shared_ptr<ParserNode> newRoot;

    /* Loop through the all the nodes to find the scene node. */
    for(const std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(root->data) ){

        /* Skip the first node which is the "s72-v1" */
        if(std::get_if<std::string>(&node->data) != nullptr){
            continue;
        }

        std::string nodeType = std::get<std::string>(node->GetObjectValue("type")->data);

        /* If the object has a key which is the roots, we found the scene node */
        if(nodeType == "SCENE"){
            newRoot = node;
        }
    }

    /* Recursively reconstruct its children and reset the root node */
    //tracingPath.emplace_back(newRoot);
    ReconstructNode(newRoot, XZM::mat4());
    root = newRoot;

    for(auto& camera : cameras){
        //camera.first->SetName();
        //camera.first->ComputeViewMatrix();
        //camera.first->ComputeProjectionMatrix();
        //camera.second = GetModelMatrix(camera.first->data);
    }

    for( auto& mesh : meshes){
        mesh.second->ProcessMesh();
       // mesh.second = GetModelMatrix(mesh.first->data);
    }

}


/**
 * @brief Reconstruct the child relation for a given node.
 * @param newNode The node we need to reconstruct.
 */
void S72Helper::ReconstructNode(std::shared_ptr<ParserNode> newNode, XZM::mat4 newMat) {

    /* If it is not an object, no need to reconstruct it. */
    if(std::get_if<ParserNode::PNMap>(&newNode->data) == nullptr){
        return;
    }

    ParserNode::PNMap newMap = std::get<ParserNode::PNMap>(newNode->data);
    std::string type = std::get<std::string>(newMap["type"]->data);

    if(type == "NODE"){
        //tracingPath.emplace_back(newNode);

        XZM::mat4 translation = XZM::Translation(S72Helper::FindTranslation(*newNode));
        XZM::mat4 rotation = XZM::QuatToMat4(S72Helper::FindRotation(*newNode));
        XZM::mat4 scale = XZM::Scaling(S72Helper::FindScale(*newNode));

        newMat = scale * rotation * translation * newMat;
    }
    else if(type == "MESH" || type == "CAMERA"){
        //std::shared_ptr<ParserNode> transMat(new ParserNode);


        //newMap.insert(std::make_pair("wTransform", transMat));

        if(type == "MESH"){
            //meshes.push_back(std::make_shared<Mesh>(newNode));
            std::string meshName = std::get<std::string>(newNode->GetObjectValue("name")->data);

            if(!meshes.count(meshName)){
                meshes.insert(std::make_pair(meshName,std::make_shared<Mesh>(newNode)));
            }

            //meshInstances.emplace_back(meshes[meshName], newMat);
            meshes[meshName]->instances.emplace_back(newMat);
            instanceCount++;
        }
        else{
            //cameras.push_back(std::make_shared<Camera>(newNode));
            std::string cameraName = std::get<std::string>(newNode->GetObjectValue("name")->data);
            std::shared_ptr<Camera> newCamera = std::make_shared<Camera>(newNode,cameraName);
            newCamera->ProcessCamera(newMat);
            //cameras.emplace_back(std::make_shared<Camera>(newNode), newMat);
            cameras.emplace_back(newCamera);
        }
    }


    /* If it has a mesh key. */
    if(newMap.count("mesh")){
        size_t idx = (size_t)std::get<float>(newMap["mesh"]->data);
        ReconstructNode(std::get<ParserNode::PNVector>(root->data)[idx],newMat);

        //newMap["mesh"] = std::get<ParserNode::PNVector>(root->data)[idx];
    }
    /* If it has a camera key. */
    if(newMap.count("camera")){
        size_t idx = (size_t)std::get<float>(newMap["camera"]->data);
        ReconstructNode(std::get<ParserNode::PNVector>(root->data)[idx],newMat);
        //newMap["camera"] = std::get<ParserNode::PNVector>(root->data)[idx];
    }
    /* If it has a roots key. */
    if(newMap.count("roots")){
        //ParserNode::PNVector newVec = std::get<ParserNode::PNVector>(newMap["roots"]->data);

        /* Loop through nodes in the vector and replace it with the real reference. */
        for(std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(newMap["roots"]->data) ){
            auto idx = (size_t)std::get<float>(node ->data);
            node = std::get<ParserNode::PNVector>(root->data)[idx];
            ReconstructNode(node,newMat);
        }
        //newMap["roots"]->data = newVec;
    }
    /* If it has a children key. */
    if(newMap.count("children")){
        //ParserNode::PNVector newVec = std::get<ParserNode::PNVector>(newMap["children"]->data);

        /* Loop through nodes in the vector and replace it with the real reference. */
        for(std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(newMap["children"]->data) ){
            auto idx = (size_t)std::get<float>(node ->data);
            node = std::get<ParserNode::PNVector>(root->data)[idx];
            ReconstructNode(node,newMat);
        }
       // newMap["children"]->data = newVec;
    }

    newNode->data = newMap;
}


void S72Helper::UpdateNodes(std::shared_ptr<ParserNode> &newNode, XZM::vec3 translation, XZM::quat rotation, XZM::vec3 scale) {
    /* If it is not an object, no need to iterate it.
    if(std::get_if<ParserNode::PNMap>(&newNode->data) == nullptr){
        return;
    }

    ParserNode::PNMap newMap = std::get<ParserNode::PNMap>(newNode->data);
    std::string type = std::get<std::string>(newMap["type"]->data);

    if(type == "NODE"){
        translation = translation * FindTranslation(*newNode);
        rotation = XZM::Normalize(FindRotation(*newNode) * rotation);
        scale = scale * FindScale(*newNode);
    }
    else if(type == "MESH" || type == "CAMERA"){
        std::shared_ptr<ParserNode> transNode(new ParserNode());
        transNode->data = translation;

        std::shared_ptr<ParserNode> rotNode(new ParserNode());
        rotNode->data = rotation;

        std::shared_ptr<ParserNode> scaleNode(new ParserNode());
        scaleNode->data = scale;

        newMap.insert(std::make_pair("wTranslation", transNode));
        newMap.insert(std::make_pair("wRotation", rotNode));
        newMap.insert(std::make_pair("wScale", scaleNode));
    } */

    /* If it has a mesh key. */
    //if(newMap.count("mesh")){
        //size_t idx = (size_t)std::get<float>(newMap["mesh"]->data);
    //    UpdateNodes(newMap["mesh"],translation,rotation,scale);
    //}
    /* If it has a camera key. */
    //if(newMap.count("camera")){
        //size_t idx = (size_t)std::get<float>(newMap["camera"]->data);
   //     UpdateNodes(newMap["camera"],translation,rotation,scale);
   // }
    /* If it has a roots key. */
    //if(newMap.count("roots")){

        /* Loop through nodes in the vector and replace it with the real reference. */
   //     for(std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(newMap["roots"]->data) ){
   //         UpdateNodes(node,translation,rotation,scale);
   //     }
   // }
    /* If it has a children key. */
    //if(newMap.count("children")){

        /* Loop through nodes in the vector and replace it with the real reference. */
    //    for(std::shared_ptr<ParserNode>& node : std::get<ParserNode::PNVector>(newMap["children"]->data) ){
   //         UpdateNodes(node,translation,rotation,scale);
    //    }
    //}

   // newNode->data = newMap;
}


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
