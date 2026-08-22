This repo uses  typeid which causes complications especially when hot reloading dlls that include querying pools created in it. Below macros is a much better solution than using typeID, just type it in each component like this:
 
```cpp
struct FooComponent{
  int bulletsLeft{};
  float length{};
  COMP_NAME(FooComponent);
};
```
where COMP_NAME will resolve into :
```cpp
#define COMP_NAME(type) \
  static constexpr std::string_view getName(){ \
    return #type;\
    } \
  static constexpr uint64_t getHash() {\
    return Hash(#type);\
  } \
  ```
  where Hash can be a determiinistic fn1va hash function : 

```cpp
#include <string_view>
#include <cstdint>

constexpr uint64_t Hash(std::string_view str) {
    uint64_t hash = 14695981039346656037ULL;
    for (char c : str) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}
```
Now creating and getting pools become trivial!
```cpp
template<typename T>
  void createPool(){
      if (pools.find(T::getHash()) != pools.end()){
          throw std::runtime_error("POOL ALREADY EXIST!!!");
      } 
      pools[T::getHash()] = std::make_unique<Pool<T>>();
  }


template<typename T>
  Pool<T>& getPool(){
      if (pools.find(T::getHash()) == pools.end()){
          throw std::runtime_error("POOL DOES NOT EXIST!!!");
      } 
  
      IComponentPool* purePtr = pools[T::getHash()].get();
      Pool<T>* specificPoolPtr = static_cast<Pool<T>*>(purePtr); 
      return *specificPoolPtr;
  }
```
Serialization/deserialization usage with new 
```cpp
      //alternative function for easier readability of files to be deserialized
 void deserializeComponent(Entity e, std::string_view compName, std::vector<Variable>&& vars){
      uint64_t compHash = Hash(compName);
      if(pools.find(compHash) == pools.end()){
          throw std::runtime_error("POOL DOES  NOT EXIST!");
      }
      pools[compHash]->assignComponentFields(e, std::move(vars));
  }

  void deserializeComponent(Entity e, uint64_t compHash, std::vector<Variable>&& vars){
      if(pools.find(compHash) == pools.end()){
          throw std::runtime_error("POOL DOES  NOT EXIST!");
      }
      pools[compHash]->assignComponentFields(e, std::move(vars));
  }
```
All in all good, name collisions are insured against due to createpool checking if it already exists!, so all good here!!


