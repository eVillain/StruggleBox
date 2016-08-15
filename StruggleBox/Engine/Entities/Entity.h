#ifndef ENTITY_H
#define ENTITY_H

#include "Attribute.h"
#include <string>
#include <map>
#include <vector>

typedef enum {
    ENTITY_NONE = 0,            // For completion's sake
    ENTITY_DECOR = 1,           // Has gfx and optionally physics components
    ENTITY_HUMANOID = 2,        // Walks and talks
    ENTITY_SKELETON = 3,        // Undead, kills good things
    ENTITY_ITEM = 4,            // Can be picked up, used and thrown
    ENTITY_PROJECTILE = 5,      // Physical object fired from weapons
    ENTITY_LIGHT = 6,           // NOT IMPLEMENTED - MIGHT BE FACTORED OUT INTO A COMPONENT OR INTO CUBE DATA
} EntityType;

typedef enum {
    ALIGNMENT_CHAOTIC = -1,
    ALIGNMENT_NEUTRAL = 0,
    ALIGNMENT_LAWFUL = 1,
} EntityAlignment;


inline static std::string NameForEntity(const EntityType type)
{
    switch (type) {
        case ENTITY_NONE:
            return "No entity";
            break;
        case ENTITY_DECOR:
            return "Decor entity";
            break;
        case ENTITY_HUMANOID:
            return "Humanoid entity";
            break;
        case ENTITY_SKELETON:
            return "Skeleton entity";
            break;
        case ENTITY_ITEM:
            return "Item entity";
            break;
        case ENTITY_PROJECTILE:
            return "Projectile entity";
            break;
        case ENTITY_LIGHT:
            return "Light entity";
            break;
        default:
			return "Unknown entity";
            break;
    }
};

class EntityComponent;

class Entity
{
public:
    Entity(const std::string& name);
    ~Entity();

    // Entity Interface
    const unsigned int GetID() const;

    // Attribute Interface
    bool HasAttribute(const std::string& attrName) const;
    
    template<typename T> T& GetAttributeDataPtr(const std::string& attrName)
    {
        Attribute *a = GetAttribute<T>(attrName);
        if ( a != NULL &&  a->GetMagicNumber() == a->magic_number_for<T>() ) {
                attributeUpdate = true;
                return a->as<T>();
            
        } else {
            //        printf("[Entity] couldn't get attrib data pointer for %s, adding \n", attrName.c_str() );
            AddAttribute<T>(attrName);
            Attribute *a2 = GetAttribute<T>(attrName);
            return a2->as<T>();
        }
    }
    
    const std::vector<std::string> GetAttributeNames();
    const std::vector<std::string> GetAttributeTypes();
    Attribute* GetAttributeBase(const std::string& attrName) const;
    static const int GetNextEntityID() { return _nextEntityID; };
    std::map<const std::string, Attribute*>& GetAttributes() { return m_Attributes; }

    // Debugging output of all attributes and components
    void Print();
    
private:
    static int _nextEntityID;

    void RemoveAttribute(const std::string& attrName);
    void ClearAttributes();
    template<typename T> void AddAttribute(const std::string& attrName);
    template<typename T> void AddAttribute(const std::string& attrName, T value);
    
    // Entity Data
    unsigned int m_ID;
    bool attributeUpdate;
    std::map<const std::string, Attribute*> m_Attributes;
    template<typename T> Attribute* GetAttribute(const std::string& attrName);
    template<typename T> const Attribute* GetAttribute(const std::string& attrName) const;
};

#endif /* ENTITY_H */
