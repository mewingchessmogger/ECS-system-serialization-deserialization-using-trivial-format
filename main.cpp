#include <iostream>
#include <fstream>
#include <string>
#include <variant>
#include <vector>
#include <typeinfo> // Required for typeid
#include <filesystem>
#include <array>
#include "include/engine_components.hpp"
#include "include/ecs_registry.hpp"
#include <sstream>
#include <cctype>

using vec3 = std::array<float,3>;


std::string getNextToken(std::istream& is) {
    char ch{};
    std::string token{};

    // 1. Skip whitespace
    while (is.get(ch) && std::isspace(ch)); // while we can get a char and its a whitespace

    if (!is) return ""; // End of file

    // 2. Is it a symbol?
    if (ch == '{' || ch == '}') {
        token = ch;
        return token;
    }

    // 3. Otherwise, read until the next whitespace or symbol
    token += ch;
    while (is.peek() != EOF && !std::isspace(is.peek())) { // peek will check next character without changing anythign
        char next = is.peek();
        if (next == '{' || next == '}') 
            break;
        is.get(ch);
        token += ch;
    }
    return token;
}

struct Bullet {
    vec3 position{};
    int size{};
    float drag{};

    REFLECT_3(position, size, drag);
    REFLECT_ADDRESS_3(position, size, drag);
};

struct Health{
    float health = 100;
    int hearts = 3;
    int foo{};

    REFLECT_3(health, hearts, foo);
    REFLECT_ADDRESS_3(health, hearts, foo);
};

struct ComponentPacket{
    std::string sName{}; // struct name from typeid
    std::vector<Variable> vars{};
    uint32_t id{};
};

void serialize(std::ostream& os, std::vector<Variable> &&vars){
    for( Variable& v : vars){
        if(std::holds_alternative<int>(v.var)){
            os << "      " << "int " << v.name << " ";
            int& ptr = std::get<int>(v.var);
            os <<  ptr << "\n";
        }
        else if(std::holds_alternative<float>(v.var)){
            os << "      " << "float " << v.name << " ";
            float& ptr = std::get<float>(v.var);
            os << ptr << "\n";
        }
        else if(std::holds_alternative<uint32_t>(v.var)){
            os << "      " << "uint32_t " << v.name << " ";
            uint32_t& ptr = std::get<uint32_t>(v.var);
            os << ptr << "\n";
        }
        else if(std::holds_alternative<vec3>(v.var)){
            os << "      " << "vec3 " << v.name << " ";
            vec3& ptr = std::get<vec3>(v.var);
            os << (ptr)[0] << " " << (ptr)[1] << " " << (ptr)[2]   << "\n";
        }
    }
}


void serializeReg(std::ofstream& os, ECS::Registry& reg){


    auto liveIDs = reg.getLiveIDs();

    
    const char* goon = "Entity ";
    for(auto e : liveIDs){
        os << goon <<e <<   " {\n";
        auto& map = reg.getPoolMap();
        for(auto& [key, pool] : map){
            if(pool->hasEntity(e)){
                os << "   " << key.name() << " {\n";
                
                serialize(os, pool->getComponentFields(e));
                os << "   }\n";
            }

        }
        os << "}\n";
    }

}

void serializePackets(std::ofstream& os, std::vector<ComponentPacket>& pkts){
    
    for(auto&pkt : pkts){
        os << "Entity " << pkt.id << " {\n";
        os << "   " << pkt.sName << " {\n";
        serialize(os, std::move(pkt.vars));
        os << "   }\n";
        os << "} \n";
    }
}



std::vector<ComponentPacket> deserializeFile(std::ifstream& is){
    enum class Scope{
        Entity, Struct, Count
    };

    std::string sName{};
    std::string type{};
    Variable currentVar{};
    uint32_t currentID{};
    std::vector<Variable> vars{};
    std::string token{};
    std::vector<Scope> scope{};
    std::vector<ComponentPacket> packets{};

    while(!is.eof()){
        token = getNextToken(is);

        if(token == "Entity"){
            scope.push_back(Scope::Entity);
            currentID = (uint32_t)std::stoul(getNextToken(is));
        }
        else if(token == "struct"){
            scope.push_back(Scope::Struct);
            sName = token + " " +  getNextToken(is);
        }
        else if(token == "{" && scope.back() == Scope::Struct){
            
            while(is.peek() != '}' && is.peek() != EOF) {
                type = getNextToken(is);
                currentVar.name = getNextToken(is);
                if(type == "float"){
                    currentVar.var = std::stof(getNextToken(is));
                }
                else if(type == "vec3"){
                    float f0 = stof(getNextToken(is));
                    float f1 = stof(getNextToken(is));
                    float f2 = stof(getNextToken(is));
                    currentVar.var = vec3{f0,f1,f2};
                }
                else if (type == "int"){
                    currentVar.var = std::stoi(getNextToken(is));

                }
                else if(type == "uint32_t"){
                    currentVar.var = (uint32_t)std::stoul(getNextToken(is));
                    
                }else{
                    throw std::runtime_error("ERROR TYPE DONT EXIST!!");
                }
                std::cout << currentVar.name << "!!\n";
                vars.push_back(currentVar);
                while(std::isspace(is.peek()) && is.peek() != EOF){
                    is.get();
                }
            }
            std::cout << "size of the vector is: " << vars.size() << "\n";
            packets.push_back({sName, std::move(vars), currentID});
            vars.clear();
            scope.pop_back();
        }
    }


    return packets;
}

void addRandomToReg(ECS::Registry& reg){
    
    uint32_t b = reg.createEntity();
    reg.getPool<Bullet>().assign(b, {vec3{0.1f,0.2f,0.3f},2, 0.2f});
    
    b = reg.createEntity();
    reg.getPool<Bullet>().assign(b, {vec3{0.5f,0.2f,0.3f},20, 0.6f});
    
    b = reg.createEntity();
    reg.getPool<Bullet>().assign(b, {vec3{0.9f,0.9f,0.9f},99, 0.9f});
    reg.getPool<Health>().assign(b,{299.0f,99,799});


    uint32_t r = reg.createEntity();
    reg.getPool<Renderable>().assign(r,{1});
    
    
    uint32_t h = reg.createEntity();
    reg.getPool<Health>().assign(h,{0.0f,5,7});
    uint32_t child = reg.createEntity();
    reg.getPool<Parent>().assign(child, {2, 1});
   
}

void main(){
    
       
    ECS::Registry reg{};
    reg.createPool<Bullet>();
    reg.createPool<Health>();
    reg.createPool<Renderable>();
    reg.createPool<Parent>();
    
    addRandomToReg(reg);
   
    std::ofstream save("save.txt"); // std::ios_base::app

    save << std::fixed << std::setprecision(3);

    serializeReg(save,reg);
    save.close();

    std::ifstream is("save.txt");

    auto packets = std::move(deserializeFile(is));    
    
    is.close();
    reg.cleanResetAll();
    
    for(auto& pkt: packets){
        reg.routeComponentToPool(pkt.id,pkt.sName,std::move(pkt.vars));
    }
    
    addRandomToReg(reg);


    std::ofstream secSave("save2.txt"); // std::ios_base::app

    secSave << std::fixed << std::setprecision(3);


    serializeReg(secSave,reg);
    secSave.close();

}